#include "data/formats/rff.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>

namespace adam::modules::recrep
{
    namespace rff
    {
        // Trim-Helper
        static inline std::string trim(const std::string& s) 
        {
            size_t start = s.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) return "";
            size_t end = s.find_last_not_of(" \t\r\n");
            return s.substr(start, end - start + 1);
        }

        // Multiple spaces to single space (inner only)
        static inline std::string normalize_spaces(const std::string& s) 
        {
            std::string result;
            bool last_was_space = false;

            for (char c : s) 
            {
                if (std::isspace(static_cast<unsigned char>(c))) 
                {
                    if (!last_was_space)
                     {
                        result.push_back(' ');
                        last_was_space = true;
                    }
                } 
                else 
                {
                    result.push_back(c);
                    last_was_space = false;
                }
            }

            // Trim trailing space if created
            if (!result.empty() && result.back() == ' ')
                result.pop_back();
                
            return result;
        }

        time_string to_time_string(const std::tm& tm_val)
        {
            time_string ts;
            std::ostringstream oss;

            oss << std::put_time(&tm_val, time_format_string);
            std::string s = oss.str();

            for (size_t i = 0; i < sizeof(ts.text) && i < s.length(); ++i)
                ts.text[i] = s[i];
                
            return ts;
        }

        std::tm from_time_string(const time_string& ts)
        {
            std::tm tm_val{};
            tm_val.tm_isdst = -1;
            
            std::string raw(ts.text, sizeof(ts.text));
            
            std::string trimmed     = trim(raw);
            std::string normalized  = normalize_spaces(trimmed);
            
            std::istringstream iss(normalized);
            iss >> std::get_time(&tm_val, rff::time_format_string);
            
            if (iss.fail()) 
            {
                tm_val = {};
                tm_val.tm_isdst = -1;
            }
            
            return tm_val;
        }
    }
}
