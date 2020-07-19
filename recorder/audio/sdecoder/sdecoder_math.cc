#include "sdecoder.h"

using namespace codec_sdecoder;

const int16_t sdecoder::tab_inv_sqrt[49] = {
    32767, 31790, 30894, 30070, 29309, 28602, 27945, 27330, 26755, 26214,
    25705, 25225, 24770, 24339, 23930, 23541, 23170, 22817, 22479, 22155,
    21845, 21548, 21263, 20988, 20724, 20470, 20225, 19988, 19760, 19539,
    19326, 19119, 18919, 18725, 18536, 18354, 18176, 18004, 17837, 17674,
    17515, 17361, 17211, 17064, 16921, 16782, 16646, 16514, 16384
};

const int16_t sdecoder::tab_log2[33] = {
    0,  1455,  2866,  4236,  5568,  6863,  8124,  9352, 10549, 11716,
    12855, 13967, 15054, 16117, 17156, 18172, 19167, 20142, 21097, 22033,
    22951, 23852, 24735, 25603, 26455, 27291, 28113, 28922, 29716, 30497,
    31266, 32023, 32767
};

const int16_t sdecoder::tab_pow2[33] = {
    16384, 16743, 17109, 17484, 17867, 18258, 18658, 19066, 19484, 19911,
    20347, 20792, 21247, 21713, 22188, 22674, 23170, 23678, 24196, 24726,
    25268, 25821, 26386, 26964, 27554, 28158, 28774, 29405, 30048, 30706,
    31379, 32066, 32767
};

/**
 *
 *        Compute 1/sqrt(L_x).
 *        L_x is positive. The result is in Q30.
 *
 *    Complexity Weight : 56
 *
 *    Inputs :
 *
 *        L_x
 *            32 bit long signed integer (int32_t) whose value falls in the
 *            range : 0x0000 0000 <= L_x <= 0x7fff ffff.
 *
 *    Outputs :
 *
 *        none
 *
 *    Returned Value :
 *
 *        L_y
 *            32 bit long signed integer (int32_t) whose value falls in the
 *            range : 0x0000 0000 <= L_y <= 0x3fff ffff.
 *            L_y is a Q30 value (point between b30 and b29)
 *
 *    Algorithm :
 *
 *        The function 1/sqrt(L_x) is approximated by a table (tab_inv_sqrt)
 *        and linear interpolation :
 *
 *            1 - Normalization of L_x
 *            2 - If (30-exponant) is even then shift right once
 *            3 - exponant = (30-exponant)/2  +1
 *            4 - i = bit25-b31 of L_x,    16 <= i <= 63  ->because of normalization
 *            5 - a = bit10-b24
 *            6 - i -=16
 *            7 - L_y = tab_inv_sqrt[i]<<16 - (tab_inv_sqrt[i] - tab_inv_sqrt[i+1]) * a * 2
 *            8 - L_y >>= exponant
 *
 */

int32_t sdecoder::inv_sqrt(int32_t L_x)
{
    int16_t exp, i, a, tmp;
    int32_t L_y;

    if (L_x <= (int32_t)0) return ((int32_t)0x3fffffff);


    exp = op_norm_l(L_x);
    L_x = op_lshl(L_x, exp);                                                    // L_x is normalized

    exp = op_sub((int16_t)30, exp);
    if ((exp & 1) == 0)                                                         // If exponant even -> shift right
        L_x = op_lshr(L_x, (int16_t)1);

    exp = op_shr(exp, (int16_t)1);
    exp = op_add(exp, (int16_t)1);

    L_x = op_lshr(L_x, (int16_t)9);
    i   = op_extract_h(L_x);                                                    // Extract b25-b31
    L_x = op_lshr(L_x, (int16_t)1);
    a   = op_extract_l(L_x);                                                    // Extract b10-b24
    a   = a & (int16_t)0x7fff;

    i   = op_sub(i, (int16_t)16);

    L_y = op_ldeposit_h(tab_inv_sqrt[i]);                                       // tab_inv_sqrt[i] << 16
    tmp = op_sub(tab_inv_sqrt[i], tab_inv_sqrt[i + 1]);
    // tab_inv_sqrt[i] - tab_inv_sqrt[i+1])
    L_y = op_lmsu(L_y, tmp, a);                                                 // L_y -=  tmp*a*2

    L_y = op_lshr(L_y, exp);                                                    // denormalization

    return L_y;
}

/**
 *        Compute Log2(L_x).
 *        L_x is positive.
 *
 *    Complexity Weight : 48
 *
 *    Inputs :
 *
 *        L_x
 *            32 bit long signed integer (int32_t) whose value falls in the
 *            range : 0x0000 0000 <= L_x <= 0x7fff ffff.
 *
 *    Outputs :
 *
 *        exponant
 *            Integer part of Log2()
 *            16 bit  signed integer (int16_t) whose value falls in the
 *            range :   0 <= exponant <= 30
 *
 *        fraction
 *            Fractional part of Log2()
 *            16 bit signed integer (int16_t) whose value falls in the
 *            range : 0x0000 0000 <= fraction <= 0x7fff.
 *            It's a Q15 value (point between b15 and b16).
 *
 *    Returned Value :
 *
 *        none
 *
 *    Algorithm :
 *
 *        The function Log2(L_x) is approximated by a table (tab_log2)
 *        and linear interpolation :
 *
 *            1 - Normalization of L_x
 *            2 - exponant = 30-exponant
 *            3 - i = bit25-b31 of L_x,    32 <= i <= 63  ->because of normalization
 *            4 - a = bit10-b24
 *            5 - i -=32
 *            6 - fraction = tab_log2[i]<<16 - (tab_log2[i] - tab_log2[i+1]) * a * 2
 *
 */

void sdecoder::Log2(int32_t L_x, int16_t * exponant, int16_t * fraction)
{
    int16_t exp, i, a, tmp;
    int32_t L_y;

    if (L_x <= (int32_t)0)
    {
        *exponant = 0;
        *fraction = 0;
        return;
    }

    exp = op_norm_l(L_x);
    L_x = op_lshl(L_x, exp);                                                    // L_x is normalized

    *exponant = op_sub((int16_t)30, exp);

    L_x = op_lshr(L_x, (int16_t)9);
    i   = op_extract_h(L_x);                                                    // Extract b25-b31
    L_x = op_lshr(L_x, (int16_t)1);
    a   = op_extract_l(L_x);                                                    // Extract b10-b24 of fraction
    a   = a & (int16_t)0x7fff;

    i   = op_sub(i, (int16_t)32);

    L_y = op_ldeposit_h(tab_log2[i]);                                           // tab_log2[i] << 16
    tmp = op_sub(tab_log2[i], tab_log2[i + 1]);                                 // tab_log2[i] - tab_log2[i+1]
    L_y = op_lmsu(L_y, tmp, a);                                                 // L_y -= tmp*a*2

    *fraction = op_extract_h(L_y);
}

/**
 *        L_x = pow(2.0, exponant.fraction).
 *
 *    Complexity Weight : 17
 *
 *    Inputs :
 *
 *        exponant
 *            Integer part
 *            16 bit  signed integer (int16_t) whose value falls in the
 *            range :   0 <= exponant <= 30
 *
 *        fraction
 *            Fractional part
 *            16 bit signed integer (int16_t) whose value falls in the
 *            range : 0x0000 0000 <= fraction <= 0x7fff.
 *            It's a Q15 value (point between b15 and b16).
 *
 *    Outputs :
 *
 *        none
 *
 *    Returned Value :
 *
 *        L_x
 *            32 bit long signed integer (int32_t) whose value falls in the
 *            range : 0x0000 0000 <= L_x <= 0x7fff ffff.
 *
 *    Algorithm :
 *
 *        The function pow2(L_x) is approximated by a table (tab_pow2)
 *        and linear interpolation :
 *
 *            1 - i = bit11-b16 of fraction,   0 <= i <= 31
 *            2 - a = bit0-b10  of fraction
 *            3 - L_x = tab_pow2[i]<<16 - (tab_pow2[i] - tab_pow2[i+1]) * a * 2
 *            4 - L_x = L_x >> (30-exponant)     (with rounding)
 *
 */

int32_t sdecoder::pow2(int16_t exponant, int16_t fraction)
{
    int16_t exp, i, a, tmp;
    int32_t L_x;

    L_x = op_ldeposit_l(fraction);
    L_x = op_lshl(L_x, (int16_t)6);
    i   = op_extract_h(L_x);                                                    // Extract b10-b16 of fraction
    L_x = op_lshr(L_x, (int16_t)1);
    a   = op_extract_l(L_x);                                                    // Extract b0-b9   of fraction
    a   = a & (int16_t)0x7fff;


    L_x = op_ldeposit_h(tab_pow2[i]);                                           // tab_pow2[i] << 16
    tmp = op_sub(tab_pow2[i], tab_pow2[i + 1]);                                 // tab_pow2[i] - tab_pow2[i+1]
    L_x = op_lmsu(L_x, tmp, a);                                                 // L_x -= tmp*a*2

    exp = op_sub((int16_t)30, exponant);
    L_x = op_lshr_r(L_x, exp);
    return L_x;
}
