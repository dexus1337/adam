#include "data/formats/rff.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>

namespace adam::modules::recrep
{
    namespace rff
    {
        time_string to_time_string(const std::tm& tm_val)
        {
            time_string ts;
            std::ostringstream oss;
            oss << std::put_time(&tm_val, time_format_string);
            std::string s = oss.str();
            for (size_t i = 0; i < sizeof(ts.text) && i < s.length(); ++i) {
                ts.text[i] = s[i];
            }
            return ts;
        }

        std::tm from_time_string(const time_string& ts)
        {
            std::tm tm_val{};
            std::string s;
            for (size_t i = 0; i < sizeof(ts.text) && ts.text[i] != '\0'; ++i) {
                s.push_back(ts.text[i]);
            }
            std::istringstream iss(s);
            iss >> std::get_time(&tm_val, time_format_string);
            return tm_val;
        }
    }
}
