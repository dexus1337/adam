#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <adam-core.hpp>

namespace adam::lib::network
{
    /**
     * @brief Downloads a file over HTTP(S).
     * @param url URL of the resource to download.
     * @param user_agent User-Agent string to use for the HTTP request.
     * @param out_bytes Output buffer populated with the downloaded file content.
     * @return true if successful, false otherwise.
     */
    bool download_file(const std::string& url, const std::string& user_agent, std::vector<uint8_t>& out_bytes);
}
