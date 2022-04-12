#ifndef __EV_VERSION_H__
#define __EV_VERSION_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_VERSION Version
 * @{
 */

/**
 * @brief Major version.
 */
#define EV_VERSION_MAJOR            0

/**
 * @brief Minor version.
 */
#define EV_VERSION_MINOR            0

/**
 * @brief Patch version.
 */
#define EV_VERSION_PATCH            3

/**
 * @brief Development version.
 */
#define EV_VERSION_PREREL           7

/**
 * @brief Version calculate helper macro.
 * @param[in] a     Major version.
 * @param[in] b     Minor version.
 * @param[in] c     Patch version.
 * @param[in] d     Development version.
 */
#define EV_VERSION(a, b, c, d)      (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))

/**
 * @brief Current version code.
 */
#define EV_VERSION_CODE             \
    EV_VERSION(EV_VERSION_MAJOR, EV_VERSION_MINOR, EV_VERSION_PATCH, EV_VERSION_PREREL)

/**
 * @brief Get version code as c string.
 * @return      Version code.
 */
const char* ev_version_str(void);

/**
 * @brief Get version code as number.
 * @return      Version code
 */
unsigned ev_version_code(void);

/**
 * @} EV_VERSION
 */

#ifdef __cplusplus
}
#endif

#endif
