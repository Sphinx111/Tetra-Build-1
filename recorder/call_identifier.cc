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
#include "cid.h"
#include "call_identifier.h"

/**
 * @brief Constructor
 *
 */

call_identifier_t::call_identifier_t(uint32_t cid)
{
    m_cid           = cid;
    m_usage_marker  = 0;
    m_data_received = 0.;

    m_ssi.clear();

    time_t now;
    time(&now);

    for (int cnt = 0; cnt < MAX_USAGES; cnt++)
    {
        m_file_name[cnt]         = "";
        m_last_traffic_time[cnt] = now;
    }

    audio = new audio_decoder();
    audio->init();
}

/**
 * @brief Destructor
 *
 */

call_identifier_t::~call_identifier_t()
{
    m_ssi.clear();

    if (audio) delete audio;
}

/**
 * @brief Clean up the usage marker which were recording but
 *        has not been updated for TIMEOUT_RELEASE_S. This
 *        function should be run periodically by main program
 *
 * TODO handle also ssi last_seen timeout
 *
 */

void call_identifier_t::clean_up()
{
    time_t now;
    time(&now);

    for (int cnt = 0; cnt < MAX_USAGES; cnt++)
    {
        if (difftime(now, m_last_traffic_time[m_usage_marker]) > TIMEOUT_RELEASE_S) // check if timeout exceed predefined value
        {
            m_file_name[m_usage_marker] = "";                                       // reset the file name to release the marker
        }
    }

}

/**
 * @brief Push traffic to this CID taking care of TIMEOUT_S
 *        If timeout exceeded, a new file is created.
 *        This function store also data received in Kb
 */

void call_identifier_t::push_traffic(const char * data, uint32_t len)
{
    time_t now;
    time(&now);

    if (difftime(now, m_last_traffic_time[m_usage_marker]) > TIMEOUT_S)         // check if timeout exceed predefined value
    {
        m_file_name[m_usage_marker] = "";                                       // force to start a new record since timeout
    }

    m_last_traffic_time[m_usage_marker] = now;

    if (m_file_name[m_usage_marker] == "")
    {
        struct tm * timeinfo;
        timeinfo = localtime(&now);

        char filename[512] = "";
        char tmp[16]       = "";

        strftime(tmp, 16, "%Y%m%d_%H%M%S", timeinfo);                                             // get time
        snprintf(filename, sizeof(filename), "out/%s_%06u_%02u.out", tmp, m_cid, m_usage_marker); // create file filename

        m_file_name[m_usage_marker] = filename;
        m_data_received = 0.;
    }

    FILE * file = fopen(m_file_name[m_usage_marker].c_str(), "ab");
    fwrite(data, 1, len, file);                                                 // 1 byte * len elements
    fflush(file);
    fclose(file);

    m_data_received += len / 1000.;
}

/**
 * @brief Push traffic in raw format to this CID taking care of TIMEOUT_S
 *        If timeout exceeded, a new file is created.
 *        This function store also data received in Kb
 */

void call_identifier_t::push_traffic_raw(const char * data, uint32_t len)
{
    time_t now;
    time(&now);

    if (difftime(now, m_last_traffic_time[m_usage_marker]) > TIMEOUT_S)         // check if timeout exceed predefined value
    {
        m_file_name[m_usage_marker] = "";                                       // force to start a new record since timeout
    }

    m_last_traffic_time[m_usage_marker] = now;

    if (m_file_name[m_usage_marker] == "")
    {
        struct tm * timeinfo;
        timeinfo = localtime(&now);

        char filename[512] = "";
        char tmp[16]       = "";

        strftime(tmp, 16, "%Y%m%d_%H%M%S", timeinfo);                                             // get time
        snprintf(filename, sizeof(filename), "raw/%s_%06u_%02u.raw", tmp, m_cid, m_usage_marker); // create file filename

        m_file_name[m_usage_marker] = filename;
        m_data_received = 0.;

        audio->init();
    }

    // string filename_debug = m_file_name[m_usage_marker] + ".cod";
    // FILE * file = fopen(filename_debug.c_str(), "ab");
    // audio->process_frame_debug(file, (int16_t *)data, raw_output, 0);
    // fflush(file);
    // fclose(file);

    int16_t raw_output[480];

    // TODO for now, there is no stealing handling (always 0)
    if (audio->process_frame((int16_t *)data, raw_output, 0))                   // check if raw output is valid
    {
        FILE * file = fopen(m_file_name[m_usage_marker].c_str(), "ab");
        fwrite(raw_output, 2, 480, file);                                       // 2 speech frames of 240 elements * sizeof(int16_t)
        fflush(file);
        fclose(file);

        m_data_received += len / 1000.;
    }
}

/**
 * @brief Update usage marker
 *
 */

void call_identifier_t::update_usage_marker(uint8_t usage_marker)
{
    m_usage_marker = usage_marker;
}
