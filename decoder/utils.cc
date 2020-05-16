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

vector<uint8_t> vector_extract(vector<uint8_t> source, uint32_t pos, uint32_t length)
{
    uint32_t vec_len = source.size() - pos;

    if (length > vec_len)
    {
        length = vec_len;
    }

    vector<uint8_t> ret(source.begin() + pos, source.begin() + pos + length);
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
    for (int idx = 0; idx < len; idx++)
    {
        cout << (char)(data[idx] + '0');
    }

    if (len > 0)
    {
        cout << endl;
    }
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


