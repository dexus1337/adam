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
        m_shared_memory_base(nullptr),
        m_shared_memory_size(0)
        #ifdef ADAM_PLATFORM_WINDOWS
        , m_shared_memory_handle(nullptr)
        #endif
    {

    }

    memory_shared::~memory_shared() {}

    bool memory_shared::create(uint64_t buffer_size) 
    {
        bool success = false;

        #ifdef ADAM_PLATFORM_LINUX
        // Ensure name starts with / for POSIX compliance
        std::string linux_name = (m_name.c_str()[0] == '/') ? m_name.c_str() : "/" + std::string(m_name.c_str());
        
        int fd = shm_open(linux_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);

        if (fd == -1) 
            return false;

        // Set the size of the shared memory segment
        if (ftruncate(fd, buffer_size) == -1) 
        {
            close(fd);
            return false;
        }

        m_shared_memory_base = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        close(fd); // fd is no longer needed after mmap
        
        success = m_shared_memory_base != MAP_FAILED;
        #elifdef ADAM_PLATFORM_WINDOWS
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

        if (success)
        {
            m_is_owner              = true;
            m_shared_memory_size    = buffer_size;
        }

        return success;
    }

    bool memory_shared::open() 
    {
        #ifdef ADAM_PLATFORM_LINUX
        std::string linux_name = (m_name.c_str()[0] == '/') ? m_name.c_str() : "/" + std::string(m_name.c_str());
        
        int fd = shm_open(linux_name.c_str(), O_RDWR, 0666);
        if (fd == -1) return false;

        struct stat shm_stats;
        if (fstat(fd, &shm_stats) == -1) 
        {
            close(fd);
            return false;
        }

        m_shared_memory_size = shm_stats.st_size;

        m_shared_memory_base = mmap(NULL, m_shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        close(fd);
        
        return m_shared_memory_base != MAP_FAILED;
        #elif defined(ADAM_PLATFORM_WINDOWS)
        m_shared_memory_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, m_name.c_str());

        if (m_shared_memory_handle == NULL) 
            return false;

        m_shared_memory_base = MapViewOfFile(m_shared_memory_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

        return m_shared_memory_base != nullptr;
        #endif
    }

    void memory_shared::shutdown() 
    {
        if (m_shared_memory_base) 
        {
            #ifdef ADAM_PLATFORM_LINUX
            munmap(m_shared_memory_base, m_shared_memory_size);
            #elifdef ADAM_PLATFORM_WINDOWS
            UnmapViewOfFile(m_shared_memory_base);
            #endif
            m_shared_memory_base = nullptr;
        }

        #ifdef ADAM_PLATFORM_LINUX
        if (m_is_owner)
        {
            std::string linux_name = (m_name.c_str()[0] == '/') ? m_name.c_str() : "/" + std::string(m_name.c_str());
            shm_unlink(linux_name.c_str());
        }
        #elifdef ADAM_PLATFORM_WINDOWS
        if (m_shared_memory_handle) 
        {
            CloseHandle(m_shared_memory_handle);
            m_shared_memory_handle = NULL;
        }
        #endif
    }
}