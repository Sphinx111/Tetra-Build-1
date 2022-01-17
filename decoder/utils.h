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
#ifndef UTILS_H
#define UTILS_H
#include <cstdint>
#include <cstdarg>
#include <iostream>
#include <vector>
#include <chrono>

uint64_t get_value(std::vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);
uint64_t get_value_64(std::vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);
uint32_t get_value_32(std::vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);
uint16_t get_value_16(std::vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);
uint8_t  get_value_8(std::vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);

std::vector<uint8_t> vector_extract(std::vector<uint8_t> source, uint32_t pos, int32_t length); // extract sub-vector
std::vector<uint8_t> vector_append(std::vector<uint8_t> vec1, std::vector<uint8_t> vec2);       // concatenate vectors
void print_vector(std::vector<uint8_t> data, int len);
std::string vector_to_string(std::vector<uint8_t> data, int len);

// miscellaneous functions
// TODO to sort
int pattern_at_position_score(std::vector<uint8_t> data, std::vector<uint8_t> pattern, uint32_t position);

char get_tetra_digit(const uint8_t val);

std::string text_gsm_7_bit_decode(std::vector<uint8_t> data, const int16_t len);
std::string text_generic_8_bit_decode(std::vector<uint8_t> data, const int16_t len);
std::string location_nmea_decode(std::vector<uint8_t> data, const int16_t len);

double utils_decode_integer_twos_complement(uint32_t data, uint8_t n_bits, double mult);
int32_t utils_substract(int32_t val1, int32_t val2);

std::string format_str(const char *fmt, ...);

#endif /* UTILS_H */
