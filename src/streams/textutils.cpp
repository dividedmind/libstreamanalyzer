/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Jos van den Oever <jos@vandenoever.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "jstreamsconfig.h"
#include "textutils.h"

/**
 * Return the position of the first byte that is not valid
 * utf8. Return value of 0 means that the entire string is valid.
 * If the last character is incomplete the returned value points to the start
 * of that character and nb is set to the number of characters that is missing.
 **/
bool
jstreams::checkUtf8(const char* p, int32_t length) {
    const char* end = p + length;
    // check if the text is valid UTF-8
    char nb = 0;
    while (p < end) {
        unsigned char c = *p;
        if (nb) {
            if ((0xC0 & c) != 0x80) {
                return false;
            }
            nb--;
        } else if ((0xE0 & c) == 0xC0) {
            nb = 1;
        } else if ((0xF0 & c) == 0xE0) {
            nb = 2;
        } else if (c >= 0xF0 && c <= 0xF4) {
            nb = 3;
        } else if (c > 0x7F
                || (c < 0x20 && !(c == 0x9 || c == 0xA || c == 0xD))) {
            return false;
        }
        p++;
    }
    // the string is only valid utf8 if it contains only complete characters
    return nb == 0;
}
bool
jstreams::checkUtf8(const std::string& p) {
    return checkUtf8(p.c_str(), p.size());
}

// http://www.w3.org/TR/REC-xml/#charsets
/**
 * Return the position of the first byte that is not valid
 * utf8. Return value of 0 means that the entire string is valid.
 * If the last character is incomplete the returned value points to the start
 * of that character and nb is set to the number of characters that is missing.
 **/
const char*
jstreams::checkUtf8(const char* p, int32_t length, char& nb) {
    const char* end = p + length;
    const char* cs = p;
    // check if the text is valid UTF-8
    nb = 0;
    while (p < end) {
        unsigned char c = *p;
        if (nb) {
            if ((0xC0 & c) != 0x80) {
                nb = 0;
                return p;
            }
            nb--;
        } else if ((0xE0 & c) == 0xC0) {
            cs = p;
            nb = 1;
        } else if ((0xF0 & c) == 0xE0) {
            cs = p;
            nb = 2;
        } else if (c >= 0xF0 && c <= 0xF4) {
            cs = p;
            nb = 3;
        } else if (c > 0x7F
                || (c < 0x20 && !(c == 0x9 || c == 0xA || c == 0xD))) {
            return p;
        }
        p++;
    }
    // the string is only valid utf8 if it contains only complete characters
    return (nb) ?cs :0;
}
const char*
jstreams::checkUtf8(const std::string& p, char& nb) {
    return checkUtf8(p.c_str(), p.size(), nb);
}

/**
 * convert the \r and \n in utf8 strings into spaces
 **/
void
jstreams::convertNewLines(char* p) {
    int32_t len = strlen(p);
    bool ok = checkUtf8(p, len);
    if (!ok) fprintf(stderr, "string is not valid utf8\n");

    const char* end = p+len;
    char nb = 0;
    while (p < end) {
        unsigned char c = *p;
        if (nb) {
            if ((0xC0 & c) != 0x80) {
                return;
            }
            nb--;
        } else if ((0xE0 & c) == 0xC0) {
            nb = 1;
        } else if ((0xF0 & c) == 0xE0) {
            nb = 2;
        } else if (c >= 0xF0 && c <= 0xF4) {
            nb = 3;
        } else if (c == '\n' || c == '\r') {
            *p = ' ';
        }
        p++;
    }
}

#define swap16(x) \
      ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
 
#define swap32(x) \
      ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) \
     | (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))
#define swap64(x) \
      ((((x) & 0xff00000000000000ull) >> 56)  \
     | (((x) & 0x00ff000000000000ull) >> 40)  \
     | (((x) & 0x0000ff0000000000ull) >> 24)  \
     | (((x) & 0x000000ff00000000ull) >> 8)   \
     | (((x) & 0x00000000ff000000ull) << 8)   \
     | (((x) & 0x0000000000ff0000ull) << 24)  \
     | (((x) & 0x000000000000ff00ull) << 40)  \
     | (((x) & 0x00000000000000ffull) << 56))

#ifdef __BIG_ENDIAN__
int16_t
jstreams::readLittleEndianInt16(const char* c) {
    return swap16(*reinterpret_cast<const int16_t*>(c));
}
uint16_t
jstreams::readLittleEndianUInt16(const char* c) {
    return swap16(*reinterpret_cast<const uint16_t*>(c));
}
int32_t
jstreams::readLittleEndianInt32(const char* c) {
    return swap32(*reinterpret_cast<const int32_t*>(c));
}
uint32_t
jstreams::readLittleEndianUInt32(const char* c) {
    return swap32(*reinterpret_cast<const uint32_t*>(c));
}
int64_t
jstreams::readLittleEndianInt64(const char* c) {
    return swap64(*reinterpret_cast<const int64_t*>(c));
}
uint64_t
jstreams::readLittleEndianUInt64(const char* c) {
    return swap64(*reinterpret_cast<const uint64_t*>(c));
}
#else
int16_t
jstreams::readBigEndianInt16(const char* c) {
    return swap16(*reinterpret_cast<const int16_t*>(c));
}
uint16_t
jstreams::readBigEndianUInt16(const char* c) {
    return swap16(*reinterpret_cast<const uint16_t*>(c));
}
int32_t
jstreams::readBigEndianInt32(const char* c) {
    return swap32(*reinterpret_cast<const int32_t*>(c));
}
uint32_t
jstreams::readBigEndianUInt32(const char* c) {
    return swap32(*reinterpret_cast<const uint32_t*>(c));
}
int64_t
jstreams::readBigEndianInt64(const char* c) {
    return swap64(*reinterpret_cast<const int64_t*>(c));
}
uint64_t
jstreams::readBigEndianUInt64(const char* c) {
    return swap64(*reinterpret_cast<const uint64_t*>(c));
}
#endif
