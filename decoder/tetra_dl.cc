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

/**
 * @brief Constructor
 *
 * Initialize:
 *   - synchronization
 *   - time
 *   - cell
 *   - Viterby codec polynomials
 *
 */

tetra_dl::tetra_dl(int debug_level, bool remove_fill_bit_flag)
{
    g_debug_level          = debug_level;
    g_remove_fill_bit_flag = remove_fill_bit_flag;
    
    g_frame_len = 510;                                                          // burst length [510 bits]
    g_frame_data.clear();

    g_is_synchronized  = false;
    g_sync_bit_counter = 0;

    // initialize TDMA time
    g_time.tn = 1;
    g_time.mn = 1;
    g_time.fn = 1;

    // initialize cell informations
    g_cell_infos.color_code         = 0;
    g_cell_infos.mcc                = 0;
    g_cell_infos.mnc                = 0;
    g_cell_infos.scrambling_code    = 0;
    g_cell_infos.downlink_frequency = 0;
    g_cell_infos.uplink_frequency   = 0;
    g_cell_informations_acquired    = false;

    /*
     * Initialize Viterbi coder/decoder for MAC
     *
     * 8.2.3.1.1 Generator polynomials for the RCPC 16-state mother code of rate 1/4
     *
     * G1 = 1 + D +             D^4 (8.3)
     * G2 = 1 +     D^2 + D^3 + D^4 (8.4)
     * G3 = 1 + D + D^2 +       D^4 (8.5)
     * G4 = 1 + D +       D^3 + D^4 (8.6)
     *
     * NOTE: representing bit order must be reversed for the codec, eg. 1 + D + 0 + 0 + D^4 -> 10011
     *
     */

    vector<int> polynomials;
    int constraint = 6;

    polynomials.push_back(0b10011);
    polynomials.push_back(0b11101);
    polynomials.push_back(0b10111);
    polynomials.push_back(0b11011);
    viterbi_codec16_14 = new ViterbiCodec(constraint, polynomials);

    mac_defrag = new mac_defrag_t(g_debug_level);
    
}

/**
 * @brief Destructor
 *
 */

tetra_dl::~tetra_dl()
{
    delete mac_defrag;
}

/**
 * @brief Reset the synchronizer
 *
 * Burst was matched, we can reset the synchronizer to allow 50 missing frames (expressed in burst units = 50 * 510 bits)
 *
 */

void tetra_dl::reset_synchronizer()
{
    g_is_synchronized  = true;
    g_sync_bit_counter = g_frame_len * 50;                                      // allow 50 missing frames (in bits unit)
}

/**
 * @brief Process a received symbol.
 *
 * This function is called by "physical layer" when a bit is ready
 * to be processed.
 *
 * Note that "frame" is actually called "burst" in Tetra doc
 *
 * @return 1 if frame (burst) found, 0 otherwise
 *
 */

int tetra_dl::rx_symbol(uint8_t sym)
{
    g_frame_data.push_back(sym);                                                // insert symbol at buffer end
    if (g_frame_data.size() < g_frame_len) return 0;                            // not enough data to process

    int frame_found = 0;
    int score_begin = pattern_at_position_score(g_frame_data, normal_training_sequence3_begin, 0);
    int score_end   = pattern_at_position_score(g_frame_data, normal_training_sequence3_end, 500);

    if ((score_begin == 0) && (score_end < 2))                                  // frame (burst) is matched and can be processed
    {
        frame_found = 1;
        reset_synchronizer();                                                   // reset missing sync synchronizer
    }

    int cleared_flag = 0;

    if (frame_found || (g_is_synchronized && (g_sync_bit_counter % 510 == 0)))  // the frame can be processed either by presence of training sequence, either by synchronised and still allowed missing frames
    {
        increment_tn();
        process_frame();
        g_frame_data.clear();                                                   // frame has been processed, clear it
        cleared_flag = 1;                                                       // set flag to prevent erasing first bit in frame
    }

    g_sync_bit_counter--;

    if (g_sync_bit_counter <= 0)                                                // synchronization is lost
    {
        printf("* synchronization lost\n");
        g_is_synchronized  = false;
        g_sync_bit_counter = 0;
    }

    if (!cleared_flag)
    {
        g_frame_data.erase(g_frame_data.begin());                               // remove first symbol from buffer to make space for next one
    }

    return frame_found;
}

/**
 * @brief Report information to screen
 *
 */

void tetra_dl::print_data()
{
    string txt = "";
    for (int i = 0; i < 12; i++) txt += g_frame_data[i] == 0 ? "0" : "1";

    txt += " ";
    for (int i = 12; i < 64; i++) txt += g_frame_data[i] == 0 ? "0" : "1";

    txt += " ";
    for (int i = 510 - 11; i < 510; i++) txt += g_frame_data[i] == 0 ? "0" : "1";

    printf("%s", txt.c_str());
}

/**
 * @brief Process frame to decide which type of burst it is then service lower MAC
 *
 */

void tetra_dl::process_frame()
{
    int score_sync    = pattern_at_position_score(g_frame_data, synchronization_training_sequence, 214);
    int score_normal1 = pattern_at_position_score(g_frame_data, normal_training_sequence1, 244);
    int score_normal2 = pattern_at_position_score(g_frame_data, normal_training_sequence2, 244);

    //printf("    scores: sync / norm1 / norm2 = %2d %2d %2d -> ", score_sync, score_normal1, score_normal2);

    // soft decision
    int score_min = score_sync;
    int burst_type = SB;

    if (score_normal1 < score_min)
    {
        score_min  = score_normal1;
        burst_type = NDB;
    }

    if (score_normal2 < score_min)
    {
        score_min  = score_normal2;
        burst_type = NDB_SF;
    }

    if (score_min > 5)                                                          // invalid burst found
    {
        burst_type = -1;
    }
    else                                                                        // insert it for MAC lower layer processing
    {
        service_lower_mac(g_frame_data, burst_type);                            // send it to MAC
    }
}

/**
 * @brief Calculate cell scrambling code
 *
 * Scrambling code - see 8.2.5
 * Tetra scrambling code - 30 bits, see 23.2.1 - Figure 141
 * for synchronisation burst, MCC = MNC = ColorCode = 0
 *
 */

void tetra_dl::calculate_scrambling_code()
{
    uint16_t lmcc        = g_cell_infos.mcc & 0x03ff;                           // 10 MSB of MCC
    uint16_t lmnc        = g_cell_infos.mnc & 0x3fff;                           // 14 MSB of MNC
    uint16_t lcolor_code = g_cell_infos.color_code & 0x003f;                    // 6 MSB of ColorCode

    g_cell_infos.scrambling_code = lcolor_code | (lmnc << 6) | (lmcc << 20);     // 30 MSB bits
    g_cell_infos.scrambling_code = (g_cell_infos.scrambling_code << 2) | 0x0003; // scrambling initialized to 1 on bits 31-32 - 8.2.5.2 (54)
}

/**
 * @brief Increment TDMA counter with wrap-up as required
 *
 */

void tetra_dl::increment_tn()
{
    g_time.tn++;
    if (g_time.tn > 4)                                                          // time slot
    {
        g_time.fn++;
        g_time.tn = 1;
    }

    if (g_time.fn > 18)                                                         // frame number
    {
        g_time.mn++;
        g_time.fn = 1;
    }

    if (g_time.mn > 60)                                                         // multi-frame number
    {
        g_time.mn = 1;
    }
}
