#ifndef CALLIDENTIFIER_H
#define CALLIDENTIFIER_H
#include <cstdint>
#include <string>

/*
 * Call identifier class
 */

class call_identifier_t {
public:
    call_identifier_t(uint32_t cid);
    ~call_identifier_t();

    uint32_t m_cid;
    uint8_t m_usage_marker;
    bool m_tx_granted;

    double m_data_received;

    static const        int MAX_USAGES = 64;                                    // maximum usages defined by norm
    static constexpr double TIMEOUT_S  = 10.0;                                  // maximum timeout between messages TODO handle Txx timers
    
    string m_file_name[MAX_USAGES];                                             // file names to use for usage marker/cid
    time_t m_last_traffic_time[MAX_USAGES];                                     // last traffic seen to know when to start new record
    
    vector<ssi_t> m_ssi;                                                        // list of ssi associated with this cid

    void push_traffic(const char * data, uint32_t len);
    void update_usage_marker(uint8_t usage_marker);
    
};


#endif /* CALLIDENTIFIER_H */
