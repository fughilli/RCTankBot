#include "base64enc.h"

const char Base64StreamEncoder::char_6263[2] = {'+','/'};
const char Base64StreamEncoder::start_delim  = '$';
const char Base64StreamEncoder::end_delim    = '^';
const char Base64StreamEncoder::pad_char     = '=';

char Base64StreamEncoder::b64lookup(uint8_t data_6bit)
{
#ifdef B64_STR_LOOKUP
    return b64lookupstr[data_6bit & 0x3F];
#else
    data_6bit &= 0x3F;
    if(data_6bit < 26)
        return 'A'+data_6bit;
    if(data_6bit < 52)
        return 'a'+(data_6bit-26);
    if(data_6bit < 62)
        return '0'+(data_6bit-52);

    return char_6263[data_6bit-62];
#endif // B64_STR_LOOKUP
}

uint8_t Base64StreamEncoder::b64reverselookup(char b64char)
{
#ifdef B64_STR_LOOKUP
    return b64rlookuparray[b64char & 0x7F];
#else
    b64char &= 0x7F;
    if(b64char >= 'A' && b64char <= 'Z')
        return b64char - 'A';
    if(b64char >= 'a' && b64char <= 'z')
        return b64char - 'a' + 26;
    if(b64char >= '0' && b64char <= '9')
        return b64char - '0' + 52;
    if(b64char == '+')
        return 62;
    if(b64char == '/')
        return 63;

    return 0;
#endif // B64_STR_LOOKUP
}


Base64StreamEncoder::Base64StreamEncoder()
{
    m_b64sr_w_i = 0;
    m_b64sr_w_i = 0;
    m_b64sr_w = 0;
    m_b64sr_w = 0;
    hit_end = false;
}

void Base64StreamEncoder::write(uint8_t data)
{
    m_b64sr_w <<= 8;
    m_b64sr_w |= data;
    m_b64sr_w_i += 2;


    m_write_cb(Base64StreamEncoder::b64lookup(m_b64sr_w >> m_b64sr_w_i));

    if(m_b64sr_w_i == 6)
    {
        m_write_cb(Base64StreamEncoder::b64lookup(m_b64sr_w));
        m_b64sr_w_i = 0;
    }
}

void Base64StreamEncoder::write(const uint8_t* i_data, uint32_t len)
{
    m_write_cb(start_delim);
    for(uint32_t i = 0; i < len; i++)
        write(i_data[i]);
    flush();
    m_write_cb(end_delim);
}

void Base64StreamEncoder::flush()
{
    if(m_b64sr_w_i)
    {
        m_write_cb(Base64StreamEncoder::b64lookup(m_b64sr_w << (6 - m_b64sr_w_i)));
        for(uint8_t i = 1; i < (8-m_b64sr_w_i)/2; i++)
        {
            m_write_cb('=');
        }
    }

    m_b64sr_w_i = 0;
}

void Base64StreamEncoder::flushr()
{
    m_b64sr_r_i = 0;

    if(available() == 0)
        return;

    while(m_avail_cb() > 0)
    {
        if(m_read_cb() == '^')
            break;
    }
}

int Base64StreamEncoder::available()
{
    return (m_avail_cb()*3)/4;
}

uint8_t Base64StreamEncoder::read()
{
    if(available() == 0)
        return 0;

    m_b64sr_r <<= 6;
    char rc = m_read_cb();
    if(rc == end_delim)
        return 0;
    m_b64sr_r |= Base64StreamEncoder::b64reverselookup(rc);
    if(m_b64sr_r_i == 0)
    {
        m_b64sr_r <<= 6;
        rc = m_read_cb();
        if(rc == end_delim)
            return 0;
        m_b64sr_r |= Base64StreamEncoder::b64reverselookup(rc);
        m_b64sr_r_i = 6;
    }
    m_b64sr_r_i -= 2;
    return(m_b64sr_r >> m_b64sr_r_i) & 0xFF;
}

uint32_t Base64StreamEncoder::read(uint8_t* o_data, uint32_t len)
{
    if(available() == 0)
        return 0;

    bool fail = true;
    while(fail && m_avail_cb() > 0)
    {
        if(m_read_cb() == '$')
            fail = false;
    }

    if(fail)
        return 0;

    uint32_t i;
    for(i = 0; i < len && available() > 0; i++)
    {
        o_data[i] = read();
        if(hit_end)
        {
            hit_end = false;
            break;
        }
    }
    return i;
}

void Base64StreamEncoder::setWriteCallback(void(* write_cb)(char))
{
    m_write_cb = write_cb;
}

void Base64StreamEncoder::setReadCallback(char(* read_cb)(void))
{
    m_read_cb = read_cb;
}

void Base64StreamEncoder::setAvailableCallback(int(* avail_cb)(void))
{
    m_avail_cb = avail_cb;
}

uint32_t Base64StreamEncoder::decodeBuffer(uint8_t* dest, uint32_t destlen, const uint8_t* src, uint32_t srclen)
{
    uint32_t srcidx = 0, destidx = 0;

    uint32_t partial = 0;
    uint32_t partial_idx = 0;

    while(srcidx < srclen)
    {
        partial <<= 6;
        char rc = src[srcidx++];
        if(rc == pad_char)
            return destidx;
        partial |= b64reverselookup(rc);
        if(partial_idx == 0)
        {
            if(srcidx == srclen)
                return 0;
            partial <<= 6;
            rc = src[srcidx++];
            if(rc == pad_char)
                return destidx;
            partial |= b64reverselookup(rc);
            partial_idx = 6;
        }
        partial_idx -= 2;
        if(destidx == destlen)
            return 0;
        dest[destidx++] = ((partial >> partial_idx) & 0xFF);
    }

    return destidx;
}

uint32_t Base64StreamEncoder::encodeBuffer(uint8_t* dest, uint32_t destlen, const uint8_t* src, uint32_t srclen)
{
    uint32_t srcidx = 0, destidx = 0;

    while(srcidx < (srclen * 8))
    {
        if(destidx == destlen)
            return 0;

        uint8_t offset = (srcidx % 8);
        uint8_t data = src[srcidx / 8];
        data >>= offset;

        if(offset > 2)
        {
            data |= (src[(srcidx / 8) + 1] << (8-offset));
        }

        dest[destidx++] = b64lookup(data);

        srcidx += 6;
    }

    return destidx;
}

