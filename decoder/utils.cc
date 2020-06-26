/*
 *  tetra-kit
 *  Copyright (C) 2020  LarryTh <dev@logami.fr>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "utils.h"

/**
 * @brief Get uint64_t value from uint8_t vector
 *
 */

uint64_t get_value(vector<uint8_t> vec, uint64_t start_pos_in_vector, uint8_t field_len)
{
    uint64_t val = 0;

    for (uint64_t pos = 0; (pos < field_len) && (pos + start_pos_in_vector < vec.size()); pos++)
    {
        val += (vec[pos + start_pos_in_vector] << (field_len - pos - 1));
    }

    return val;
}

/**
 * @brief Get uint64_t value from uint8_t vector
 *
 */

uint64_t get_value_64(vector<uint8_t> vec, uint64_t start_pos_in_vector, uint8_t field_len)
{
    return get_value(vec, start_pos_in_vector, field_len);
}

/**
 * @brief Get uint32_t value from uint8_t vector
 *
 */

uint32_t get_value_32(vector<uint8_t> vec, uint64_t start_pos_in_vector, uint8_t field_len)
{
    uint32_t ret = get_value(vec, start_pos_in_vector, field_len);

    return ret;
}

/**
 * @brief Get uint16_t value from uint8_t vector
 *
 */

uint16_t get_value_16(vector<uint8_t> vec, uint64_t start_pos_in_vector, uint8_t field_len)
{
    uint16_t ret = get_value(vec, start_pos_in_vector, field_len);

    return ret;
}

/**
 * @brief Get uint8_t value from uint8_t vector
 *
 */

uint8_t get_value_8(vector<uint8_t> vec, uint64_t start_pos_in_vector, uint8_t field_len)
{
    uint8_t ret = get_value(vec, start_pos_in_vector, field_len);

    return ret;
}

/**
 * @brief Extract uint8_t vector from uint8_t vector
 *
 */

vector<uint8_t> vector_extract(vector<uint8_t> source, uint32_t pos, int32_t length)
{
    vector<uint8_t> ret;

    if (length > 0)                                                             // check if invalid length requested
    {
        int32_t len = (int32_t)source.size() - (int32_t)pos;                    // actual remaining bytes in vector after pos

        if (len > 0)                                                            // check if actual length is valid
        {
            if (len > length)                                                   // we have more bytes than requested
            {
                len = length;                                                   // so return only the requested ones
            }

            std::copy(source.begin() + pos, source.begin() + pos + (uint32_t)len, back_inserter(ret));
        }
    }
    
    return ret;
}

/**
 * @brief Concatenate two uint8_t vectors
 *
 */

vector<uint8_t> vector_append(vector<uint8_t> vec1, vector<uint8_t> vec2)
{
    vector<uint8_t> ret(vec1);

    ret.insert(ret.end(), vec2.begin(), vec2.end());

    return ret;
}

/**
 * @brief Print vector
 *
 */

void print_vector(vector<uint8_t> data, int len)
{
    size_t count = data.size();

    if (len < (int)count)
    {
        count = (size_t)len;
    }

    for (size_t idx = 0; idx < count; idx++)
    {
        cout << (char)(data[idx] + '0');
    }

    if (count > 0)
    {
        cout << endl;
    }
}

/**
 * @brief Convert binary vector to text string 0/1
 *
 */

string vector_to_string(vector<uint8_t> data, int len)
{
    string res = "";

    size_t count = data.size();

    if (len < (int)count)
    {
        count = (size_t)len;
    }

    for (size_t idx = 0; idx < count; idx++)
    {
        res += (char)(data[idx] + '0');
    }

    return res;
}

/**
 * @brief Return pattern/data comparison errors count at position in data vector
 *
 * @param[in] data      Vector to look in from pattern
 * @param[in] pattern   Pattern to search
 * @param[in] position  Position in vector to start search
 *
 * @return Score based on similarity with pattern (differences count between vector and pattern)
 *
 */

int pattern_at_position_score(vector<uint8_t> data, vector<uint8_t> pattern, uint32_t position)
{
    int errors = 0;

    for (unsigned int idx = 0; idx < pattern.size(); idx++)
    {
        errors += (pattern[idx] ^ data[position + idx]);
    }

    return errors;
}

/**
 * @brief Convert binary value to TETRA (external subscriber) digit
 *
 */

 char get_tetra_digit(const uint8_t val)
{
    const char digits[14] = "0123456789*#+";
    char res;

    if (val < 13)
    {
        res = digits[val];
    }
    else
    {
        res = '?';
    }

    return res;
}

/**
 * @brief Decode 7-bit alphabet 29.5.4.3
 *
 */

string text_gsm_7_bit_decode(vector<uint8_t> data, const int16_t len)
{
    // NOTE: _ is a special char when we want to escape the character value
    //                   0        10         20        30         40        50        60        70        80        90        100       110      120
    //                   0123456789012345678901234567890123 4567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567
    const string gsm7 = "@_______________________________ !\"#{%&'()*+,-./0123456789:;<=>?_ABCDEFGHIJKLMNOPQRSTUVWXYZ______abcdefghijklmnopqrstuvwxyz_____";
    string res = "";

    for (int16_t idx = 0; idx < len / 7; idx++)
    {
        uint8_t chr = get_value(data, (uint64_t)idx * 7, 7);

        char val = gsm7[chr];
        if (val == '{')
        {
            char buf[16] = "";
            sprintf(buf, "0x%02x", chr);
            res += buf;
        }
        else
        {
            res += (char)chr;
        }
    }

    return res;
}

/**
 * @brief Decode 8-bit alphabets TODO very rough function only, doesn't check alphabet input type
 *        Unknown symbols are replaced with '_' since we already have hex dump
 *
 */

string text_generic_8_bit_decode(vector<uint8_t> data, const int16_t len)
{
    string res = "";

    for (int16_t idx = 0; idx < len / 8; idx++)
    {
        uint8_t chr = get_value(data, (uint64_t)idx * 8, 8);
        if (isprint(chr))
        {
            res += (char)chr;
        }
        else
        {
            res += '_';
        }
    }

    return res;
}

/**
 * @brief NMEA location decode (NMEA 0183)
 *        ASCII-8 plain text with CR/LF endline
 *        with optional checksum separated by *
 *
 */

string location_nmea_decode(vector<uint8_t> data, const int16_t len)
{
    string res = "";

    for (int16_t idx = 0; idx < len / 8; idx++)
    {
        uint8_t chr = get_value(data, (uint64_t)idx * 8, 8);

        if ((chr != 10) && (chr != 13))                                         // skip CR/LF
        {
            res += (char)chr;
        }
    }

    return res;
}

/**
 * @brief Decode floating point value coded as 2's complement integer
 *
 */

double utils_decode_integer_twos_complement(uint32_t data, uint8_t n_bits, double mult)
{
    double res;

    uint32_t val = data;

    if (val & (1 << (n_bits - 1)))                                              // negative value, take the 2's complement
    {
        val = ~val;                                                             // flip bits
        val += 1;                                                               // add one
        val &= (0xFFFFFFFF >> (32 - n_bits));                                   // mask bits

        res = val * (-mult) / (double)(1 << (n_bits - 1));
    }
    else                                                                        // positive value
    {
        res = val * mult / (double)(1 << (n_bits - 1));
    }

    return res;
}

/**
 * @brief Substract two 32 bits unsigned integers (val1 - val2) and return signed int 32
 *
 */

int32_t utils_substract(int32_t val1, int32_t val2)
{
    int32_t res = (int32_t)val1 - (int32_t)val2;

    return res;
}

/**
 * @brief Print formatted string like printf with variadic protection
 *
 */


string format_str(const char *fmt, ...)
{
    // variadic format function to string
    const size_t BUF_LEN = 8192;
    char buf[BUF_LEN];

    va_list args;
    va_start(args, fmt);

    vsnprintf(buf, BUF_LEN - 1, fmt, args);

    va_end(args);

    return string(buf);
}
