#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <string>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

std::string pwchar_to_utf8(PWCHAR wstr)
{
    if (!wstr) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0) return "";
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size_needed, nullptr, nullptr);
    if (!str.empty() && str.back() == '\0') str.pop_back();
    return str;
}

int main()
{
    ULONG out_buf_len = 15000;
    PIP_ADAPTER_ADDRESSES p_addresses = (PIP_ADAPTER_ADDRESSES)std::malloc(out_buf_len);
    if (p_addresses)
    {
        ULONG flags = GAA_FLAG_SKIP_DNS_SERVER;
        DWORD dw_ret_val = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, p_addresses, &out_buf_len);
        if (dw_ret_val == ERROR_BUFFER_OVERFLOW)
        {
            std::free(p_addresses);
            p_addresses = (PIP_ADAPTER_ADDRESSES)std::malloc(out_buf_len);
        }
        if (p_addresses && GetAdaptersAddresses(AF_UNSPEC, flags, NULL, p_addresses, &out_buf_len) == NO_ERROR)
        {
            for (PIP_ADAPTER_ADDRESSES p_curr_addresses = p_addresses; p_curr_addresses != nullptr; p_curr_addresses = p_curr_addresses->Next)
            {
                std::cout << "FriendlyName: " << pwchar_to_utf8(p_curr_addresses->FriendlyName) << "\n";
                std::cout << "Description: " << pwchar_to_utf8(p_curr_addresses->Description) << "\n";
                std::cout << "IfType: " << p_curr_addresses->IfType << "\n";
                std::cout << "AdapterName: " << p_curr_addresses->AdapterName << "\n";
                std::cout << "-------------------------------------------\n";
            }
        }
        std::free(p_addresses);
    }
    return 0;
}
