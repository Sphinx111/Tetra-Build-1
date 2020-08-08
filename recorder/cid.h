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
#ifndef CID_H
#define CID_H
#include <cstdint>
#include <string>

using namespace::std;

/**
 * @brief Short Subscriber Identity structure with last seen time for clean up
 *
 */

struct ssi_t {
    time_t last_seen;
    uint32_t ssi;
};

class call_identifier_t;                                                        // forward declaration

void cid_init(int raw_format_flag);
call_identifier_t * get_cid(int index);
void cid_clear();
void cid_parse_pdu(string data, FILE * fd_log);

#endif /* CID_H */
