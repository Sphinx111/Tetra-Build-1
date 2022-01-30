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
 * @brief User-Plane traffic handling
 *
 */

void tetra_dl::service_u_plane(std::vector<uint8_t> pdu, mac_logical_channel_t mac_logical_channel)
{
    if (g_debug_level >= 5)
    {
        fprintf(stdout, "DEBUG ::%-44s - mac_channel = %s pdu = %s encr = %u\n",
                "service_u_plane",
                mac_logical_channel_name(mac_logical_channel).c_str(),
                vector_to_string(pdu, pdu.size()).c_str(),
                usage_marker_encryption_mode[mac_state.downlink_usage_marker]);
        fflush(stdout);
    }

    if (mac_logical_channel == TCH_S)                                           // speech frame
    {
        report_start_u_plane("UPLANE", "TCH_S");

        static const std::size_t MIN_SIZE = 432;

        // Changed from base program, no longer cares about speech content
        // as all speech on UK Tetra is encrypted. We make no effort to decrypt
        // so no value in retaining the speech content
        if (pdu.size() >= MIN_SIZE)
        {

            uint16_t speech_frame[690] = {0};

            /*
            for (int i = 0; i < 6; i++)
            {
                speech_frame[115 * i] = 0x6b21 + i;
            }

            for (int i = 0; i < 114; i++)
            {
                speech_frame[1 + i]  = pdu[i] ? -127 : 127;
            }

            for (int i = 0; i < 114; i++)
            {
                speech_frame[116 + i] = pdu[114 + i] ? -127 : 127;
            }

            for (int i = 0; i < 114; i++)
            {
                speech_frame[231 + i] = pdu[228 + i] ? -127 : 127;
            }

            for (int i = 0; i < 90; i++)
            {
                speech_frame[346 + i] = pdu[342 + i] ? -127 : 127;
            }
            */
            // speech frame would have been converted in base64 string
            // instead we simply initialise an empty array

            report_add("downlink usage marker", mac_state.downlink_usage_marker);                               // current usage marker
            report_add("encryption mode",       usage_marker_encryption_mode[mac_state.downlink_usage_marker]); // current encryption mode
            // report_add_compressed("frame", (const unsigned char *)speech_frame, 2 * 690);                       // actual binary frame 1380 bytes
        }
        else
        {
            //report_add("invalid pdu size", (uint64_t)pdu.size());
            //report_add("pdu minimum size", (uint64_t)MIN_SIZE);
        }

        report_send();
    }

}
