#ifndef _BASE_64_ENC_H_
#define _BASE_64_ENC_H_

#ifdef ARDUINO
#include "Arduino.h"
#endif

#include <cstdint>

class Base64StreamEncoder
{
private:
    uint32_t m_b64sr_r, m_b64sr_w;
    uint8_t m_b64sr_r_i, m_b64sr_w_i;
    bool hit_end;

    void(*m_write_cb)(char);
    char(*m_read_cb)(void);
    int(*m_avail_cb)(void);

#ifndef B64_STR_LOOKUP
    static const char char_6263[2];
#else
    char b64lookupstr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint8_t b64rlookuparray[] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0,
        0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0
    };
#endif

public:
    static const char start_delim;
    static const char end_delim;
    static const char pad_char;

    static char b64lookup(uint8_t data_6bit);
    static uint8_t b64reverselookup(char b64char);

    Base64StreamEncoder();

    void write(uint8_t data);
    void write(const uint8_t * i_data, uint32_t len);
    void flush();

    void setWriteCallback(void(*write_cb)(char));
    void setReadCallback(char(*read_cb)(void));
    void setAvailableCallback(int(*avail_cb)(void));

    uint32_t decodeBuffer(uint8_t* dest, uint32_t destlen, const uint8_t* src, uint32_t srclen);
    uint32_t encodeBuffer(uint8_t* dest, uint32_t destlen, const uint8_t* src, uint32_t srclen);

    int available();

    uint8_t read();
    uint32_t read(uint8_t * o_data, uint32_t len);
    void flushr();
};

#endif // _BASE_64_ENC_H_
