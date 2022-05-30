#include "ev.h"
#include "test.h"

TEST_FIXTURE_SETUP(version)
{

}

TEST_FIXTURE_TEAREDOWN(version)
{

}

TEST_F(version, str)
{
    char buffer[64];
#if EV_VERSION_PREREL
    snprintf(buffer, sizeof(buffer), "%d.%d.%d-dev%d", EV_VERSION_MAJOR, EV_VERSION_MINOR, EV_VERSION_PATCH, EV_VERSION_PREREL);
#else
    snprintf(buffer, sizeof(buffer), "%d.%d.%d", EV_VERSION_MAJOR, EV_VERSION_MINOR, EV_VERSION_PATCH);
#endif

    ASSERT_EQ_STR(buffer, ev_version_str());
}

TEST_F(version, code)
{
    ASSERT_EQ_U32(EV_VERSION_CODE, ev_version_code());
}

TEST_F(version, compare)
{
    /* patch version compare */
    ASSERT_GT_U32(EV_VERSION(0, 0, 2, 0), EV_VERSION(0, 0, 1, 0));

    /* prerelease version always littler that release version */
    ASSERT_GT_U32(EV_VERSION(0, 0, 2, 0), EV_VERSION(0, 0, 2, 1));

    /* major version is larger than minor version */
    ASSERT_GT_U32(EV_VERSION(2, 0, 0, 0), EV_VERSION(1, 2, 0, 0));

    /* version macro should support compare in pre-process stage */
#if EV_VERSION(0, 0, 2, 0) > EV_VERSION(0, 0, 2, 1)
    ASSERT(1);
#else
    ASSERT(0);
#endif
}
