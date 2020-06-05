#include <json-c/json.h>
#include <json-c/json_util.h>
#include <zlib.h>
#include "base64.h"
#include "cid.h"
#include "call_identifier.h"
#include "window.h"

/**
 * @brief Internal call identifiers list
 *
 */

static vector<call_identifier_t> cid_list;

/**
 * @brief Initialize CID list
 *
 */

void cid_init()
{
    cid_list.clear();
}

/**
 * @brief Clean up CID/SSI with their last seen time
 *
 * TODO to finish and test in class_itentifier_t
 *
 */

void cid_clean()
{
    time_t now;
    time(&now);

    for (size_t idx = 0; idx < cid_list.size(); idx++)
    {
        cid_list[idx].clean_up();
    }
}

/**
 * @brief API to access to CID list from other modules
 *
 */

call_identifier_t * get_cid(int index)
{
    call_identifier_t * res = NULL;

    if (cid_list.size() > 0)
    {
        if (index < (int)cid_list.size())
        {
            res = &cid_list[index];
        }
    }

    return res;
}
/**
 * @brief Returns true if CID exists is in list
 *
 */

static bool cid_exists(uint32_t cid)
{
    bool b_exists = false;

    for (size_t cnt = 0; cnt < cid_list.size(); cnt++)
    {
        if (cid_list[cnt].m_cid == cid)
        {
            b_exists = true;
            break;
        }
    }

    return b_exists;
}

/**
 * @brief Return CID index in list or ((size_t)-1) if not.
 *        This last value shouldn't be used since we have to check
 *        if the CID exists before
 *
 */

static size_t cid_index(uint32_t cid)
{
    size_t index = -1;

    for (size_t cnt = 0; cnt < cid_list.size(); cnt++)
    {
        if (cid_list[cnt].m_cid == cid)
        {
            index = cnt;
            break;
        }
    }

    return index;
}

/**
 * @brief Return CID index in list for a given usage marker or ((size_t)-1) if there is none associated.
 *
 */

static bool cid_index_by_usage_marker(uint8_t usage_marker, size_t * index)
{
    bool ret = false;

    for (size_t cnt = 0; cnt < cid_list.size(); cnt++)
    {
        if (cid_list[cnt].m_usage_marker == usage_marker)
        {
            *index = cnt;
            ret = true;
            break;
        }
    }

    return ret;
}

/**
 * @brief Add CID in list if not already there
 *
 */

static void cid_add(uint32_t cid)
{
    if (!cid_exists(cid))
    {
        cid_list.push_back(call_identifier_t(cid));
    }
}

/**
 * @brief Associate a SSI to a given CID. Add the CID is it doesn't exists.
 *        Update last seen SSI time.
 *
 */

static void cid_add_ssi_to_cid(uint32_t cid, uint32_t ssi)
{
    if (ssi <= 0) return;

    if (!cid_exists(cid))
    {
        cid_add(cid);
    }

    size_t index = cid_index(cid);

    // check if ssi already exists in cid list
    bool b_exists = false;

    for (size_t cnt = 0; cnt < cid_list[index].m_ssi.size(); cnt++)
    {
        if (cid_list[index].m_ssi[cnt].ssi == ssi)
        {
            time(&cid_list[index].m_ssi[cnt].last_seen);                        // SSI exists, update its last seen time
            b_exists = true;
            break;
        }
    }

    if (!b_exists)                                                              // if not exists, add new ssi to list
    {
        ssi_t new_ssi;
        new_ssi.ssi = ssi;
        time(&new_ssi.last_seen);
        cid_list[index].m_ssi.push_back(new_ssi);
    }
}

/**
 * @brief Update CID usage marker
 *
 * TODO check CID value to ensure CID is a valid number
 *
 */

static void cid_update_usage_marker(uint32_t cid, uint8_t usage_marker)
{
    if (!cid_exists(cid))                                                       // if cid doesn't exists, create it
    {
        cid_add(cid);
    }

    size_t index = cid_index(cid);
    cid_list[index].update_usage_marker(usage_marker);
}

/**
 * @brief Release a given CID from list
 *
 */

static void cid_release(uint32_t cid)
{
    if (!cid_exists(cid)) return;

    for (vector<call_identifier_t>::iterator it = cid_list.begin(); it != cid_list.end();)
    {
        if ((*it).m_cid == cid)
        {
            it = cid_list.erase(it);                                            // erase current id and get pointer to next one
        }
        else
        {
            ++it;                                                               // go to next element
        }
    }
}

/**
 * @brief Send traffic speech frame to a CID identified by a given usage marker
 *
 */

static void cid_send_traffic_to_cid_by_usage_marker(uint8_t usage_marker, const char * data, uint32_t len)
{
    if (usage_marker > 63) return;                                              // only values from 0-63 are relevant for TETRA

    size_t index;

    if (cid_index_by_usage_marker(usage_marker, &index))
    {
        cid_list[index].push_traffic(data, len);                                // push traffic to this cid
    }
}

/**
 * @brief Main function which process Json traffic and associate SSI, CID, etc...
 *
 * WARNING: D-INFO PDU mustn't be used to allocate a call id (see p. 136)
 *
 */

void cid_parse_pdu(const char * data, FILE * fd_log)
{
    struct json_object * jobj = json_tokener_parse(data);                       // parse informations from buffer

    if (jobj == NULL) return;                                                   // can't parse Json

    struct json_object * tmp;

    // extract data common to all pdu
    json_object_object_get_ex(jobj, "service", &tmp);
    string service = json_object_get_string(tmp);

    json_object_object_get_ex(jobj, "pdu", &tmp);
    string pdu = json_object_get_string(tmp);

    json_object_object_get_ex(jobj, "usage marker", &tmp);
    uint8_t usage_marker = (uint8_t)json_object_get_int(tmp);

    json_object_object_get_ex(jobj, "ssi", &tmp);
    uint32_t ssi = (uint32_t)json_object_get_int(tmp);

    bool b_log = true;

    if (!service.compare("MAC"))                                                // MAC service (don't print)
    {
        b_log = false;                                                          // too much packets to log
    }
    else if (!service.compare("UPLANE"))                                        // traffic speech frame
    {

        json_object_object_get_ex(jobj, "uzsize", &tmp);                        // uncompressed frame length 2 * 690 + 1 bytes
        uint64_t zlib_uncomp_size = json_object_get_int(tmp);

        json_object_object_get_ex(jobj, "zsize", &tmp);                         // compressed frame length (before B64 since B64 add overhead)
        uint64_t zlib_comp_size = json_object_get_int(tmp);

        json_object_object_get_ex(jobj, "frame", &tmp);                         // zlib + B64 frame
        string frame = json_object_get_string(tmp);

        const int BUFSIZE = 4096;

        // base64 decode
        unsigned char buf_b64out[BUFSIZE] = {0};
        //uint32_t len_out =
        b64_decode((const unsigned char *)frame.c_str(), frame.length(), buf_b64out);

        // zlib uncompress
        char buf_zlib_out[BUFSIZE] = {0};
        int ret = uncompress((Bytef *)buf_zlib_out, &zlib_uncomp_size, (Bytef *)buf_b64out, zlib_comp_size);

        if (!ret) cid_send_traffic_to_cid_by_usage_marker(usage_marker, buf_zlib_out, zlib_uncomp_size); // process it
    }
    else                                                                        // other services
    {
        if ((!pdu.compare("D-ALERT")) ||
            (!pdu.compare("D-CONNECT")) ||
            (!pdu.compare("D-CONNECT ACK")) ||
            (!pdu.compare("D-SETUP")) ||
            (!pdu.compare("D-STATUS")) ||
            (!pdu.compare("D-TX GRANTED")) ||
            (!pdu.compare("D-TX CEASED")))
        {
            // register new cid and attach ssi and usage marker
            json_object_object_get_ex(jobj, "call identifier", &tmp);
            uint32_t cid = (uint32_t)json_object_get_int(tmp);

            cid_update_usage_marker(cid, usage_marker);                         // CID will be added to list if it doesn't exists yet
            cid_add_ssi_to_cid(cid, ssi);
            scr_update(data);
        }
        else if (!pdu.compare("D-RELEASE"))                                     // || (!pdu.compare("D-TX WAIT")))
        {
            // release cid
            json_object_object_get_ex(jobj, "call identifier", &tmp);
            uint32_t cid = (uint32_t)json_object_get_int(tmp);

            cid_release(cid);
            scr_update(data);
        }
        else if ((!pdu.compare("D-SDS-DATA")) ||
                 (!pdu.compare("D-STATUS")))                                    // SDS messages
        {
            scr_print_middle(data);                                             // note that there is two lines printed for every message (for analyze, we provide full hexa + decoded message)
        }
        else
        {

        }
    }

    if (b_log)                                                                  // append to log file
    {
        struct {
            int flag;
            const char *flag_str;
        } json_flags = { JSON_C_TO_STRING_NOZERO, "JSON_C_TO_STRING_NOZERO" };  // remove all empty spaces between fields

        fprintf(fd_log, "%s\n", json_object_to_json_string_ext(jobj, json_flags.flag));
        fflush(fd_log);
    }

    json_object_put(tmp);                                                       // clean objects
    json_object_put(jobj);
}
