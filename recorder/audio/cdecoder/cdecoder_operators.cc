#include <stdio.h>
#include "cdecoder.h"

using namespace codec_cdecoder;

int16_t cdecoder::op_abs_s(int16_t val1)
{
    int16_t res;

    if (val1 == (int16_t)0x8000)
    {
        res = MAX_16;
    }
    else
    {
        if (val1 < 0)
        {
            res = -val1;
        }
        else
        {
            res = val1;
        }
    }

    return res;
}


int16_t cdecoder::op_add(int16_t val1, int16_t val2)
{
    int32_t lsum = (int32_t)val1 + val2;
    int16_t res = op_sature(lsum);
    return res;
}


int16_t cdecoder::op_div_s(int16_t val1, int16_t val2)
{
    int16_t res = 0;

    if ((val1 > val2) || (val1 < 0) || (val2 < 0))
    {
        printf("Division Error\n");
        exit(0);
    }

    if (val2 == 0)
    {
        printf("Division by 0, Fatal error \n");
        exit(0);
    }

    if (val1 == 0)
    {
        res = 0;
    }
    else
    {
        if (val1 == val2)
        {
            res = MAX_16;
        }
        else
        {
            int32_t lnum   = op_ldeposit_l(val1);
            int32_t ldenom = op_ldeposit_l(val2);

            for (int16_t iteration = 0; iteration < 15; iteration++)
            {
                res <<=1;
                lnum <<= 1;

                if (lnum >= ldenom)
                {
                    lnum = op_lsub(lnum, ldenom);
                    res  = op_add(res, (int16_t)1);
                }
            }
        }
    }

    return res;
}


int16_t cdecoder::op_extract_h(int32_t lval1)
{
    int16_t res = (int16_t)(lval1 >> 16);

    return res;
}


int16_t cdecoder::op_extract_l(int32_t lval1)
{
    int16_t res = (int16_t)lval1;

    return res;
}


int32_t cdecoder::op_labs(int32_t lval1)
{
    int32_t lres;

    if (lval1 == MIN_32)
    {
        lres = MAX_32;
    }
    else
    {
        if (lval1 < 0)
        {
            lres = -lval1;
        }
        else
        {
            lres = lval1;
        }
    }

    return lres;
}


int32_t cdecoder::op_ladd(int32_t lval1, int32_t lval2)
{
    int32_t lres = lval1 + lval2;

    if (((lval1 ^ lval2) & MIN_32) == 0)
    {
        if ((lres ^ lval1) & MIN_32)
        {
            lres = (lval1 < 0) ? MIN_32 : MAX_32;
            g_overflow = 1;
        }
    }

    return lres;
}


int32_t cdecoder::op_ldeposit_h(int16_t val1)
{
    int32_t lres = (int32_t)val1 << 16;

    return lres;
}


int32_t cdecoder::op_ldeposit_l(int16_t val1)
{
    int32_t lres = (int32_t)val1;

    return lres;
}


int32_t cdecoder::op_lmac(int32_t lval3, int16_t val1, int16_t val2)
{
    int32_t lprod = op_lmult(val1, val2);
    int32_t lres  = op_ladd(lval3, lprod);

    return lres;
}


int32_t cdecoder::op_lmac0(int32_t lval3, int16_t val1, int16_t val2)
{
    int32_t lprod = op_lmult0(val1, val2);
    int32_t lres  = op_ladd(lval3, lprod);

    return lres;
}


int32_t cdecoder::op_lmsu(int32_t lval3, int16_t val1, int16_t val2)
{
    int32_t lprod = op_lmult(val1, val2);
    int32_t lres  = op_lsub(lval3, lprod);

    return lres;
}


int32_t cdecoder::op_lmsu0(int32_t lval3, int16_t val1, int16_t val2)
{
    int32_t lprod = op_lmult0(val1, val2);
    int32_t lres  = op_lsub(lval3, lprod);

    return lres;
}


int32_t cdecoder::op_lmult(int16_t val1, int16_t val2)
{
    int32_t lres = (int32_t)val1 * (int32_t)val2;

    if (lres != (int32_t)0x40000000)
    {
        lres *= 2;
    }
    else
    {
        g_overflow = 1;
        lres = MAX_32;
    }

    return lres;
}


int32_t cdecoder::op_lmult0(int16_t val1, int16_t val2)
{
    int32_t lres = (int32_t)val1 * (int32_t)val2;

    return lres;
}


int32_t cdecoder::op_lnegate(int32_t lval1)
{
    int32_t lres = (lval1 == MIN_32) ? MAX_32 : -lval1;

    return lres;
}


int32_t cdecoder::op_lshl(int32_t lval1, int16_t val2)
{
    int32_t lres;

    if (val2 <= 0)
    {
        lres = op_lshr(lval1, (int16_t)(-val2));
    }
    else
    {
        for (; val2 > 0; val2--)
        {
            if (lval1 > (int32_t)0x3FFFFFFF)
            {
                g_overflow = 1;
                lres  = MAX_32;
                break;
            }
            else
            {
                if (lval1 < (int32_t)0xC0000000)
                {
                    g_overflow = 1;
                    lres  = MIN_32;
                    break;
                }
            }
            lval1 *= 2;
            lres = lval1;
        }
    }

    return lres;
}


int32_t cdecoder::op_lshr(int32_t lval1, int16_t val2)
{
    int32_t lres;

    if (val2 < 0)
    {
        lres = op_lshl(lval1, (int16_t)(-val2));
    }
    else
    {
        if (val2 >= 31)
        {
            lres = (lval1 < 0L) ? -1 : 0;
        }
        else
        {
            if (lval1 < 0)
            {
                lres = ~((~lval1) >> val2);
            }
            else
            {
                lres = lval1 >> val2;
            }
        }
    }

    return lres;
}


int32_t cdecoder::op_lshr_r(int32_t lval1, int16_t val2)
{
    int32_t lres;

    if (val2 > 31)
    {
        lres = 0;
    }
    else
    {
        lres = op_lshr(lval1, val2);
        if (val2 > 0)
        {
            if ((lval1 & ((int32_t)1 << (val2 - 1))) != 0)
            {
                lres++;
            }
        }
    }

    return lres;
}


int32_t cdecoder::op_lsub(int32_t lval1, int32_t lval2)
{
    int32_t lres = lval1 - lval2;

    if (((lval1 ^ lval2) & MIN_32) != 0)
    {
        if ((lres ^ lval1) & MIN_32)
        {
            lres  = (lval1 < 0L) ? MIN_32 : MAX_32;
            g_overflow = 1;
        }
    }

    return lres;
}


int16_t cdecoder::op_mult(int16_t val1, int16_t val2)
{
    int32_t lprod = (int32_t)val1 * (int32_t)val2;

    lprod = (lprod & (int32_t)0xFFFF8000) >> 15;

    if (lprod & (int32_t)0x00010000)
    {
        lprod = lprod | (int32_t) 0xFFFF0000;
    }

    int16_t res = op_sature(lprod);

    return res;
}


int16_t cdecoder::op_mult_r(int16_t val1, int16_t val2)
{
    int32_t lprod_arr;

    lprod_arr  = (int32_t)val1 * (int32_t)val2;                             // product
    lprod_arr += (int32_t) 0x00004000;                                      // round
    lprod_arr &= (int32_t) 0xFFFF8000;
    lprod_arr >>= 15;                                                       // shift

    if (lprod_arr & (int32_t)0x00010000)                                    // sign extend when necessary
    {
        lprod_arr |= (int32_t) 0xFFFF0000;
    }

    int16_t res = op_sature(lprod_arr);

    return res;
}


int16_t cdecoder::op_negate(int16_t val1)
{
    int16_t res = (val1 == MIN_16) ? MAX_16 : -val1;

    return res;
}


int16_t cdecoder::op_norm_l(int32_t lval1)
{
    int16_t res;

    if (lval1 == 0)
    {
        res = 0;
    }
    else
    {
        if (lval1 == (int32_t)0xFFFFFFFF)
        {
            res = 31;
        }
        else
        {
            if (lval1 < 0)
            {
                lval1 = ~lval1;
            }

            for (res = 0; lval1 < (int32_t)0x40000000; res++)
            {
                lval1 <<= 1;
            }
        }
    }

    return res;
}


int16_t cdecoder::op_norm_s(int16_t val1)
{
    int16_t res;

    if (val1 == 0)
    {
        res = 0;
    }
    else
    {
        if (val1 == (int16_t) 0xFFFF)
        {
            res = 15;
        }
        else
        {
            if (val1 < 0)
            {
                val1 = ~val1;
            }

            for (res = 0; val1 < 0x4000; res++)
            {
                val1 <<= 1;
            }
        }
    }

    return res;
}


int16_t cdecoder::op_etsi_round(int32_t lval1)
{
    int32_t lround = op_ladd(lval1, (int32_t)0x00008000);
    int16_t res    = op_extract_h(lround);

    return res;
}


int16_t cdecoder::op_sature(int32_t lval1)
{
    int16_t res;

    if (lval1 > 0X00007fffL)
    {
        g_overflow = 1;
        res = MAX_16;
    }
    else if (lval1 <(int32_t)0xFFFF8000L)
    {
        g_overflow = 1;
        res = MIN_16;
    }
    else
    {
        g_overflow = 0;
        res = op_extract_l(lval1);
    }

    return res;
}


int16_t cdecoder::op_shl(int16_t val1, int16_t val2)
{
    int16_t res;

    if (val2 < 0)
    {
        res = op_shr(val1, (int16_t)(-val2));
    }
    else
    {
        int32_t resultat = (int32_t) val1 * ((int32_t)1 << val2);
        if (((val2 > 15) && (val1 != 0)) || (resultat !=(int32_t)((int16_t)resultat)))
        {
            g_overflow = 1;
            res = (val1 > 0) ? MAX_16 : MIN_16;
        }
        else
        {
            res = op_extract_l(resultat);
        }
    }

    return res;
}


int16_t cdecoder::op_shr(int16_t val1, int16_t val2)
{
    int16_t res;

    if (val2 < 0)
    {
        res = op_shl(val1, (int16_t)(-val2));
    }
    else
    {
        if (val2 >= 15)
        {
            res =(val1 < 0) ? -1 : 0;
        }
        else
        {
            if (val1 < 0)
            {
                res = ~((~val1) >> val2);
            }
            else
            {
                res = val1 >> val2;
            }
        }
    }

    return res;
}


int16_t cdecoder::op_sub(int16_t val1, int16_t val2)
{
    int32_t ldiff = (int32_t)val1 - val2;
    int16_t res   = op_sature(ldiff);

    return res;
}


int32_t cdecoder::op_lcomp(int16_t hi, int16_t lo)
{
    return op_add_sh(op_load_sh(lo, (int16_t)0), hi, (int16_t)15);
}


void cdecoder::op_lextract(int32_t L_32, int16_t *hi, int16_t *lo)
{
    *hi  = op_extract_h(op_lshl(L_32,(int16_t)1));
    *lo  = op_extract_l(op_sub_sh(L_32, *hi, (int16_t)15));
}


int32_t cdecoder::op_mpy_mix(int16_t hi1, int16_t lo1, int16_t lo2)
{
    int16_t p1;
    int32_t L_32;

    p1   = op_extract_h(op_lmult0(lo1, lo2));
    L_32 = op_lmult0(hi1,lo2);

    return op_add_sh(L_32, p1, (int16_t)1);
}


int32_t cdecoder::op_mpy_32(int16_t hi1, int16_t lo1, int16_t hi2, int16_t lo2)
{
    int16_t p1, p2;
    int32_t L_32;

    p1   = op_extract_h(op_lmult0(hi1, lo2));
    p2   = op_extract_h(op_lmult0(lo1, hi2));
    L_32 = op_lmult0(hi1, hi2);
    L_32 = op_add_sh(L_32, p1, (int16_t)1);

    return op_add_sh(L_32, p2, (int16_t)1);
}


int32_t cdecoder::op_div_32(int32_t L_num, int16_t denom_hi, int16_t denom_lo)
{
    int16_t approx, hi, lo, n_hi, n_lo;
    int32_t t0;


    /* First approximation: 1 / L_denom = 1/denom_hi */

    approx = op_div_s((int16_t)0x3FFF, denom_hi);    /* result in Q15 */

    /* 1/L_denom = approx * (2.0 - L_denom * approx) */

    t0 = op_mpy_mix(denom_hi, denom_lo, approx);                                /* result in Q29 */

    t0 = op_lsub((int32_t)0x40000000, t0);        /* result in Q29 */

    op_lextract(t0, &hi, &lo);

    t0 = op_mpy_mix(hi, lo, approx);            /* = 1/L_denom in Q28 */

    /* L_num * (1/L_denom) */

    op_lextract(t0, &hi, &lo);
    op_lextract(L_num, &n_hi, &n_lo);
    t0 = op_mpy_32(n_hi, n_lo, hi, lo);

    return(op_lshl(t0,(int16_t)2));            /* From Q28 to Q30 */
}

const int16_t cdecoder::POW2[16] = {
    -1, -2, -4, -8, -16, -32, -64, -128, -256, -512,
    -1024, -2048, -4096, -8192, -16384, -32768
};


int32_t cdecoder::op_add_sh(int32_t L_var2, int16_t var1, int16_t shift)
{
    return(op_lmsu0(L_var2, var1, POW2[shift]));
}


int32_t cdecoder::op_add_sh16(int32_t L_var2, int16_t var1)
{
    return(op_lmsu(L_var2, var1, (int16_t)-32768));
}


int16_t cdecoder::op_bin2int(int16_t no_of_bits, int16_t *bitstream)
{
    int16_t value, i, bit;

    value = 0;
    for (i = 0; i < no_of_bits; i++)
    {
        value = op_shl(value,(int16_t)1);
        bit = *bitstream++;
        if (bit == BIT_1)  value += 1;
    }
    return(value);
}


void cdecoder::op_int2bin(int16_t value, int16_t no_of_bits, int16_t *bitstream)
{
    int16_t *pt_bitstream, i, bit;

    pt_bitstream = bitstream + no_of_bits;

    for (i = 0; i < no_of_bits; i++)
    {
        bit = value & MASK;
        if (bit == 0)
            *--pt_bitstream = BIT_0;
        else
            *--pt_bitstream = BIT_1;
        value = op_shr(value,(int16_t)1);
    }
}


int32_t cdecoder::op_load_sh(int16_t var1, int16_t shift)
{
    return(op_lmsu0((int32_t)0, var1, POW2[shift]));
}


int32_t cdecoder::op_load_sh16(int16_t var1)
{
    return(op_lmsu((int32_t)0, var1, (int16_t)-32768));
}


int32_t cdecoder::op_norm_v(int32_t L_var3, int16_t var1, int16_t *var2)
{
    int16_t shift;

    shift = op_norm_l(L_var3);
    if(op_sub(shift, var1) > 0) shift = var1;
    *var2 = shift;
    return(op_lshl(L_var3, shift));
}


int16_t cdecoder::op_store_hi(int32_t L_var1, int16_t var2)
{
    static int16_t SHR[8] = {16, 15, 14, 13, 12, 11, 10, 9};
    return(op_extract_l(op_lshr(L_var1, SHR[var2])));
}


int32_t cdecoder::op_sub_sh(int32_t L_var2, int16_t var1, int16_t shift)
{
    return(op_lmac0(L_var2, var1, POW2[shift]));
}


int32_t cdecoder::op_sub_sh16(int32_t L_var2, int16_t var1)
{
    return(op_lmac(L_var2, var1, (int16_t)-32768));
}
