#include "smart_pong_ai.h"

#include <string.h>

uint32_t smart_pong_crc32(const void* data, uint32_t len)
{
    const unsigned char* p = (const unsigned char*)data;
    uint32_t crc = 0xFFFFFFFFu;

    for (uint32_t i = 0; i < len; ++i)
    {
        crc ^= (uint32_t)p[i];
        for (uint32_t bit = 0; bit < 8u; ++bit)
        {
            const uint32_t mask = (uint32_t)(-(int32_t)(crc & 1u));
            crc = (crc >> 1u) ^ (0xEDB88320u & mask);
        }
    }

    return ~crc;
}

bool smart_pong_learning_pack(const smart_pong_learning_state_t* st, uint32_t version, smart_pong_learning_blob_t* blob)
{
    if (!st || !blob) return false;
    memset(blob, 0, sizeof(*blob));
    blob->magic = SMART_PONG_PERSIST_MAGIC;
    blob->version = version;
    blob->state = *st;
    blob->crc32 = smart_pong_crc32(blob, (uint32_t)(sizeof(*blob) - sizeof(blob->crc32)));
    return true;
}

bool smart_pong_learning_unpack(const smart_pong_learning_blob_t* blob, uint32_t expected_version, smart_pong_learning_state_t* st)
{
    if (!blob || !st) return false;
    if (blob->magic != SMART_PONG_PERSIST_MAGIC) return false;
    if (blob->version != expected_version) return false;

    const uint32_t crc = smart_pong_crc32(blob, (uint32_t)(sizeof(*blob) - sizeof(blob->crc32)));
    if (crc != blob->crc32) return false;

    *st = blob->state;
    return true;
}
