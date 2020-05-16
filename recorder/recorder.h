#ifndef RECORDER_H
#define RECORDER_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
//#include <sstream>
//#include <math.h>
#include <string.h>
#include <vector>
#include <fstream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <json-c/json.h>
#include <json-c/json_util.h>
#include "base64.h"
#include <zlib.h>

using namespace::std;

/*
 * Short subscriber identity structure
 *
 */


struct ssi_t {
    time_t last_seen;
    uint32_t ssi;
};

class call_identifier_t;                                                        // forward declaration

call_identifier_t * get_cid(int index);

#endif /* RECORDER_H */
