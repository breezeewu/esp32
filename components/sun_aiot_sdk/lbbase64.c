#include<stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lbbase64.h"
#define u_int32_t unsigned int
#define u_int8_t unsigned char


#ifndef UINT_MAX
#define UINT_MAX 0xffffffff
#endif

#ifndef AV_RB32
#   define AV_RB32(x)                                \
    (((uint32_t)((const u_int8_t*)(x))[0] << 24) |    \
               (((const u_int8_t*)(x))[1] << 16) |    \
               (((const u_int8_t*)(x))[2] <<  8) |    \
                ((const u_int8_t*)(x))[3])
#endif

/**
 * Calculate the output size needed to base64-encode x bytes to a
 * null-terminated string.
 */
#define SRS_AV_BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)

#ifndef AV_WL32
#   define AV_WL32(p, darg) do {                \
        unsigned d = (darg);                    \
        ((u_int8_t*)(p))[0] = (d);               \
        ((u_int8_t*)(p))[1] = (d)>>8;            \
        ((u_int8_t*)(p))[2] = (d)>>16;           \
        ((u_int8_t*)(p))[3] = (d)>>24;           \
    } while(0)
#endif

#   define AV_WN(s, p, v) AV_WL##s(p, v)

#   if    defined(AV_WN32) && !defined(AV_WL32)
#       define AV_WL32(p, v) AV_WN32(p, v)
#   elif !defined(AV_WN32) &&  defined(AV_WL32)
#       define AV_WN32(p, v) AV_WL32(p, v)
#   endif

#ifndef AV_WN32
#   define AV_WN32(p, v) AV_WN(32, p, v)
#endif

#define AV_BSWAP16C(x) (((x) << 8 & 0xff00)  | ((x) >> 8 & 0x00ff))
#define AV_BSWAP32C(x) (AV_BSWAP16C(x) << 16 | AV_BSWAP16C((x) >> 16))

#ifndef av_bswap32
static const u_int32_t av_bswap32(u_int32_t x)
{
    return AV_BSWAP32C(x);
}
#endif

#define av_be2ne32(x) av_bswap32(x)

/**
 * @file
 * @brief Base64 encode/decode
 * @author Ryan Martell <rdm4@martellventures.com> (with lots of Michael)
 */

/* ---------------- private code */
static const u_int8_t map2[256] =
{
    0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff,

    0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff,
    0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b,
    0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
    0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33,

                      0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

#define BASE64_DEC_STEP(i) do { \
    bits = map2[in[i]]; \
    if (bits & 0x80) \
        goto out ## i; \
    v = i ? (v << 6) + bits : bits; \
} while(0)

int lbbase64_decode(const char* in_str, unsigned char* out, int out_size)
{
    u_int8_t *dst = out;
    u_int8_t *end = out + out_size;
    //printf("lbbase64_decode(in_str:%s, out:%p, out_size:%d)\n", in_str, out, out_size);
    // no sign extension
    const u_int8_t *in = (const u_int8_t*)in_str;
    unsigned bits = 0xff;
    unsigned v = 0;

    while (end - dst > 3) {
        BASE64_DEC_STEP(0);
        BASE64_DEC_STEP(1);
        BASE64_DEC_STEP(2);
        BASE64_DEC_STEP(3);
        // Using AV_WB32 directly confuses compiler
        v = av_be2ne32(v << 8);
        AV_WN32(dst, v);
        dst += 3;
        in += 4;
    }
    if (end - dst) {
        BASE64_DEC_STEP(0);
        BASE64_DEC_STEP(1);
        BASE64_DEC_STEP(2);
        BASE64_DEC_STEP(3);
        *dst++ = v >> 16;
        if (end - dst)
            *dst++ = v >> 8;
        if (end - dst)
            *dst++ = v;
        in += 4;
    }
    while (1) {
        BASE64_DEC_STEP(0);
        in++;
        BASE64_DEC_STEP(0);
        in++;
        BASE64_DEC_STEP(0);
        in++;
        BASE64_DEC_STEP(0);
        in++;
    }

out3:
    *dst++ = v >> 10;
    v <<= 2;
out2:
    *dst++ = v >> 4;
out1:
out0:
    return bits & 1 ? -1 : dst - out;
}

/*****************************************************************************
* b64_encode: Stolen from VLC's http.c.
* Simplified by Michael.
* Fixed edge cases and made it work from data (vs. strings) by Ryan.
*****************************************************************************/

char* lbbase64_encode(char* out, int out_size, const unsigned char* in, int in_size)
{
    static const char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *ret, *dst;
    unsigned i_bits = 0;
    int i_shift = 0;
    int bytes_remaining = in_size;

    if (in_size >= (int)(UINT_MAX / 4) ||
        out_size < SRS_AV_BASE64_SIZE(in_size))
        return NULL;
    ret = dst = out;
    while (bytes_remaining > 3) {
        i_bits = AV_RB32(in);
        in += 3; bytes_remaining -= 3;
        *dst++ = b64[ i_bits>>26        ];
        *dst++ = b64[(i_bits>>20) & 0x3F];
        *dst++ = b64[(i_bits>>14) & 0x3F];
        *dst++ = b64[(i_bits>>8 ) & 0x3F];
    }
    i_bits = 0;
    while (bytes_remaining) {
        i_bits = (i_bits << 8) + *in++;
        bytes_remaining--;
        i_shift += 8;
    }
    while (i_shift > 0) {
        *dst++ = b64[(i_bits << 6 >> i_shift) & 0x3f];
        i_shift -= 6;
    }
    while ((dst - ret) & 3)
        *dst++ = '=';
    *dst = '\0';

    return ret;
}