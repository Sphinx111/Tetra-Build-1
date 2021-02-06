#include "mac_defrag.h"
#include "utils.h"

static const int DEBUG_VAL = 1;                                                 // start debug informations at level 1

/**
 * @brief Defragmenter constructor
 *
 */

mac_defrag_t::mac_defrag_t(int debug_level)
{
    g_debug_level = debug_level;
    tm_sdu.clear();

    fragments_count = 0;
    b_stopped       = true;
}

/**
 * @brief Defragmenter destructor
 *
 */

mac_defrag_t::~mac_defrag_t()
{
    tm_sdu.clear();
}

/**
 * @brief Start defragmenter, flush previous data if already in use
 *        and report informations
 *
 * NOTE: total fragmented length is unknown
 *
 */

void mac_defrag_t::start(const mac_address_t address, const tetra_time_t time_slot)
{
    if (tm_sdu.size() > 0u)
    {
        if (g_debug_level >= DEBUG_VAL)
        {
            printf("  * DEFRAG FAILED   : invalid %d fragments received for SSI = %u: %u recovered for address %u\n",
                   fragments_count,
                   mac_address.ssi,
                   (uint32_t)tm_sdu.size(),
                   mac_address.ssi);
        }
    }

    mac_address     = address;                                                  // at this point, the defragmenter MAC address contains encryption mode
    start_time      = time_slot;
    fragments_count = 0;

    if (g_debug_level >= DEBUG_VAL)
    {
        printf("  * DEFRAG START    : SSI = %u - TN/FN/MN = %02u/%02u/%02u\n",
               mac_address.ssi,
               start_time.tn,
               start_time.fn,
               start_time.mn);
    }

    tm_sdu.clear();                                                             // clear the buffer

    b_stopped = false;
}

/**
 * @brief Append data to defragmenter
 *
 */

void mac_defrag_t::append(const std::vector<uint8_t> sdu, const mac_address_t address)
{
    if (b_stopped)                                                              // we can't append if in stopped mode
    {
        if (g_debug_level >= DEBUG_VAL)
        {
            printf("  * DEFRAG APPEND   : FAILED SSI = %u\n", address.ssi);
        }
    }
    else if (address.ssi != mac_address.ssi)                                    // check mac addresses
    {
        stop();                                                                 // stop defragmenter

        if (g_debug_level >= DEBUG_VAL)
        {
            printf("  * DEFRAG APPEND   : FAILED appending SSI = %u while fragment SSI = %u\n", mac_address.ssi, address.ssi);
        }
    }
    else
    {
        tm_sdu = vector_append(tm_sdu, sdu);
        fragments_count++;

        if (g_debug_level >= DEBUG_VAL)
        {
            printf("  * DEFRAG APPEND   : SSI = %u - TN/FN/MN = %02u/%02u/%02u - fragment %d - length = sdu %u / tm_sdu %u - encr = %u\n",
                   mac_address.ssi,
                   start_time.tn,
                   start_time.fn,
                   start_time.mn,
                   fragments_count,
                   (uint32_t)sdu.size(),
                   (uint32_t)tm_sdu.size(),
                   mac_address.encryption_mode
                );
        }
    }
}

/**
 * @brief Check SDU validity and return it
 *
 */

std::vector<uint8_t> mac_defrag_t::get_sdu(uint8_t * encryption_mode, uint8_t * usage_marker)
{
    std::vector<uint8_t> ret;

    if (b_stopped)
    {
        if (g_debug_level >= DEBUG_VAL)
        {
            printf("  * DEFRAG END      : FAILED SSI = %u - TN/FN/MN = %02u/%02u/%02u - fragment %d - length = %u - encr = %u\n",
                   mac_address.ssi,
                   start_time.tn,
                   start_time.fn,
                   start_time.mn,
                   fragments_count,
                   (uint32_t)tm_sdu.size(),
                   mac_address.encryption_mode
                );
        }
    }
    else
    {
        // FIXME add check
        *encryption_mode = mac_address.encryption_mode;
        *usage_marker    = mac_address.usage_marker;
        ret = tm_sdu;
    }

    return ret;
}

/**
 * @brief Stop defragmenter
 *
 */

void mac_defrag_t::stop()
{
    // clean stop
    b_stopped       = true;
    fragments_count = 0;
    tm_sdu.clear();
}
