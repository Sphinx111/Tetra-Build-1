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
#ifndef MAC_DEFRAG_H
#define MAC_DEFRAG_H
#include <cstdint>
#include <vector>
#include <string>
#include "tetra_common.h"

using namespace std;

/**
 * @brief MAC defragmenter
 *
 */

class mac_defrag_t {
public:
    mac_defrag_t(int debug_level);
    ~mac_defrag_t();

    mac_address_t mac_address;                                                  // MAC address
    tetra_time_t  start_time;                                                   // start time of defragemnter (will be used to stop on missing/invalid end frag packet receive)

    vector<uint8_t> mac_ressource;
    vector<uint8_t> tm_sdu;                                                     // reconstructed TM-SDU to be transfered to LLC

    int g_debug_level;
    bool b_stopped;
    uint8_t fragments_count;
    void start(const mac_address_t address, const tetra_time_t time_slot);
    void append(const vector<uint8_t> sdu, const mac_address_t address);
    vector<uint8_t> get_sdu();
    void stop();
};

#endif /* MAC_DEFRAG_H */
