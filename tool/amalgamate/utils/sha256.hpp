#ifndef EV_AMALGAMATE_SHA256_HPP
#define EV_AMALGAMATE_SHA256_HPP

#include <string>

namespace am {

/**
 * @brief Calcuate sha256
 * @param[in] data - Data to calculate.
 * @return Sha256 in string.
 */
std::string sha256(const std::string& data);

}

#endif
