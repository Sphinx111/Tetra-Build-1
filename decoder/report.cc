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
#include "base64.h"

#include <zlib.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

/**
 * @brief Prepare Json report
 *
 * Initialize Json object, add tetra common informations.
 * Must be ended by send
 *
 */

void tetra_dl::report_start(std::string service, std::string pdu)
{
    //report_start(service.c_str(), pdu.c_str());

    jdoc.SetObject();                                                           // create empty Json DOM

    report_add("service", service);
    report_add("pdu",     pdu);

    report_add("tn", g_time.tn);
    report_add("fn", g_time.fn);
    report_add("mn", g_time.mn);

    // TODO improve SSI/USSI/EVENT label handling
    // only SSI is used for now, it is not always relevant

    report_add("ssi",          mac_address.ssi);
    report_add("usage marker", mac_address.usage_marker);
    
    report_add("address_type", mac_address.address_type);

    switch (mac_address.address_type)
    {
    case 0b001:                                                                 // SSI
        report_add("actual ssi", mac_address.ssi);
        break;

    case 0b011:                                                                 // USSI
        report_add("ussi", mac_address.ussi);
        break;

    case 0b100:                                                                 // SMI
        report_add("smi", mac_address.smi);
        break;

    case 0b010:                                                                 // event label
        report_add("event label", mac_address.event_label);
        break;

    case 0b101:                                                                 // SSI + event label (event label assignment)
        report_add("actual ssi", mac_address.ssi);
        report_add("event label", mac_address.event_label);
        break;

    case 0b110:                                                                 // SSI + usage marker (usage marker assignment)
        report_add("actual ssi", mac_address.ssi);
        report_add("actual usage marker", mac_address.usage_marker);
        break;

    case 0b111:                                                                 // SMI + event label (event label assignment)
        report_add("smi", mac_address.smi);
        report_add("event label", mac_address.event_label);
        break;
    }
}

/**
 * @brief Add string data to report
 *
 */

void tetra_dl::report_add(std::string field, std::string val)
{
    rapidjson::Value key(field.c_str(), jdoc.GetAllocator());
    rapidjson::Value dat(val.c_str(),   jdoc.GetAllocator());;

    jdoc.AddMember(key, dat, jdoc.GetAllocator());
}

/**
 * @brief Add integer data to report
 *
 */

void tetra_dl::report_add(std::string field, uint8_t val)
{
    report_add(field, (uint64_t)val);
}

/**
 * @brief Add integer data to report
 *
 */
void tetra_dl::report_add(std::string field, uint16_t val)
{
    report_add(field, (uint64_t)val);
}

/**
 * @brief Add integer data to report
 *
 */

void tetra_dl::report_add(std::string field, uint32_t val)
{
    report_add(field, (uint64_t)val);
}

/**
 * @brief Add integer data to report
 *
 */

void tetra_dl::report_add(std::string field, uint64_t val)
{
    rapidjson::Value key(field.c_str(), jdoc.GetAllocator());
    rapidjson::Value dat(val);

    jdoc.AddMember(key, dat, jdoc.GetAllocator());
}

/**
 * @brief Add double data to report
 *
 */

void tetra_dl::report_add(std::string field, double val)
{
    rapidjson::Value key(field.c_str(), jdoc.GetAllocator());
    rapidjson::Value dat(val);

    jdoc.AddMember(key, dat, jdoc.GetAllocator());
}

/**
 * @brief Add vector of integer data to report as hexadecimal string.
 *        Note that 0x is omitted to preserve space
 *
 */

void tetra_dl::report_add(std::string field, std::vector<uint8_t> vec)
{
    std::string txt = "";
    char buf[32] = "";

    uint32_t pos = 0;
    for (std::size_t cnt = 0; cnt < vec.size() / 8; cnt++)
    {
        uint8_t val = get_value(vec, pos, 8);
        pos += 8;

        if (cnt > 0)
        {
            sprintf(buf, " %02x", val);
        }
        else
        {
            sprintf(buf, "%02x", val);
        }

        txt += buf;
    }

    report_add(field, txt);
}

/**
 * @brief Add an array of data to Json
 *
 */

void tetra_dl::report_add_array(std::string name, std::vector<std::tuple<std::string, uint64_t>> & infos)
{
    rapidjson::Value arr(rapidjson::kArrayType);                                                    

    for (std::size_t cnt = 0; cnt < infos.size(); cnt++)
    {
        rapidjson::Value jobj;
        jobj.SetObject();
        
        std::string field = std::get<0>(infos[cnt]);
        uint64_t val = std::get<1>(infos[cnt]);
        
        jobj.AddMember(rapidjson::Value(field.c_str(), jdoc.GetAllocator()).Move(),                                                                                                                                                                                                  
                       rapidjson::Value(val).Move(),                                                                                                                                                                                                                            
                       jdoc.GetAllocator());                                                                                                                                                                                                                                    
                                                                                                                                                                                                                                                                                
        arr.PushBack(jobj, jdoc.GetAllocator());                                                                                                                                                                                                                                
    }                                                                                                                                                                                                                                                                           
                                                                                                                                                                                                                                                                                
    jdoc.AddMember(rapidjson::Value(name.c_str(), jdoc.GetAllocator()), arr, jdoc.GetAllocator()); 
}

/**
 * @brief Add data by compressing it with zlib then encoding in base-64,
 *   including all required stuff to be expanded as to know:
 *    - compressed zlib size (field "zsize")
 *    - uncompressed zlib size (field "uzsize")
 *
 */

void tetra_dl::report_add_compressed(std::string field, const unsigned char * binary_data, uint16_t data_len)
{
    const int BUFSIZE = 2048;

    // zlib compress
    char buf_zlib[BUFSIZE] = {0};                                               // zlib output buffer
    uLong  z_uncomp_size = (uLong)data_len;                                     // uncompressed length
    uLongf z_comp_size   = compressBound(z_uncomp_size);                        // compressed length

    compress((Bytef *)buf_zlib, &z_comp_size, (Bytef *)binary_data, z_uncomp_size); // compress frame to buf_zlib

    // Base64 encode
    char buf_b64[BUFSIZE] = {0};
    b64_encode((const unsigned char *)buf_zlib, (uint64_t)z_comp_size, (unsigned char *)buf_b64);

    report_add("uzsize", (uint64_t)z_uncomp_size);                              // uncompressed size (needed for zlib uncompress)
    report_add("zsize",  (uint64_t)z_comp_size);                                // compressed size
    report_add(field,    buf_b64);                                              // actual data
}

/**
 * @brief Send Json report to UDP
 *
 */

void tetra_dl::report_send()
{
    rapidjson::StringBuffer buffer;                                             // the size of buffer is automatically increased by the writer
    buffer.Clear();

    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    jdoc.Accept(writer);

    std::string output(buffer.GetString());

    char eol = '\n';
    write(socketfd, output.c_str(), output.length() * sizeof(char));            // string doesn't contain newline
    write(socketfd, &eol, sizeof(char));                                        // so send it alone

    if (g_debug_level > 1)
    {
        printf("%s\n", output.c_str());
    }
}

