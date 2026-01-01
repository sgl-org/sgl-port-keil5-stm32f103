/* source/core/sgl_snprintf.c
 *
 * MIT License
 *
 * Copyright(c) 2023-present All contributors of SGL  
 * Document reference link: docs directory
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>


/**
 * @brief append a character to the buffer
 * @param buf buffer
 * @param size buffer size
 * @param pos current position
 * @param c character to append
 */
static inline void append_char(char *buf, size_t size, size_t *pos, char c)
{
    if (*pos + 1 < size) {
        buf[*pos] = c;
    }
    (*pos)++;
}


/**
 * @brief append a string to the buffer
 * @param buf buffer
 * @param size buffer size
 * @param pos current position
 * @param s string to append
 */
static inline void append_str(char *buf, size_t size, size_t *pos, const char* s)
{
    while (*s) append_char(buf, size, pos, *s++);
}

/**
 * @brief append an integer to the buffer
 * @param buf buffer
 * @param size buffer size
 * @param pos current position
 * @param val integer to append
 */
static inline void append_int(char *buf, size_t size, size_t *pos, int val)
{
    char tmp[32];
    int i = 0;
    bool neg = val < 0;

    if (neg) val = -val;

    do {
        tmp[i++] = '0' + (val % 10);
        val /= 10;
    } while (val);

    if (neg) tmp[i++] = '-';

    while (i--) append_char(buf, size, pos, tmp[i]);
}

/**
 * @brief append a hexadecimal number to the buffer
 * @param buf buffer
 * @param size buffer size
 * @param pos current position
 * @param val hexadecimal number to append
 * @param upper whether to use uppercase letters
 */
static inline void append_hex(char *buf, size_t size, size_t *pos, unsigned int val, bool upper)
{
    char tmp[32];
    int i = 0;
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";

    do {
        tmp[i++] = digits[val & 0xF];
        val >>= 4;
    } while (val);

    while (i--) append_char(buf, size, pos, tmp[i]);
}

/**
 * @brief append a float to the buffer
 * @param buf buffer
 * @param size buffer size
 * @param pos current position
 * @param val float to append
 * @param precision number of decimal places (-1 for default of 6)
 */
static void append_float(char *buf, size_t size, size_t *pos, double val, int precision)
{
    int int_part = (int)val;
    double frac = val - int_part;

    if (val < 0) {
        append_char(buf, size, pos, '-');
        int_part = -int_part;
        frac = -frac;
    }

    append_int(buf, size, pos, int_part);
    append_char(buf, size, pos, '.');

    int prec = (precision >= 0) ? precision : 6;
    for (int i = 0; i < prec; i++) {
        frac *= 10;
        int d = (int)frac;
        append_char(buf, size, pos, '0' + d);
        frac -= d;
    }
}

/**
 * @brief format a string, a simple version of vsnprintf
 * @param buf buffer
 * @param size buffer size
 * @param fmt format string
 * @param ap argument list
 * @return number of characters written
 */
int sgl_vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
    size_t pos = 0;

    while (*fmt) {
        char c = *fmt;

        if (c != '%') {
            append_char(buf, size, &pos, c);
            fmt++;
            continue;
        }

        fmt++;
        int precision = -1;

        // Parse precision (e.g., %.2f)
        if (*fmt == '.') {
            fmt++;
            precision = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                precision = precision * 10 + (*fmt - '0');
                fmt++;
            }
        }

        char spec = *fmt;

        switch (spec) {
        case 's': {
            const char *s = va_arg(ap, const char*);
            append_str(buf, size, &pos, s);
            break;
        }

        case 'd': {
            int d = va_arg(ap, int);
            append_int(buf, size, &pos, d);
            break;
        }

        case 'x': {
            unsigned int x = va_arg(ap, unsigned int);
            append_hex(buf, size, &pos, x, false);
            break;
        }

        case 'X': {
            unsigned int x = va_arg(ap, unsigned int);
            append_hex(buf, size, &pos, x, true);
            break;
        }

        case 'f': {
            double f = va_arg(ap, double);
            append_float(buf, size, &pos, f, precision);
            break;
        }

        case 'c': {
            char ch = (char)va_arg(ap, int);
            append_char(buf, size, &pos, ch);
            break;
        }

        case '%': {
            append_char(buf, size, &pos, '%');
            break;
        }

        default:
            append_char(buf, size, &pos, '%');
            append_char(buf, size, &pos, spec);
            break;
        }

        fmt++;
    }

    if (size > 0) {
        buf[(pos < size) ? pos : (size - 1)] = '\0';
    }

    return (int)pos;
}

/**
 * @brief format a string, a simple version of snprintf
 * @param buf buffer
 * @param size buffer size
 * @param fmt format string
 * @param ... arguments
 * @return number of characters written
 */
int sgl_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = sgl_vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}
