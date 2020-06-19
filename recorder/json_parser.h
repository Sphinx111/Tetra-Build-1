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
#ifndef JSON_PARSER_H
#define JSON_PARSER_H
#include <cstdint>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

using namespace::std;

/**
 * @brief Json object parser with error handling
 *
 */

class json_parser_t {
public:
    json_parser_t(string data);
    ~json_parser_t();

    bool is_valid();
    bool read(const string field, string   * result);
    bool read(const string field, uint8_t  * result);
    bool read(const string field, uint16_t * result);
    bool read(const string field, uint32_t * result);
    bool read(const string field, uint64_t * result);

    void write_report(FILE * fd_file);
    string to_string();
private:
    rapidjson::Document jdoc;
    bool b_valid;
};

#endif /* JSON_PARSER_H */
