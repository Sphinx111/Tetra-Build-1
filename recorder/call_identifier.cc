#include "recorder.h"
#include "call_identifier.h"

/*
 * Call identifier class
 *
 * 14.6 Protocol timers (p179)
 * ----------------------------
 *
 * NOTES:
 *  - PDU descriptions in chapter 14.7 p180
 *
 *  [+] T310 timer - Call active -> Disconnect when timer expires
 *      Min. 5s
 *      No max, in D-SETUP or set by SwMI
 *    Max may be set by SwMI on:
 *      D-INFO
 *      D-CALL-RESTORE
 *    Starts on:
 *      D-CONNECT
 *      D-CONNECT-ACK
 *      D-TX-CONTINUE
 *      D-SETUP (contains call timeout Pt to MtPt only -> don't use it for now)
 *      D-INFO
 *      D-CALL-RESTORE
 *    Terminates on:
 *      D-RELEASE
 *      D-TX-WAIT          
 *
 *  [+] T311 timer - Call active TX -> Forced ceased transmission when timer expires
 *      Max. 300s
 *    Starts on:
 *      D-TX-GRANTED (transmit)
 *      D-TX-CONTINUE (continue)
 *      D-CONNECT (transmit)
 *      D-CONNECT-ACK (transmit)
 *    Terminates on:
 *      D-TX-INTERRUPT
 *      D-RELEASE
 *      D-TX-WAIT
 *      D-TX-CONTINUE (not continue)
 *
 */


call_identifier_t::call_identifier_t(uint32_t cid)
{
    // contructor
    m_cid           = cid;
    m_usage_marker  = 0;
    m_tx_granted    = false;
    m_data_received = 0.;
    
    m_ssi.clear();
    
    time_t now;
    time(&now);
    
    for (int cnt = 0; cnt < MAX_USAGES; cnt++)
    {
        m_file_name[cnt]         = "";
        m_last_traffic_time[cnt] = now;
    }
}


call_identifier_t::~call_identifier_t()
{
    m_ssi.clear();
}


void call_identifier_t::push_traffic(const char * data, uint32_t len)
{
    // set approx kB received
    time_t now;
    time(&now);

    if (difftime(now, m_last_traffic_time[m_usage_marker]) > TIMEOUT_S)                // check if timeout exceed predefined value
    {
        m_file_name[m_usage_marker] = "";                                              // force to start a new record since timeout
    }

    m_last_traffic_time[m_usage_marker] = now;
        
    if (m_file_name[m_usage_marker] == "")
    {
        struct tm *timeinfo;        
        timeinfo = localtime (&now);
        
        char filename[256];
        char tmp[16];

        strftime(tmp, 16, "%Y%m%d_%H%M%S", timeinfo);                                      // get time
        snprintf(filename, sizeof(filename), "out/%s_%06u_%02u.out", tmp, m_cid, m_usage_marker); // create file filename

        m_file_name[m_usage_marker] = filename;
        m_data_received = 0.;
    }

    //sprintf(filename, "rec/%06d_%02d.out", CID, Usage);
    FILE * file = fopen(m_file_name[m_usage_marker].c_str(), "ab");
    fwrite(data, 1, len, file);                                                 // 1 byte * len elements
    fflush(file);
    fclose(file);

    m_data_received += len / 1000.;
}


void call_identifier_t::update_usage_marker(uint8_t usage_marker)
{
    // update usage
    m_usage_marker = usage_marker;
}

