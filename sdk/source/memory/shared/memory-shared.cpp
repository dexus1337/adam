#include "memory/shared/memory-shared.hpp"

#ifdef ADAM_PLATFORM_LINUX
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace adam 
{
    memory_shared::memory_shared(const string_hashed& name) 
     :  m_name(name), 
        m_is_owner(false),
        m_shared_memory_base(nullptr),
        m_shared_memory_size(0)
        #ifdef   ADAM_PLATFORM_LINUX
        , m_signal_sem(nullptr)
        #endif
        #ifdef ADAM_PLATFORM_WINDOWS
        , m_shared_memory_handle(nullptr)
        #endif
        , m_signal(this)
    {

    }

    memory_shared::~memory_shared() {}

    bool memory_shared::create(uint64_t buffer_size) 
    {
        bool success = false;

        #ifdef ADAM_PLATFORM_LINUX
        // Ensure name starts with / for POSIX compliance
        std::string linux_name = (m_name.c_str()[0] == '/') ? m_name.c_str() : "/" + std::string(m_name.c_str());

        // Incase earlier calls fucked shit up, remove the name
        shm_unlink(linux_name.c_str());
        
        int fd = shm_open(linux_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);

        if (fd == -1) 
            return false;

        // Align buffer size to page size and add space for the semaphore
        long page_size = sysconf(_SC_PAGESIZE);
        buffer_size = ((buffer_size + sizeof(sem_t) + page_size - 1) / page_size) * page_size;

        // Set the size of the shared memory segment
        if (ftruncate(fd, buffer_size) == -1) 
        {
            close(fd);
            return false;
        }

        auto start = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        close(fd); // fd is no longer needed after mmap
        
        if (start == MAP_FAILED)
            return false;
        
        m_signal_sem            = reinterpret_cast<sem_t*>(start);                      // Place the semaphore at the start of the shared memory
        m_shared_memory_base    = reinterpret_cast<uint8_t*>(start) + sizeof(sem_t);    // Reserve space for the semaphore at the start of the shared memory

        success = start != MAP_FAILED;
        #elif defined(ADAM_PLATFORM_WINDOWS)

        // Align buffer size to page size
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        uint64_t page_size = sys_info.dwPageSize;
        buffer_size = ((buffer_size + page_size - 1) / page_size) * page_size;

        // On Windows, CreateFileMapping with INVALID_HANDLE_VALUE uses the system paging file.
        m_shared_memory_handle = CreateFileMappingA
        (
            INVALID_HANDLE_VALUE,    
            NULL,                    
            PAGE_READWRITE,          
            (DWORD)(buffer_size >> 32), 
            (DWORD)(buffer_size & 0xFFFFFFFF), 
            m_name.c_str()
        );         

        if (m_shared_memory_handle == NULL) 
        return false;

        m_shared_memory_base = MapViewOfFile(m_shared_memory_handle, FILE_MAP_ALL_ACCESS, 0, 0, buffer_size);

        success = m_shared_memory_base != nullptr;
        #endif

        if (!success)
            return false;

        m_is_owner              = true;
        m_shared_memory_size    = buffer_size;

        if (!m_signal.create())
        {
            destroy(); // Clean up if signal initialization fails
            return false;
        }

        return true;
    }

    bool memory_shared::open() 
    {
        #ifdef ADAM_PLATFORM_LINUX
        std::string linux_name = (m_name.c_str()[0] == '/') ? m_name.c_str() : "/" + std::string(m_name.c_str());
        
        int fd = shm_open(linux_name.c_str(), O_RDWR, 0666);

        if (fd == -1) 
            return false;

        struct stat shm_stats;
        if (fstat(fd, &shm_stats) == -1) 
        {
            close(fd);
            return false;
        }

        auto start = mmap(NULL, shm_stats.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        close(fd); // fd is no longer needed after mmap
        
        if (start == MAP_FAILED)
            return false;
        
        m_signal_sem            = reinterpret_cast<sem_t*>(start);                      // Place the semaphore at the start of the shared memory
        m_shared_memory_base    = reinterpret_cast<uint8_t*>(start) + sizeof(sem_t);    // Reserve space for the semaphore at the start of the shared memory
        m_shared_memory_size    = shm_stats.st_size;

        #elif defined(ADAM_PLATFORM_WINDOWS)
        m_shared_memory_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, m_name.c_str());

        if (m_shared_memory_handle == NULL) 
            return false;

        auto start = MapViewOfFile(m_shared_memory_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

         if (start == nullptr)
            return false;

        m_shared_memory_base = start;

        MEMORY_BASIC_INFORMATION mbi;
        
        if (VirtualQuery(ptr, &mbi, sizeof(mbi)))
            m_shared_memory_size = mbi.RegionSize;
        #endif

        m_is_owner              = false;

        if (!m_signal.open())
        {
            destroy(); // Clean up if signal initialization fails
            return false;
        }

        return true;
    }

    bool memory_shared::destroy() 
    {
        if (!m_signal.destroy())
            return false;

        if (m_shared_memory_base) 
        {
            #ifdef ADAM_PLATFORM_LINUX
            if (munmap(reinterpret_cast<void*>(m_signal_sem), m_shared_memory_size) != 0)
                return false;
            m_signal_sem = nullptr;
            #elif defined(ADAM_PLATFORM_WINDOWS)
            if (!UnmapViewOfFile(m_shared_memory_base))
                return false;
            #endif
            m_shared_memory_base = nullptr;
        }

        #ifdef ADAM_PLATFORM_LINUX
        if (m_is_owner)
        {
            std::string linux_name = (m_name.c_str()[0] == '/') ? m_name.c_str() : "/" + std::string(m_name.c_str());
            shm_unlink(linux_name.c_str());
        }
        #elif defined(ADAM_PLATFORM_WINDOWS)
        if (m_shared_memory_handle) 
        {
            CloseHandle(m_shared_memory_handle);
            m_shared_memory_handle = NULL;
        }
        #endif

        return true;
    }
}