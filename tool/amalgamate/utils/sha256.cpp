#include "sha256.hpp"
#include "sha256.h"

std::string am::sha256(const std::string& data)
{
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (uint8_t*)data.c_str(), data.size());

    uint8_t sha256_bin[SHA256_BLOCK_SIZE];
    sha256_final(&ctx, sha256_bin);

    char buf[65]; size_t i;
    for (i = 0; i < sizeof(sha256_bin); i++)
    {
        snprintf(buf + 2 * i, sizeof(buf) - 2 * i, "%02x", sha256_bin[i]);
    }

    return buf;
}
