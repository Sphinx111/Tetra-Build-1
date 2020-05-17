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
#include <iostream>
#include <vector>

using namespace std;

uint64_t get_value(vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);
uint64_t get_value_64(vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);
uint32_t get_value_32(vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);
uint16_t get_value_16(vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);
uint8_t  get_value_8(vector<uint8_t> v, uint64_t start_pos_in_vector, uint8_t field_len);

vector<uint8_t> vector_extract(vector<uint8_t> source, uint32_t pos, uint32_t length);        // extract sub-vector
vector<uint8_t> vector_append(vector<uint8_t> vec1, vector<uint8_t> vec2);                            // concatenate vectors
void print_vector(vector<uint8_t> data, int len);

// general functions
int pattern_at_position_score(vector<uint8_t> data, vector<uint8_t> pattern, uint32_t position);

#endif /* UTILS_H */
