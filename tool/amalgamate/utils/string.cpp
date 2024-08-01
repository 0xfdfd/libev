#include <algorithm>
#include <cctype>
#include "string.hpp"

am::StringVec am::split_lines(const std::string& str)
{
    am::StringVec lines;
    std::string::size_type start = 0;
    std::string::size_type end = str.find('\n');

    while (end != std::string::npos)
    {
        lines.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find('\n', start);
    }

    // Add the last line if there is one
    lines.push_back(str.substr(start));

    return lines;
}

bool am::start_with(const std::string str, const std::string& prefix)
{
    if (prefix.size() > str.size())
    {
        return false;
    }
    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

std::string am::remove_leading_spaces(const std::string& str)
{
    size_t idx = 0;
    for (; idx < str.size(); idx++)
    {
        if (!std::isspace(str[idx]))
        {
            break;
        }
    }

    return str.substr(idx);
}

std::string am::remove_trailing_spaces(const std::string& str)
{
    if (str.size() == 0)
    {
        return str;
    }

    size_t pos = str.size() - 1;
    while (pos > 0)
    {
        if (!std::isspace(str[pos]))
        {
            return str.substr(0, pos + 1);
        }
        pos--;
    }

    if (std::isspace(str[0]))
    {
        return std::string();
    }
    return str.substr(0, 1);
}

std::string am::convert_into_unix(const std::string& str)
{
    std::string ret = str;
    ret.erase(std::remove(ret.begin(), ret.end(), '\r'), ret.end());
    return ret;
}
