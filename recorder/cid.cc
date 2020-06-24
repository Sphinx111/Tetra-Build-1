#include <cstdint>
#include <zlib.h>
#include "base64.h"
#include "cid.h"
#include "call_identifier.h"
#include "window.h"
#include "json_parser.h"
#include "utils.h"

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

//void cid_parse_pdu(const char * data, FILE * fd_log)
void cid_parse_pdu(string data, FILE * fd_log)
{
    json_parser_t * jparser = new json_parser_t(data);

    // extract data common to all pdu
    string   service;
    string   pdu;
    uint8_t  usage_marker;
    uint8_t  downlink_usage_marker;
    uint32_t ssi;

    bool b_valid = true;                                                        // check if the pdu is valid

    // cerr << jparser->to_string() <<endl;
    // delete jparser;
    // return;

    b_valid = b_valid && jparser->read("service",      &service);
    b_valid = b_valid && jparser->read("pdu",          &pdu);
    b_valid = b_valid && jparser->read("usage marker", &usage_marker);
    b_valid = b_valid && jparser->read("ssi",          &ssi);

    if (!b_valid)                                                               // invalid Json text, stop processing here
    {
        delete jparser;
        return;
    }

    bool b_log = true;

    if (!service.compare("MAC"))                                                // MAC service (don't print)
    {
        b_log = false;                                                          // too much packets to log
    }
    else if (!service.compare("UPLANE"))                                        // traffic speech frame
    {
        
        uint64_t zlib_uncomp_size;
        uint64_t zlib_comp_size;
        string frame;
        b_valid = jparser->read("downlink usage marker", &downlink_usage_marker); // may differ from usage marker
        b_valid = b_valid && jparser->read("uzsize", &zlib_uncomp_size);          // uncompressed frame length 2 * 690 + 1 bytes
        b_valid = b_valid && jparser->read("zsize",  &zlib_comp_size);            // compressed frame length (before B64 since B64 add overhead)
        b_valid = b_valid && jparser->read("frame",  &frame);                     // zlib + B64 frame

        if (b_valid)                                                            // we can process current speech frame
        {
            const int BUFSIZE = 4096;

            // base64 decode
            unsigned char buf_b64out[BUFSIZE] = {0};
            b64_decode((const unsigned char *)frame.c_str(), frame.length(), buf_b64out);

            // zlib uncompress
            char buf_zlib_out[BUFSIZE] = {0};
            int ret = uncompress((Bytef *)buf_zlib_out, &zlib_uncomp_size, (Bytef *)buf_b64out, zlib_comp_size);

            if (!ret)
            {
                cid_send_traffic_to_cid_by_usage_marker(downlink_usage_marker, buf_zlib_out, zlib_uncomp_size); // process it
            }
        }
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
            uint32_t cid;
            b_valid = jparser->read("call identifier", &cid);

            if (b_valid)
            {
                cid_update_usage_marker(cid, usage_marker);                     // CID will be added to list if it doesn't exists yet
                cid_add_ssi_to_cid(cid, ssi);
                scr_update(data);
            }
        }
        else if (!pdu.compare("D-RELEASE"))                                     // || (!pdu.compare("D-TX WAIT")))
        {
            // release cid
            uint32_t cid;
            b_valid = jparser->read("call identifier", &cid);

            if (b_valid)
            {
                cid_release(cid);
                scr_update(data);
            }
        }
        else if ((!pdu.compare("D-SDS-DATA")) ||
                 (!pdu.compare("D-STATUS")))                                    // SDS messages
        {
            string sds_msg;
            b_valid = jparser->read("infos", &sds_msg);

            if (b_valid)                                                        // text message can be printed
            {
                uint8_t  msg_ref = 0;
                uint32_t party_ssi = 0;
                uint8_t  protocol_id = 0;

                //{"service":"CMCE","pdu":"D-SDS-DATA","tn":2,"fn":13,"mn":41,"ssi":299906,"usage marker":47,"calling party type identifier":1,"calling party ssi":401101,"sds type identifier":3,"protocol id":130,"message type":0,"sds-pdu":"SDS-TRANSFER","message reference":225,"protocol info":"text messaging (SDS-TL)","text coding scheme":1,"infos":"_____ _)______(______b_)______(%"}

                jparser->read("message reference", &msg_ref);
                jparser->read("calling party ssi", &party_ssi);
                jparser->read("protocol id", &protocol_id);
                scr_print_sds(format_str("prot:%3u ssi:%6u calling:%6u ref:%3u  msg: '%s'", protocol_id, ssi, party_ssi, msg_ref, sds_msg.c_str()));
            }
            scr_update(data);                                                   // note that there is two lines printed for every message (for analyze, we provide full hexa + attempted decoded message with 8 bits charset)
        }
        else
        {

        }
    }

    if (b_log)                                                                  // append to log file
    {
        jparser->write_report(fd_log);
    }

    delete jparser;
}
