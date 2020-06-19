#include "mac_defrag.h"
#include "utils.h"

static const int DEBUG_VAL = 1;                                                 // start debug informations at level 1

mac_defrag_t::mac_defrag_t(int debug_level)
{
    g_debug_level = debug_level;
    tm_sdu.clear();

    fragments_count = 0;
    b_stopped       = true;
}

mac_defrag_t::~mac_defrag_t()
{
    tm_sdu.clear();
}

void mac_defrag_t::start(const mac_address_t address, const tetra_time_t time_slot)
{
    // start the defragment, flush previous data if already in use
    // and report informations
    //
    // NOTE: total fragmented length is unknown
    //

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

    mac_address     = address;
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

void mac_defrag_t::append(const vector<uint8_t> sdu, const mac_address_t address)
{
    // we can't append if in stopped mode
    if (b_stopped)
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
            printf("  * DEFRAG APPEND   : SSI = %u - TN/FN/MN = %02u/%02u/%02u - fragment %d - length = sdu %u / tm_sdu %u\n",
                   mac_address.ssi,
                   start_time.tn,
                   start_time.fn,
                   start_time.mn,
                   fragments_count,
                   (uint32_t)sdu.size(),
                   (uint32_t)tm_sdu.size());
        }
    }
}

vector<uint8_t> mac_defrag_t::get_sdu()
{
    // check SDU validity and return it
    vector<uint8_t> ret;

   if (b_stopped)
   {
       if (g_debug_level >= DEBUG_VAL)
       {
           printf("  * DEFRAG END      : FAILED SSI = %u - TN/FN/MN = %02u/%02u/%02u - fragment %d - length = %u\n",
                  mac_address.ssi,
                  start_time.tn,
                  start_time.fn,
                  start_time.mn,
                  fragments_count,
                  (uint32_t)tm_sdu.size());
       }
   }
   else
   {
       // FIXME add check
       ret = tm_sdu;
   }

    return ret;
}

void mac_defrag_t::stop()
{
    // clean stop
    b_stopped       = true;
    fragments_count = 0;
    tm_sdu.clear();
}
