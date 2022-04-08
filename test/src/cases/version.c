#include "ev.h"
#include "test.h"

TEST(version, str)
{
    char buffer[64];
#if EV_VERSION_PREREL
    snprintf(buffer, sizeof(buffer), "%d.%d.%d-dev%d", EV_VERSION_MAJOR, EV_VERSION_MINOR, EV_VERSION_PATCH, EV_VERSION_PREREL);
#else
    snprintf(buffer, sizeof(buffer), "%d.%d.%d", EV_VERSION_MAJOR, EV_VERSION_MINOR, EV_VERSION_PATCH);
#endif

    ASSERT_EQ_STR(buffer, ev_version_str());
}

TEST(version, code)
{
    ASSERT_EQ_U32(EV_VERSION_CODE, ev_version_code());
}
