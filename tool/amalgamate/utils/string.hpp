#ifndef EV_AMALGAMATE_STRING_HPP
#define EV_AMALGAMATE_STRING_HPP

#include <string>
#include <vector>
#include <sstream>

namespace am {

/**
 * @brief String vector.
 */
typedef std::vector<std::string> StringVec;

/**
 * @brief Split data into lines.
 * @param[in] data - The conent to split
 * @return String vector.
 */
StringVec split_lines(const std::string& data);

/**
 * @brief Check if \p str start with \p prefix.
 * @param[in] str - The source string to match.
 * @param[in] prefix - The prefix string.
 * @return bool.
 */
bool start_with(const std::string str, const std::string& prefix);

/**
 * @brief Remove leading spaces.
 * @param[in] str - String.
 * @return String.
 */
std::string remove_leading_spaces(const std::string& str);

/**
 * @brief Remove trailing spaces.
 * @param[in] str - String.
 * @return String.
 */
std::string remove_trailing_spaces(const std::string& str);

/**
 * @brief Convert file format into unix line ending.
 * @param[in] str - Data to convert.
 * @return Converted data.
 */
std::string convert_into_unix(const std::string& str);

/**
 * @brief Convert value into std::string.
 * @param[in] v - The value to convert.
 * @return The converted string.
 */
template<typename T>
std::string to_string(const T& v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

}

#endif
