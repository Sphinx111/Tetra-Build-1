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
#include "tetra_dl.h"
#include "utils.h"

/**
 * @brief SNDCP service entry point - see 28.4
 *
 */

void tetra_dl::service_sndcp(std::vector<uint8_t> pdu, mac_logical_channel_t mac_logical_channel)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - mac_channel = %s pdu = %s\n", "service_sndcp", mac_logical_channel_name(mac_logical_channel).c_str(), vector_to_string(pdu, pdu.size()).c_str());
        fflush(stdout);
    }

    report_start("SNDCP", "RAW-DATA");
    report_add("data", pdu);
    report_send();

    //uint32_t pos = 0;
}
