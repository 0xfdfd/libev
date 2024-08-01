#ifndef EV_AMALGAMATE_FILE_HPP
#define EV_AMALGAMATE_FILE_HPP

#include <string>

namespace am {

/**
 * @brief Read the whole file.
 * @param[in] path - Path to file.
 * @return Content of file.
 */
std::string read_file(const std::string& path);

/**
 * @brief Write data into file.
 * @param[in] path - Path to file.
 * @param[in] content - Content.
 */
void write_file(const std::string& path, const std::string& content);

/**
 * @breif Get filename from path.
 * @param[in] path - File path.
 * @return File name.
 */
std::string file_name(const std::string& path);

}

#endif
