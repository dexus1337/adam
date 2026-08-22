#include "download.hpp"
#include <adam-core.hpp>

#if defined(ADAM_PLATFORM_WINDOWS)
#include <windows.h>
#include <wininet.h>
#else
#include <curl/curl.h>
#endif

namespace adam::lib::network
{
    #if defined(ADAM_PLATFORM_WINDOWS)
    bool download_file(const std::string& url, const std::string& user_agent, std::vector<uint8_t>& out_bytes)
    {
        if (url.empty()) return false;

        HINTERNET hInternet = InternetOpenA(user_agent.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) return false;

        DWORD timeout_ms = 5000;
        InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout_ms, sizeof(timeout_ms));
        InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout_ms, sizeof(timeout_ms));

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), "Accept: image/png,image/jpeg\r\n", static_cast<DWORD>(-1L), INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
        if (!hUrl)
        {
            InternetCloseHandle(hInternet);
            return false;
        }

        uint8_t buffer[8192];
        DWORD bytes_read = 0;
        out_bytes.clear();

        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytes_read) && bytes_read > 0)
        {
            out_bytes.insert(out_bytes.end(), buffer, buffer + bytes_read);
        }

        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);

        return !out_bytes.empty();
    }
    #else
    static size_t curl_write_callback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        size_t total_size = size * nmemb;
        std::vector<uint8_t>* mem = static_cast<std::vector<uint8_t>*>(userp);
        mem->insert(mem->end(), static_cast<uint8_t*>(contents), static_cast<uint8_t*>(contents) + total_size);
        return total_size;
    }

    bool download_file(const std::string& url, const std::string& user_agent, std::vector<uint8_t>& out_bytes)
    {
        if (url.empty()) return false;

        CURL* curl = curl_easy_init();
        if (!curl) return false;

        out_bytes.clear();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_bytes);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        return (res == CURLE_OK && !out_bytes.empty());
    }
    #endif
}
