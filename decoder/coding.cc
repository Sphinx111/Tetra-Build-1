#include "tetra_dl.h"
#include "utils.h"

/**
 * @brief Fibonacci LFSR descrambling - 8.2.5
 *
 */

vector<uint8_t> tetra_dl::dec_descramble(vector<uint8_t> data, int len, uint32_t scrambling_code) // OK
{
    const uint8_t poly[14] = {32, 26, 23, 22, 16, 12, 11, 10, 8, 7, 5, 4, 2, 1}; // Feedback polynomial - see 8.2.5.2 (8.39)

    vector<uint8_t> res;

    uint32_t lfsr = scrambling_code;                                            // linear feedback shift register initialization (=0 + 3 for BSCH, calculated from Color code ch 19 otherwise)
    for (int i = 0; i < len; i++)
    {
        uint32_t bit = lfsr >> (32 - poly[0]);                                  // apply poly (Xj + ...)
        for (int j = 1; j < 14; j++)
        {
            bit = bit ^ (lfsr >> (32 - poly[j]));
        }
        bit = bit & 1;                                                          // finish apply feedback polynomial (+ 1)
        lfsr = (lfsr >> 1) | (bit << 31);

        res.push_back(data[i] ^ (bit & 0xff));
    }

    return res;
}

/**
 * @brief (K,a) block deinterleaver - 8.2.4
 *
 */

vector<uint8_t> tetra_dl::dec_deinterleave(vector<uint8_t> data, uint32_t K, uint32_t a)
{
    vector<uint8_t> res(K, 0);                                                  // output vector is size K

    for (unsigned int idx = 1; idx <= K; idx++)
    {
        uint32_t k = 1 + (a * idx) % K;
        res[idx - 1] = data[k - 1];                                             // to interleave: DataOut[i-1] = DataIn[k-1]
    }

    return res;
}

/**
 * @brief Depuncture with 2/3 rate - 8.2.3.1.3
 *
 */

vector<uint8_t> tetra_dl::dec_depuncture23(vector<uint8_t> data, uint32_t len)
{
    const uint8_t P[] = {0, 1, 2, 5};                                           // 8.2.3.1.3 - P[1..t]
    vector<uint8_t> res(4 * len * 2 / 3, 2);                                    // 8.2.3.1.2 with flag 2 for erase bit in Viterbi routine

    uint8_t t = 3;                                                              // 8.2.3.1.3
    uint8_t period = 8;                                                         // 8.2.3.1.2

    for (uint32_t j = 1; j <= len; j++)
    {
        uint32_t i = j;                                                         // punct->i_func(j);
        uint32_t k = period * ((i - 1) / t) + P[i - t * ((i - 1) / t)];         // punct->period * ((i-1)/t) + P[i - t*((i-1)/t)];
        res[k - 1] = data[j - 1];
    }

    return res;
}

/**
 * @brief Viterbi decoding of RCPC code 16-state mother code of rate 1/4 - 8.2.3.1.1
 *
 */

vector<uint8_t> tetra_dl::dec_viterbi_decode16_14(vector<uint8_t> data)
{
    string s_in = "";
    for (unsigned int i = 0; i < data.size(); i++)
    {
        s_in += (char)(data[i] + '0');
    }

    string sOut = viterbi_codec16_14->Decode(s_in);

    vector<uint8_t> res;

    for (unsigned i = 0; i < sOut.size(); i++)
    {
        res.push_back((uint8_t)(sOut[i] - '0'));
    }

    return res;
}

/**
 * @brief Reed-Muller decoder and FEC correction 30 bits in, 14 bits out
 *
 * TODO no FEC for now
 *
 */

vector<uint8_t> tetra_dl::dec_reed_muller_3014_decode(vector<uint8_t> data)
{
    return vector_extract(data, 0, 14);
}

/**
 * @brief Calculated CRC16 ITU-T X.25 - CCITT
 *
 */

int tetra_dl::check_crc16ccitt(vector<uint8_t> data, int len)
{
    uint16_t crc = 0xFFFF;                                                      // CRC16-CCITT initial value

    for (int i = 0; i < len; i++)
    {
        uint16_t bit = (uint16_t)data[i];

        crc ^= bit << 15;
        if(crc & 0x8000)
        {
            crc <<= 1;
            crc ^= 0x1021;                                                      // CRC16-CCITT polynomial
        }
        else
        {
            crc <<= 1;
        }
    }

    return crc == 0x1D0F;                                                       // CRC16-CCITT reminder value
}
