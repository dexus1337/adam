#include "memory/memory.hpp"

#ifdef ADAM_PLATFORM_LINUX
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

#if defined(ADAM_PLATFORM_ANDROID)
#include <cstring>
#include <cerrno>

static inline std::string get_android_shm_path(const char* name)
{
    const char* tmp = std::getenv("TMPDIR");
    std::string base;
    if (tmp && tmp[0] != '\0' && access(tmp, W_OK) == 0)
        base = tmp;
    else
    {
        std::string pkg;
        FILE* f = fopen("/proc/self/cmdline", "r");
        if (f)
        {
            char buf[256] = {0};
            if (fgets(buf, sizeof(buf), f))
                pkg = buf;
            fclose(f);
        }

        if (!pkg.empty())
        {
            std::string c1 = "/data/user/0/" + pkg + "/cache";
            if (access(c1.c_str(), W_OK) == 0)
                base = c1;
            else
            {
                std::string c2 = "/data/data/" + pkg + "/cache";
                if (access(c2.c_str(), W_OK) == 0)
                    base = c2;
                else
                {
                    std::string c3 = "/data/user/0/" + pkg + "/files";
                    if (access(c3.c_str(), W_OK) == 0)
                        base = c3;
                }
            }
        }
    }

    if (base.empty())
        base = "/data/local/tmp";

    while (!base.empty() && base.back() == '/')
        base.pop_back();

    if (name && name[0] != '/')
        base += "/";

    if (name)
        base += name;

    return base;
}

static inline int android_shm_open(const char* name, int flags, mode_t mode)
{
    std::string path = get_android_shm_path(name);
    return open(path.c_str(), flags, mode);
}

static inline int android_shm_unlink(const char* name)
{
    std::string path = get_android_shm_path(name);
    int res = unlink(path.c_str());
    return res;
}
#define shm_open android_shm_open
#define shm_unlink android_shm_unlink
#endif
#endif

namespace adam 
{
    memory::memory(const string_hashed& name) 
     :  m_name(name), 
        m_s_state(state_zero),
        m_shared_memory_base(nullptr),
        m_shared_memory_size(0),
        m_memory_offset(0)
        #ifdef ADAM_PLATFORM_WINDOWS
        , m_shared_memory_handle(nullptr)
        #endif
    {
    }

    memory::~memory() 
    {
        if (is_created()) destroy(); 
    }

    bool memory::create(uint64_t buffer_size) 
    {
        if (buffer_size == 0) return false;

        bool success = false;

        #ifdef ADAM_PLATFORM_LINUX
        // Ensure name starts with / for POSIX compliance
        std::string linux_name;
        if (!m_name.empty() && m_name.c_str()[0] == '/') linux_name = m_name.c_str();
        else linux_name = "/" + std::string(m_name.c_str());

        // Unlink any existing segment to ensure a clean state
        shm_unlink(linux_name.c_str());
        
        int fd = shm_open(linux_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);
        if (fd == -1) return false;

        // Ensure permissions are correctly set
        fchmod(fd, 0666);

        // Align buffer size to system page size
        long page_size = sysconf(_SC_PAGESIZE);
        buffer_size = ((buffer_size + page_size - 1) / page_size) * page_size;

        // Set the size of the shared memory segment
        if (ftruncate(fd, buffer_size) == -1) 
        {
            close(fd);
            return false;
        }

        auto start = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        close(fd); // fd is no longer needed after mmap
        
        if (start == MAP_FAILED) return false;
        
        m_shared_memory_base = start;
        success = (start != MAP_FAILED);
        #elif defined(ADAM_PLATFORM_WINDOWS)

        // Align buffer size to system page size
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
            static_cast<DWORD>(buffer_size >> 32), 
            static_cast<DWORD>(buffer_size & 0xFFFFFFFF), 
            m_name.c_str()
        );         

        if (m_shared_memory_handle == NULL) return false;

        m_shared_memory_base = MapViewOfFile(m_shared_memory_handle, FILE_MAP_ALL_ACCESS, 0, 0, buffer_size);
        success = (m_shared_memory_base != nullptr);
        #endif

        if (!success) return false;

        m_s_state               = state_created | state_active | state_owned;
        m_shared_memory_size    = buffer_size;

        return true;
    }

    bool memory::open() 
    {
        #ifdef ADAM_PLATFORM_LINUX
        std::string linux_name;
        if (!m_name.empty() && m_name.c_str()[0] == '/') linux_name = m_name.c_str();
        else linux_name = "/" + std::string(m_name.c_str());
        
        int fd = shm_open(linux_name.c_str(), O_RDWR, 0666);
        if (fd == -1) return false;

        struct stat shm_stats;
        if (fstat(fd, &shm_stats) == -1) 
        {
            close(fd);
            return false;
        }

        auto start = mmap(NULL, shm_stats.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        close(fd); // fd is no longer needed after mmap
        
        if (start == MAP_FAILED) return false;
        
        m_shared_memory_base    = start;
        m_shared_memory_size    = shm_stats.st_size;

        #elif defined(ADAM_PLATFORM_WINDOWS)
        m_shared_memory_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, m_name.c_str());
        if (m_shared_memory_handle == NULL) return false;

        auto start = MapViewOfFile(m_shared_memory_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (start == nullptr) return false;

        m_shared_memory_base = start;

        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(start, &mbi, sizeof(mbi)))
        {
            m_shared_memory_size = mbi.RegionSize;
        }
        #endif

        m_s_state = state_created | state_active;

        return true;
    }

    bool memory::destroy() 
    {
        bool result = true;

        if (m_shared_memory_base) 
        {
            #ifdef ADAM_PLATFORM_LINUX
            result &= (munmap(m_shared_memory_base, m_shared_memory_size) == 0);
            #elif defined(ADAM_PLATFORM_WINDOWS)
            result &= static_cast<bool>(UnmapViewOfFile(m_shared_memory_base));
            #endif
            m_shared_memory_base = nullptr;
        }

        #ifdef ADAM_PLATFORM_LINUX
        if (is_owner())
        {
            std::string linux_name;
            if (!m_name.empty() && m_name.c_str()[0] == '/') linux_name = m_name.c_str();
            else linux_name = "/" + std::string(m_name.c_str());
            shm_unlink(linux_name.c_str());
        }
        #elif defined(ADAM_PLATFORM_WINDOWS)
        if (m_shared_memory_handle) 
        {
            result &= static_cast<bool>(CloseHandle(m_shared_memory_handle));
            m_shared_memory_handle = NULL;
        }
        #endif

        m_s_state = state_zero;

        return result;
    }
}