#include "tetra_dl.h"
#include "utils.h"
#include <zlib.h>
#include "base64.h"

/**
 * @brief Prepare Json report
 *
 * Initialize Json object, add tetra common informations.
 * Must be terminated by send to free data
 *
 */

void tetra_dl::report_start(const char* service, const char* pdu)
{
    jobj = json_object_new_object();                                            // initialize json object

    json_object_object_add(jobj, "service", json_object_new_string(service));
    json_object_object_add(jobj, "pdu",     json_object_new_string(pdu));
    
    // time information
    json_object_object_add(jobj, "tn", json_object_new_int(g_time.tn));
    json_object_object_add(jobj, "fn", json_object_new_int(g_time.fn));
    json_object_object_add(jobj, "mn", json_object_new_int(g_time.mn));
    
    // mac informations
    json_object_object_add(jobj, "ssi",          json_object_new_int(mac_address.ssi));
    json_object_object_add(jobj, "usage marker", json_object_new_int(mac_address.usage_marker));
}

/**
 * @brief Prepare Json report
 *
 * Initialize Json object, add tetra common informations.
 * Must be terminated by send to free data
 *
 */

void tetra_dl::report_start(string service, string pdu)
{
    report_start(service.c_str(), pdu.c_str());
}

/**
 * @brief Add string data to report
 *
 */

void tetra_dl::report_add(const char *field, const char *val)
{
    json_object_object_add(jobj, field, json_object_new_string(val));
}

/**
 * @brief Add string data to report
 *
 */

void tetra_dl::report_add(string field, string val)
{
    json_object_object_add(jobj, field.c_str(), json_object_new_string(val.c_str()));
}

/**
 * @brief Add integer data to report
 *
 */

void tetra_dl::report_add(string field, uint8_t val)
{
    json_object_object_add(jobj, field.c_str(), json_object_new_int(val));
}

/**
 * @brief Add integer data to report
 *
 */
void tetra_dl::report_add(string field, uint16_t val)
{
    json_object_object_add(jobj, field.c_str(), json_object_new_int(val));
}

/**
 * @brief Add integer data to report
 *
 */

void tetra_dl::report_add(string field, uint32_t val)
{
    json_object_object_add(jobj, field.c_str(), json_object_new_int(val));    
}

/**
 * @brief Add integer data to report
 *
 */

void tetra_dl::report_add(string field, uint64_t val)
{
    json_object_object_add(jobj, field.c_str(), json_object_new_int64(val));
}

/**
 * @brief Add list of integer data to report
 *
 */

void tetra_dl::report_add(string field, vector<uint64_t> vec)
{
    struct json_object *arr = json_object_new_array();

    for (size_t cnt = 0; cnt < vec.size(); cnt++ )
    {
        json_object_array_add(arr, json_object_new_int64(vec[cnt]));
    }

    json_object_object_add(jobj, field.c_str(), arr);
}

/**
 * @brief Add data by compressing it with zlib then encoding in base-64,
 *   including all required stuff to be expanded as to know:
 *    - compressed zlib size (field "zsize")
 *    - uncompressed zlib size (field "uzsize")
 *
 */

void tetra_dl::report_add_compressed(string field, const unsigned char * binary_data, uint16_t data_len)
{
    const int BUFSIZE = 2048;

    // zlib compress
    char buf_zlib[BUFSIZE] = {0};                                               // zlib output buffer
    uint64_t z_uncomp_size = (uLong) data_len;                                  // uncompressed length
    uint64_t z_comp_size   = compressBound(z_uncomp_size);                      // compressed length
    z_comp_size = compressBound(z_uncomp_size);                                 // compressed length
    
    compress((Bytef *)buf_zlib, &z_comp_size, (Bytef *)binary_data, z_uncomp_size); // compress frame to buf_zlib
    
    // Base64 encode
    char buf_b64[BUFSIZE] = {0};
    b64_encode((const unsigned char *)buf_zlib, z_comp_size, (unsigned char *)buf_b64);

    report_add("uzsize", z_uncomp_size);                                        // uncompressed size (needed for zlib uncompress)
    report_add("zsize",  z_comp_size);                                          // compressed size
    report_add(field, buf_b64);
}

/**
 * @brief Send Json report and free data
 *
 */

void tetra_dl::report_send()
{
    struct {
        int flag;
        const char *flag_str;
    } json_flags = { JSON_C_TO_STRING_NOZERO, "JSON_C_TO_STRING_NOZERO" };      // remove all empty spaces between fields

    char buf[8192] = {0};

    strncpy(buf, json_object_to_json_string_ext(jobj, json_flags.flag), 8192 - 2);
    strcat(buf, "\n");                                                          // very important for the recorder

    write(socketfd, buf, sizeof(buf));

    //fprintf(stdout, "%s\n", json_object_to_json_string_ext(jobj, json_flags.flag)); // DEBUG print json packet
    
    json_object_put(jobj);                                                      // delete the json object
}
