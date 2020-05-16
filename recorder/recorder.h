#ifndef RECORDER_H
#define RECORDER_H
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

call_identifier_t * get_cid(int index);

#endif /* RECORDER_H */
