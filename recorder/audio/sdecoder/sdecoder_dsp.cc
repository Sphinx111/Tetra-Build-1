#include <stdio.h>
#include <stdlib.h>
#include "sdecoder.h"

using namespace codec_sdecoder;

/*	DESCRIPTION		:	Table for routine Az_Lsp().
 *
 *					Vector grid[] is in Q15
 *
 *						grid[0] = 1.0
 *						grid[grid_points+1] = -1.0
 *						for (i = 1; i < grid_points; i++)
 *						grid[i] = cos((6.283185307*i)/(2.0*grid_points))
 *
 */

const int16_t sdecoder::grid[GRID_POINTS + 1] = {
    32760,     32723,     32588,     32364,     32051,     31651,
    31164,     30591,     29935,     29196,     28377,     27481,
    26509,     25465,     24351,     23170,     21926,     20621,
    19260,     17846,     16384,     14876,     13327,     11743,
    10125,      8480,      6812,      5126,      3425,      1714,
       -1,     -1715,     -3426,     -5127,     -6813,     -8481,
   -10126,    -11744,    -13328,    -14877,    -16385,    -17847,
   -19261,    -20622,    -21927,    -23171,    -24352,    -25466,
   -26510,    -27482,    -28378,    -29197,    -29936,    -30592,
   -31165,    -31652,    -32052,    -32365,    -32589,    -32724,
   -32760
};

/*	DESCRIPTION		:	Table of lag_window coefficents for the autocorrelation.
 *
 *						lag_wind[0] = 1.00000000 	(*)
 *						lag_wind[1] = 0.99884027	(**)
 *						lag_wind[2] = 0.99551868
 *						lag_wind[3] = 0.99000722
 *						lag_wind[4] = 0.98234236
 *						lag_wind[5] = 0.97257471
 *						lag_wind[6] = 0.96076828
 *						lag_wind[7] = 0.94699991
 *						lag_wind[8] = 0.93135828
 *						lag_wind[9] = 0.91394323
 *						lag_wind[10]= 0.89486438
 *
 *	COMMENTS		:	- (*) The first coefficient whose value = 1 is just
 *					  mentioned for information, but not included in the code
 *					- (**) All the other coefficents incorporate a scaling factor
 *					  of 1,00005 corresponding to a noise floor of -43 dB
 *					- This table uses a special extended precision format
 *					  (see "comments" in the header of
 *					   the file fexp_tet.c - Section A.5.2)
 *
 */

const int16_t sdecoder::LAG_H[10] = {
    32729,
    32621,
    32440,
    32189,
    31869,
    31482,
    31031,
    30518,
    29948,
    29322
};

const int16_t sdecoder::LAG_L[10] = {
    32704,
    5120,
    18240,
    12928,
    10752,
    14912,
    9600,
    24512,
    3008,
    30016
};

/*	DESCRIPTION		:	Asymetric Hamming window for LPC analysis.
 *
 */

const int16_t sdecoder::WIND[L_WINDOW] = {
    2621,  2623,  2628,  2636,  2647,  2662,  2679,  2700,  2724,  2752,
    2782,  2816,  2852,  2892,  2936,  2982,  3031,  3084,  3140,  3199,
    3260,  3325,  3393,  3465,  3539,  3616,  3696,  3779,  3865,  3954,
    4047,  4141,  4239,  4340,  4444,  4550,  4659,  4771,  4886,  5003,
    5123,  5246,  5372,  5500,  5631,  5764,  5900,  6038,  6179,  6323,
    6468,  6617,  6767,  6920,  7075,  7233,  7392,  7554,  7718,  7884,
    8052,  8223,  8395,  8569,  8746,  8924,  9104,  9286,  9470,  9655,
    9842,  10031, 10221, 10413, 10607, 10802, 10999, 11197, 11396, 11597,
    11799, 12002, 12207, 12413, 12619, 12827, 13036, 13246, 13457, 13669,
    13882, 14095, 14309, 14524, 14740, 14956, 15173, 15391, 15608, 15827,
    16046, 16265, 16484, 16704, 16924, 17144, 17364, 17584, 17804, 18024,
    18245, 18465, 18684, 18904, 19124, 19343, 19561, 19780, 19998, 20215,
    20432, 20648, 20864, 21079, 21293, 21506, 21719, 21931, 22142, 22352,
    22561, 22769, 22976, 23181, 23386, 23589, 23791, 23992, 24191, 24389,
    24586, 24781, 24975, 25167, 25357, 25546, 25733, 25919, 26102, 26284,
    26464, 26642, 26819, 26993, 27165, 27336, 27504, 27670, 27834, 27996,
    28156, 28313, 28468, 28621, 28772, 28920, 29066, 29209, 29350, 29488,
    29624, 29757, 29888, 30016, 30142, 30265, 30385, 30502, 30617, 30729,
    30838, 30945, 31048, 31149, 31247, 31342, 31434, 31523, 31609, 31692,
    31772, 31850, 31924, 31995, 32063, 32128, 32190, 32249, 32304, 32357,
    32406, 32453, 32496, 32536, 32573, 32606, 32637, 32664, 32688, 32709,
    32727, 32741, 32753, 32761, 32765, 32767, 32767, 32718, 32572, 32329,
    31991, 31561, 31041, 30434, 29744, 28977, 28136, 27227, 26257, 25231,
    24156, 23039, 21888, 20709, 19511, 18301, 17088, 15878, 14680, 13501,
    12350, 11233, 10158,  9132,  8162,  7253,  6412,  5645,  4955,  4348,
    3828,  3397,  3059,  2817,  2670,  2621
};


/**************************************************************************
 *
 *    ROUTINE                :    Autocorr
 *
 *    DESCRIPTION            :    Compute autocorrelations
 *
 **************************************************************************
 *
 *    USAGE                :    Autocorr(buffer_in,p,buffer_out1,buffer_out2)
 *                            (Routine_Name(input1,input2,output1,output2))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : Input signal buffer
 *                        - Format : int16_t
 *
 *    INPUT2            :    - Description : LPC order
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Autocorrelations buffer (msb)
 *                            - Format : int16_t
 *
 *        OUTPUT2            :    - Description : Autocorrelations buffer (lsb)
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::dsp_autocorr(int16_t * x, int16_t p, int16_t * r_h, int16_t * r_l)
{
    int16_t i, j, norm;
    int16_t y[L_WINDOW];
    int32_t sum;

    // Windowing of signal

    for (i=0; i<L_WINDOW; i++)
        y[i] = op_mult_r(x[i], WIND[i]);

    // Compute r[0] and test for overflow

    do {
        g_overflow = 0;
        sum = 1;                                                                // Avoid case of all zeros
        for (i=0; i<L_WINDOW; i++)
            sum = op_lmac0(sum, y[i], y[i]);

        // If overflow divide y[] by 4

        if (g_overflow != 0)
        {
            for (i=0; i<L_WINDOW; i++)
                y[i] = op_shr(y[i], (int16_t)2);
        }

    } while (g_overflow != 0);

    // Normalization of r[0]

    norm = op_norm_l(sum);
    sum  = op_lshl(sum, norm);
    sum  = op_lshr(sum, (int16_t)1);                                            // For special double format
    op_lextract(sum, &r_h[0], &r_l[0]);

    // r[1] to r[p]

    for (i = 1; i <= p; i++)
    {
        sum = 0;
        for (j=0; j<L_WINDOW-i; j++)
            sum = op_lmac0(sum, y[j], y[j+i]);

        sum = op_lshr(sum, (int16_t)1);                                         // Special double format
        sum = op_lshl(sum, norm);
        op_lextract(sum, &r_h[i], &r_l[i]);
    }
}

/**************************************************************************
 *
 *    ROUTINE                :    Az_Lsp
 *
 *    DESCRIPTION            :    Compute the LSPs  in the cosine domain
 *                            from the LPC coefficients  (order=10)
 *
 **************************************************************************
 *
 *    USAGE                :    Az_Lsp(buffer_in1,buffer_out,buffer_in2)
 *                            (Routine_Name(input1,output1,input2))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : Predictor coefficients
 *                        - Format : int16_t - Q12
 *
 *    INPUT2            :    - Description : Previous LSP values
 *                                      (in case not 10 roots are found)
 *                        - Format : int16_t - Q15
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Line spectral pairs in the
 *                                          cosine domain
 *                            - Format : int16_t - Q15
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::dsp_az_lsp(int16_t * a, int16_t * lsp, int16_t * old_lsp)
{
    int16_t i, j, nf, ip;
    int16_t xlow, ylow, xhigh, yhigh, xmid, ymid, xint;
    int16_t x, y, sign, exp;
    int16_t *coef;
    int16_t f1[PP/2+1], f2[PP/2+1];
    int32_t t0;

    // -------------------------------------------------------------
    //  find the sum and diff. pol. F1(z) and F2(z)
    //     F1(z) <--- F1(z)/(1+z**-1) & F2(z) <--- F2(z)/(1-z**-1)
    //
    //  f1[0] = 1.0;
    //  f2[0] = 1.0;
    //
    //  for (i = 0; i< NC; i++)
    //  {
    //    f1[i+1] = a[i+1] + a[PP-i] - f1[i] ;
    //    f2[i+1] = a[i+1] - a[PP-i] + f2[i] ;
    //  }
    // -------------------------------------------------------------

    f1[0] = 2048;                                                               // f1[0] = 1.0 is in Q11
    f2[0] = 2048;                                                               // f2[0] = 1.0 is in Q11

    for (i = 0; i< NC; i++)
    {
        t0 = op_load_sh(a[i+1], (int16_t)15);                                   // a[i+1]  in Q27
        t0 = op_add_sh(t0, a[PP-i], (int16_t)15);                               // +a[PP-i] in Q27
        t0 = op_sub_sh16(t0, f1[i]);                                            // -f1[i]  in Q27
        f1[i+1] = op_extract_h(t0);                                             // f1[i+1] = a[i+1] + a[PP-i] - f1[i]
        // result in Q11

        t0 = op_load_sh(a[i+1], (int16_t)15);                                   // a[i+1]  in Q27
        t0 = op_sub_sh(t0, a[PP-i], (int16_t)15);                               // -a[PP-i] in Q27
        t0 = op_add_sh16(t0, f2[i]);                                            // +f2[i] in Q27
        f2[i+1] = op_extract_h(t0);                                             // f2[i+1] = a[i+1] - a[PP-i] + f2[i]
        // result in Q11
    }

    // find the LSPs using the Chebichev pol. evaluation

    nf=0;                                                                       // number of found frequencies
    ip=0;                                                                       // indicator for f1 or f2

    coef = f1;

    xlow = grid[0];
    ylow = dsp_chebps(xlow, coef, NC);

    j = 0;
    while ((nf < PP) && (j < GRID_POINTS))
    {
        j++;
        xhigh = xlow;
        yhigh = ylow;
        xlow  = grid[j];
        ylow  = dsp_chebps(xlow,coef,NC);

        if (op_lmult0(ylow,yhigh) <= (int32_t)0)
        {
            // divide 4 times the interval

            for (i = 0; i < 4; i++)
            {
                t0   = op_load_sh(xlow, (int16_t)15);                           // xmid = 0.5*(xlow + xhigh)
                t0   = op_add_sh(t0, xhigh, (int16_t)15);
                xmid = op_extract_h(t0);

                ymid = dsp_chebps(xmid,coef,NC);

                if (op_lmult0(ylow,ymid) <= (int32_t)0)
                {
                    yhigh = ymid;
                    xhigh = xmid;
                }
                else
                {
                    ylow = ymid;
                    xlow = xmid;
                }
            }

            // Linear interpolation
            //    xint = xlow - ylow*(xhigh-xlow)/(yhigh-ylow);

            x   = op_sub(xhigh, xlow);
            y   = op_sub(yhigh, ylow);

            if (y == 0)
            {
                xint = xlow;
            }
            else
            {
                sign= y;
                y   = op_abs_s(y);
                exp = op_norm_s(y);
                y   = op_shl(y, exp);
                y   = op_div_s((int16_t)16383, y);
                t0  = op_lmult0(x, y);
                t0  = op_lshr(t0, op_sub((int16_t)19,exp));
                y   = op_extract_l(t0);                                         // y = (xhigh-xlow)/(yhigh-ylow) in Q10

                if (sign < 0) y = op_negate(y);

                t0   = op_load_sh(xlow, (int16_t)10);                           // xint = xlow - ylow*y
                t0   = op_lmsu0(t0, ylow, y);
                xint = op_store_hi(t0, (int16_t)6);

            }

            lsp[nf] = xint;
            xlow    = xint;
            nf++;

            if (ip == 0)
            {
                ip = 1;
                coef = f2;
            }
            else
            {
                ip = 0;
                coef = f1;
            }
            ylow = dsp_chebps(xlow,coef,NC);

        }
    }

    // Check if PP roots found

    if (op_sub(nf, PP) < 0)
    {
        for (i=0; i<PP; i++)
            lsp[i] = old_lsp[i];
        printf("\n !!Not 10 roots found in Az_Lsp()\n");
    }
}

/**************************************************************************
 *
 *    ROUTINE                :    Back_Fil
 *
 *    DESCRIPTION            :    Perform the Backward filtering of input vector
 *                        buffer_in1 with buffer_in2 and write the result
 *                        in output vector buffer_out.
 *                            All vectors are of length L
 *
 **************************************************************************
 *
 *    USAGE                :    Back_Fil(buffer_in1,buffer_in2,buffer_out,L)
 *                            (Routine_Name(input1,input2,output1,input3))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : Input vector
 *                        - Format : int16_t
 *
 *    INPUT2            :    - Description : Impulse response
 *                        - Format : int16_t - Q12
 *
 *    INPUT3            :    - Description : Vector size
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Backward filtering result
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::dsp_back_fil(int16_t * x, int16_t * h, int16_t * y, int16_t L)
{
    int16_t i, j;
    int32_t s, max;
    int32_t y32[60];                                                            // Usually, dynamic allocation of L

    // first keep the result on 32 bits and find absolute maximum

    max = 0;

    for (i = 0; i < L; i++)
    {
        s = 0;
        for (j = i; j <  L; j++)
            s = op_lmac0(s, x[j], h[j-i]);

        y32[i] = s;

        s = op_labs(s);
        if (op_lsub(s, max) > 0) max = s;
    }

    // Find the number of right shifts to do on y32[]
    // so that maximum is on 13 bits

    j = op_norm_l(max);
    if (op_sub(j,(int16_t)16) > 0) j = 16;

    j = op_sub((int16_t)18, j);

    for (i=0; i<L; i++)
        y[i] = op_extract_l(op_lshr(y32[i], j));
}

/**************************************************************************
 *
 *    ROUTINE                :    Chebps
 *
 *    DESCRIPTION            :    Evaluate the Chebishev polynomial series
 *
 **************************************************************************
 *
 *    USAGE                :    Chebps(x,buffer_in,n)
 *                            (Routine_Name(input1,input2,input3))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : Input value of evaluation
 *                                      x = cos(frequency)
 *                        - Format : int16_t - Q15
 *
 *    INPUT2            :    - Description : Coefficients of the polynomial series
 *                        - Format : int16_t - Q11
 *
 *    INPUT3            :    - Description : Polynomial order
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :    None
 *
 *    RETURNED VALUE        :    - Description : Value of the polynomial C(x)
 *                            - Format : int16_t - Q14
 *                                 (saturated to +/- 1.99)
 *
 *    COMMENTS            :    - The polynomial order is :
 *                        n = p/2   (p is the prediction order)
 *
 *                        - The polynomial is given by :
 *                            C(x)=T_n(x)+f(1)T_n-1(x)+...+f(n-1)T_1(x)+ f(n)/2
 *
 **************************************************************************/

int16_t sdecoder::dsp_chebps(int16_t x, int16_t * f, int16_t n)
{
    int16_t i, cheb;
    int16_t b0_h, b0_l, b1_h, b1_l, b2_h, b2_l;
    int32_t t0;

    // Note: All computation are done in Q24.

    b2_h = 512;                                                                 // b2 = 1.0 in Q24 DPF
    b2_l = 0;

    t0 = op_load_sh(x,(int16_t)10);                                             // 2*x in Q24
    t0 = op_add_sh(t0, f[1], (int16_t)13);                                      // + f[1] in Q24
    op_lextract(t0, &b1_h, &b1_l);                                              // b1 = 2*x + f[1]

    for (i = 2; i<n; i++)
    {
        t0 = op_mpy_mix(b1_h, b1_l, x);                                         // t0 = x*b1
        t0 = op_lshl(t0,(int16_t)1);                                            // t0 = 2.0*x*b1
        t0 = op_sub_sh(t0, b2_l, (int16_t)0);                                   // t0 = 2.0*x*b1 - b2
        t0 = op_sub_sh(t0, b2_h, (int16_t)15);
        t0 = op_add_sh(t0, f[i], (int16_t)13);                                  // t0 = 2.0*x*b1 - b2 + f[i];

        op_lextract(t0, &b0_h, &b0_l);                                          // b0 = 2.0*x*b1 - b2 + f[i];
        b2_l = b1_l;                                                            // b2 = b1;
        b2_h = b1_h;
        b1_l = b0_l;                                                            // b1 = b0;
        b1_h = b0_h;
    }
    t0 = op_mpy_mix(b1_h, b1_l, x);                                             // t0    = x*b1;
    t0 = op_sub_sh(t0, b2_l, (int16_t)0);                                       // t0   -= b2;
    t0 = op_sub_sh(t0, b2_h, (int16_t)15);
    t0 = op_add_sh(t0, f[n], (int16_t)12);                                      // t0   += 0.5*f[n];

    t0 = op_lshl(t0, (int16_t)6);                                               // Q24 to Q30 with saturation
    cheb = op_extract_h(t0);                                                    // Result in Q14

    return cheb;
}

/**************************************************************************
 *
 *    ROUTINE                :    Convolve
 *
 *    DESCRIPTION            :    Perform the convolution between two input vectors
 *                        buffer_in1 and buffer_in2 and write the result in
 *                        output vector buffer_out.
 *                            All vectors are of length L
 *
 **************************************************************************
 *
 *    USAGE                :    Convolve(buffer_in1,buffer_in2,buffer_out,L)
 *                            (Routine_Name(input1,input2,output1,input3))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : Input vector
 *                        - Format : int16_t
 *
 *    INPUT2            :    - Description : Impulse response
 *                        - Format : int16_t - Q12
 *
 *    INPUT3            :    - Description : Vector size
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Output vector
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::dsp_convolve(int16_t * x, int16_t * h, int16_t * y, int16_t L)
{
    int16_t i, n;
    int32_t s;

    for (n = 0; n < L; n++)
    {
        s = 0;
        for (i = 0; i <= n; i++)
            s = op_lmac0(s, x[i], h[n-i]);

        y[n] = op_store_hi(s, (int16_t)4);                                      // h is in Q12
    }
}

/**************************************************************************
 *
 *    ROUTINE                :    Fac_Pond
 *
 *    DESCRIPTION            :    Compute LPC spectral expansion factors (fac[])
 *                            with the LPC order fixed to 10
 *
 **************************************************************************
 *
 *    USAGE                :    Fac_Pond(gamma,buffer_out)
 *                            (Routine_Name(input1,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : Spectral expansion
 *                        - Format : int16_t - Q15
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Computed expansion factors
 *                            - Format : int16_t - Q15
 *
 *    RETURNED VALUE        :    None
 *
 *    COMMENTS            :    - fac[0] = gamma
 *                            - fac[i] = fac[i-1] * gamma    i=1,9
 *
 **************************************************************************/

void sdecoder::dsp_fac_pond(int16_t gamma, int16_t * fac)
{
    int16_t i;

    fac[0] = gamma;
    for (i=1; i<PP; i++)
        fac[i] = op_etsi_round(op_lmult(fac[i-1], gamma));
}

/**************************************************************************
 *
 *    ROUTINE                :    Get_Lsp_Pol
 *
 *    DESCRIPTION            :    Find the polynomial F1(z) or F2(z) from the LSPs
 *
 **************************************************************************
 *
 *    USAGE                :    Get_Lsp_Pol(buffer_in,buffer_out)
 *                            (Routine_Name(input1,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : line spectral pairs
 *                                      (cosine domaine)
 *                        - Format : int16_t - Q15
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Coefficients of F1 or F2
 *                            - Format : int32_t - Q24
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::dsp_get_lsp_pol(int16_t * lsp, int32_t * f)
{
    int16_t i,j, hi, lo;
    int32_t t0;

    // Computation in Q24

    *f = op_load_sh((int16_t)4096,(int16_t)12);                                 // f[0] = 1.0;           in Q24
    f++;
    *f = 0;
    *f = op_sub_sh(*f, *lsp, (int16_t)10);                                      // f[1] = -2.0 * lsp[0]; in Q24
    f++;
    lsp += 2;                                                                   // Advance lsp pointer

    for (i=2; i<=5; i++)
    {
        *f = f[-2];

        for (j=1; j<i; j++, f--)
        {
            op_lextract(f[-1] ,&hi, &lo);
            t0 = op_mpy_mix(hi, lo, *lsp);                                      // t0 = f[-1] * lsp
            t0 = op_lshl(t0, (int16_t)1);
            *f = op_ladd(*f, f[-2]);                                            // *f += f[-2]
            *f = op_lsub(*f, t0);                                               // *f -= t0
        }
        *f   = op_sub_sh(*f, *lsp, (int16_t)10);                                // *f -= lsp<<10
        f   += i;                                                               // Advance f pointer
        lsp += 2;                                                               // Advance lsp pointer
    }
}

/**************************************************************************
 *
 *    ROUTINE                :    Int_Lpc4
 *
 *    DESCRIPTION            :    Perform the LPC interpolation for the 4 sub-frames.
 *                            The interpolation is done on the LSP computed
 *                            in the cosine domain
 *
 **************************************************************************
 *
 *    USAGE                :    Int_Lpc4(buffer_in1,buffer_in2,buffer_out)
 *                            (Routine_Name(input1,input2,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : LSP of previous frame
 *                        - Format : int16_t
 *
 *    INPUT2            :    - Description : LSP of current frame
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : LPC coeff. vector for the 4 sub-frames
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::dsp_int_lpc4(int16_t * lsp_old, int16_t * lsp_new, int16_t * a)
{
    int16_t i, j, fac_new, fac_old;
    int16_t lsp[PP];
    int32_t t0;

    fac_new = 8192;                                                             // 1/4 in Q15
    fac_old = 24576;                                                            // 3/4 in Q15

    for (j=0; j<33; j+=11)
    {
        for (i=0; i<PP; i++)
        {
            t0 = op_lmult(lsp_old[i], fac_old);
            t0 = op_lmac(t0, lsp_new[i], fac_new);
            lsp[i] = op_extract_h(t0);
        }
        dsp_lsp_az(lsp, &a[j]);

        fac_old = op_sub(fac_old, (int16_t)8192);
        fac_new = op_add(fac_new, (int16_t)8192);
    }
    dsp_lsp_az(lsp_new, &a[33]);
}

/**************************************************************************
 *
 *    ROUTINE                :    Lag_Window
 *
 *    DESCRIPTION            :    Lag_window on autocorrelations
 *
 **************************************************************************
 *
 *    USAGE                :    Lag_Window(p,buffer1,buffer2)
 *                            (Routine_Name(input1,arg2,arg3))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description :  LPC order
 *                        - Format : int16_t
 *
 *    ARG2                :    - Description : Autocorrelations (msb)
 *                        - Format : int16_t
 *
 *    ARG3                :    - Description : Autocorrelations (lsb)
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *    ARG2                :    - Description : Lag_Windowed autocorrelations (msb)
 *                        - Format : int16_t
 *
 *    ARG3                :    - Description : Lag_Windowed autocorrelations (lsb)
 *                        - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 *    COMMENTS            :    r[i] *= lag_wind[i]
 *
 *                        r[i] and lag_wind[i] are in special extended precision
 *                        (for details on this format, see "comments"
 *                        in the header of the file fexp_tet.c - Section A.5.2)
 *
 **************************************************************************/

void sdecoder::dsp_lag_window(int16_t p, int16_t * r_h, int16_t * r_l)
{
    int16_t i;
    int32_t x;

    for (i=1; i<=p; i++)
    {
        x = op_mpy_32(r_h[i], r_l[i], LAG_H[i-1], LAG_L[i-1]);
        op_lextract(x, &r_h[i], &r_l[i]);
    }
}

/**************************************************************************
 *
 *    ROUTINE                :    Levin_32
 *
 *    DESCRIPTION            :    Computation of 10 LPC coefficients
 *                            based on the Levison-Durbin algorithm
 *                            in double precision
 *
 **************************************************************************
 *
 *    USAGE                :    Levin_32(buffer_in1,buffer_in2,buffer_out)
 *                            (Routine_Name(input1,input2,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : Vector of autocorrelations (msb)
 *                        - Format : int16_t - 11 values
 *
 *    INPUT2            :    - Description : Vector of autocorrelations (lsb)
 *                        - Format : int16_t - 11 values
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : LPC coefficients
 *                            - Format : int16_t - Q12
 *
 *    RETURNED VALUE        :    None
 *
 *    COMMENTS            :    Algorithm :
 *
 *                            R[i]    autocorrelations
 *                            A[i]    filter coefficients
 *                            K     reflection coefficients
 *                            Alpha    prediction gain.
 *
 *                        Initialisations :
 *
 *                            A[0] = 1
 *                            K    = -R[1]/R[0]
 *                            A[1] = K
 *                            Alpha = R[0] * (1-K**2]
 *
 *                        DO for  i = 2 to PP
 *
 *                            S =  SUM (R[j]*A[i-j] ,j=1,i-1) +  R[i]
 *                            K = -S / Alpha
 *                             An[j] = A[j] + K*A[i-j]   for j=1 to i-1
 *                                where   An[i] = new A[i]
 *                            An[i]=K
 *                            Alpha=Alpha * (1-K**2)
 *
 *                        END
 *
 **************************************************************************

 **************************************************************************
 *
 *    NOTES ON THE DYNAMICS OF THE COMPUTATIONS    :
 *
 *    The numbers used are in double precision with the following format :
 *
 *    A = AH <<15 + AL.  AH and AL are 16 bit signed integers. Since the LSB's also contain
 *    a sign bit, this format does not correspond to standard 32 bit integers.  This format is used
 *    since it allows fast execution of multiplications and divisions.
 *
 *    "DPF" will refer to this special format in the following text.
 *    (for details on this format, see "comments" in the header of file fexp_tet.c - Section A.5.2)
 *
 *    The R[i] were normalized in routine Autocorr (hence, R[i] < 1.0).
 *    The K[i] and Alpha are theoretically < 1.0.
 *    The A[i], for a sampling frequency of 8 kHz, are in practice always inferior to 16.0.
 *
 *    These characteristics allow straigthforward fixed-point implementation.  The parameters are
 *    represented as follows :
 *
 *    R[i]    Q30   +- .99..
 *    K[i]    Q30   +- .99..
 *    Alpha   Normalised -> mantissa in Q30 plus exponant
 *    A[i]    Q26   +- 15.999..
 *
 *    The additions are performed in 32 bit.  For the summation used to compute the K[i],
 *    numbers in Q30 are multiplied by numbers in Q26, with the results of the multiplications in
 *    Q26, resulting in a dynamic of +/- 32.  This is sufficient to avoid overflow, since the final
 *    result of the summation is necessarily < 1.0 as both the K[i] and Alpha are
 *    theoretically < 1.0.
 *
 **************************************************************************/

// Last A(z) for case of unstable filter

//static int16_t old_A[PP+1]={4096,0,0,0,0,0,0,0,0,0,0};

void sdecoder::dsp_levin_32(int16_t * Rh, int16_t * Rl, int16_t * A)
{
    int16_t i, j;
    int16_t hi, lo;
    int16_t Kh, Kl;                                                             // reflexion coefficient; hi and lo
    int16_t alp_h, alp_l, alp_e;                                                // Prediction gain; hi lo and exponant
    int16_t Ah[PP+1], Al[PP+1];                                                 // LPC coef. in double prec.
    int16_t Anh[PP+1], Anl[PP+1];                                               // LPC coef.for next iterat. in double prec.
    int32_t t0, t1, t2;                                                         // temporary variable

    // K = A[1] = -R[1] / R[0]

    t1  = op_lcomp(Rh[1], Rl[1]);                                               // R[1] in Q30
    t2  = op_labs(t1);                                                          // abs R[1]
    t0  = op_div_32(t2, Rh[0], Rl[0]);                                          // R[1]/R[0] in Q30
    if (t1 > 0) t0= op_lnegate(t0);                                             // -R[1]/R[0]
    op_lextract(t0, &Kh, &Kl);                                                  // K in DPF
    t0 = op_lshr(t0,(int16_t)4);                                                // A[1] in Q26
    op_lextract(t0, &Ah[1], &Al[1]);                                            // A[1] in DPF


    // Alpha = R[0] * (1-K**2)

    t0 = op_mpy_32(Kh ,Kl, Kh, Kl);                                             // K*K      in Q30
    t0 = op_labs(t0);                                                           // Some case <0 !!
    t0 = op_lsub((int32_t)0x3fffffff, t0);                                      // 1 - K*K  in Q30
    op_lextract(t0, &hi, &lo);                                                  // DPF format
    t0 = op_mpy_32(Rh[0] ,Rl[0], hi, lo);                                       // Alpha in Q30

    // Normalize Alpha

    t0 = op_norm_v(t0, (int16_t)12, &i);
    t0 = op_lshr(t0, (int16_t)1);
    op_lextract(t0, &alp_h, &alp_l);                                            // DPF format
    alp_e = i-1;                                                                // t0 was in Q30

    // ITERATIONS  I=2 to PP

    for (i= 2; i<=PP; i++)
    {
        // t0 = SUM (R[j]*A[i-j] ,j=1,i-1) +  R[i]

        t0 = 0;
        for (j=1; j<i; j++)
            t0 = op_ladd(t0, op_mpy_32(Rh[j], Rl[j], Ah[i-j], Al[i-j]));

        t0 = op_lshl(t0,(int16_t)4);                                            // result in Q26->convert to Q30
        // No overflow possible
        t1 = op_lcomp(Rh[i],Rl[i]);
        t0 = op_ladd(t0, t1);                                                   // add R[i] in Q30

        // K = -t0 / Alpha

        t1 = op_labs(t0);
        t2 = op_div_32(t1, alp_h, alp_l);                                       // abs(t0)/Alpha
        if (t0 > 0) t2= op_lnegate(t2);                                         // K =-t0/Alpha
        t2 = op_lshl(t2, alp_e);                                                // denormalize; compare to Alpha
        op_lextract(t2, &Kh, &Kl);                                              // K in DPF

        // Test for unstable filter, if unstable keep old A(z)

        if (op_abs_s(Kh) > 32750)
        {
            for (j=0; j<=PP; j++) A[j] = old_A[j];
            return;
        }

        //------------------------------------------
        //  Compute new LPC coeff. -> An[i]
        //  An[j]= A[j] + K*A[i-j]     , j=1 to i-1
        //  An[i]= K
        //------------------------------------------

        for (j=1; j<i; j++)
        {
            t0 = op_mpy_32(Kh, Kl, Ah[i-j], Al[i-j]);
            t0 = op_add_sh(t0, Ah[j], (int16_t)15);
            t0 = op_add_sh(t0, Al[j], (int16_t)0);
            op_lextract(t0, &Anh[j], &Anl[j]);
        }
        t2 = op_lshr(t2, (int16_t)4);                                           // t2 = K in Q30->convert to Q26
        op_lextract(t2, &Anh[i], &Anl[i]);                                      // An[i] in Q26

        //  Alpha = Alpha * (1-K**2)

        t0 = op_mpy_32(Kh ,Kl, Kh, Kl);                                         // K*K      in Q30
        t0 = op_labs(t0);                                                       // Some case <0 !!
        t0 = op_lsub((int32_t)0x3FFFFFFF, t0);                                  // 1 - K*K  in Q30
        op_lextract(t0, &hi, &lo);                                              // DPF format
        t0 = op_mpy_32(alp_h , alp_l, hi, lo);                                  // Alpha in Q30

        // Normalize Alpha

        t0 = op_norm_v(t0, (int16_t)12, &j);
        t0 = op_lshr(t0, (int16_t)1);
        op_lextract(t0, &alp_h, &alp_l);                                        // DPF format
        alp_e += j-1;                                                           // t0 was in Q30

        // A[j] = An[j]   Note: can be done with pointers

        for (j=1; j<=i; j++)
        {
            Ah[j] =Anh[j];
            Al[j] =Anl[j];
        }
    }

    // Truncate A[i] in Q26 to Q12 with rounding

    A[0] = 4096;
    for (i=1; i<=PP; i++)
    {
        t0   = op_lcomp(Ah[i], Al[i]);
        t0   = op_add_sh(t0, (int16_t)1, (int16_t)13);                          // rounding
        old_A[i] = A[i] = op_store_hi(t0,(int16_t)2);
    }
}

/**************************************************************************
 *
 *    ROUTINE                :    Lpc_Gain
 *
 *    DESCRIPTION            :    Compute energy of impulse response of 1/A(z)
 *                            on 60 points
 *
 **************************************************************************
 *
 *    USAGE                :    Lpc_Gain(buffer_in)
 *                            (Routine_Name(input1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : LPC coefficients
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :    None
 *
 *    RETURNED VALUE        :    - Description : Energy of impulse response of 1/A(z)
 *                            - Format : int32_t - Q20
 *
 **************************************************************************/

int32_t sdecoder::dsp_lpc_gain(int16_t * a)
{
    int16_t i;
    int32_t ener;
    int16_t h[LLG];

    // Compute the impulse response of A(z)

    h[0] = 1024;                                                                // 1.0 in Q10
    for (i=1; i<LLG; i++) h[i]=0;
    dsp_syn_filt(a, h, h, LLG, &h[1], (int16_t)0);

    // Compute the energy of the impulse response

    ener = 0;
    for (i=0; i<LLG; i++)
        ener = op_lmac0(ener, h[i], h[i]);

    return ener;
}

/**************************************************************************
 *
 *    ROUTINE                :    Lsp_Az
 *
 *    DESCRIPTION            :    Compute the LPC coefficients
 *                            from the LSPs in the cosine domain
 *                            (order=10)
 *
 **************************************************************************
 *
 *    USAGE                :    Lsp_Az(buffer_in,buffer_out)
 *                            (Routine_Name(input1,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *        INPUT1            :    - Description : Line spectral pairs in the
 *                                          cosine domain
 *                        - Format : int16_t - Q15
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *    OUTPUT1            :    - Description : Predictor coefficients
 *                            - Format : int16_t - Q12
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::dsp_lsp_az(int16_t * lsp, int16_t * a)
{
    int16_t i, j;
    int32_t f1[6], f2[6];
    int32_t t0;

    dsp_get_lsp_pol(&lsp[0],f1);
    dsp_get_lsp_pol(&lsp[1],f2);

    for (i = 5; i > 0; i--)
    {
        f1[i] = op_ladd(f1[i], f1[i-1]);                                        // f1[i] += f1[i-1];
        f2[i] = op_lsub(f2[i], f2[i-1]);                                        // f2[i] -= f2[i-1];
    }

    a[0] = 4096;
    for (i = 1, j = 10; i <= 5; i++, j--)
    {
        t0   = op_ladd(f1[i], f2[i]);                                           // f1[i] + f2[i]
        a[i] = op_extract_l(op_lshr_r(t0,(int16_t)13));                         //from Q24 to Q12 and * 0.5
        t0   = op_lsub(f1[i], f2[i]);                                           // f1[i] - f2[i]
        a[j] = op_extract_l(op_lshr_r(t0,(int16_t)13));                         //from Q24 to Q12 and * 0.5
    }
}

/**************************************************************************
 *
 *    ROUTINE                :    Pond_Ai
 *
 *    DESCRIPTION            :    Compute spectral expansion (a_exp[]) of LPC
 *                            coefficients (a[]) using spectral expansion
 *                            factors (fac[]) with the LPC order fixed to 10
 *
 **************************************************************************
 *
 *    USAGE                :    Pond_Ai(buffer_in1,buffer_in2,buffer_out)
 *                            (Routine_Name(input1,input2,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : LPC coefficients
 *                        - Format : int16_t
 *
 *    INPUT2            :    - Description : Spectral expansion factors
 *                        - Format : int16_t - Q15
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Spectral expanded LPC coefficients
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 *    COMMENTS            :    - a_exp[i] = a[i] * fac[i-1]    i=1,10
 *
 **************************************************************************/

void sdecoder::dsp_pond_ai(int16_t * a, int16_t * fac, int16_t * a_exp)
{
    int16_t i;

    a_exp[0] = a[0];
    for (i=1; i<=PP; i++)
        a_exp[i] = op_etsi_round(op_lmult(a[i], fac[i-1]));
}

/**************************************************************************
 *
 *    ROUTINE                :    Residu
 *
 *    DESCRIPTION            :    Compute the LPC residual  by filtering the input
 *                            speech through A(z)
 *
 **************************************************************************
 *
 *    USAGE                :    Residu(buffer_in1,buffer_in2,buffer_out,lg)
 *                            (Routine_Name(input1,input2,output1,input3))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : Prediction coefficients
 *                        - Format : int16_t - Q12
 *
 *    INPUT2            :    - Description : Input speech - values of buffer_in2[]
 *                                      from -p to -1 are needed
 *                        - Format : int16_t
 *
 *    INPUT3            :    - Description : Size of filtering
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Residual signal
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::dsp_residu(int16_t * a, int16_t * x, int16_t * y, int16_t lg)
{
    int16_t i, j;
    int32_t s;

    for (i = 0; i < lg; i++)
    {
        s = op_load_sh(x[i], (int16_t)12);
        for (j = 1; j <= PP; j++)
            s = op_lmac0(s, a[j], x[i-j]);

        s = op_add_sh(s, (int16_t)1, (int16_t)11);                              // Rounding
        s = op_lshl(s, (int16_t)4);                                             // Saturation
        y[i] = op_extract_h(s);
    }
}

/**************************************************************************
 *
 *    ROUTINE                :    Syn_Filt
 *
 *    DESCRIPTION            :    Perform the synthesis filtering 1/A(z)
 *
 **************************************************************************
 *
 *    USAGE                :    Syn_Filt(buffer_in1,buffer_in2,buffer_out,
 *                            lg,buffer,flag)
 *                            (Routine_Name(input1,input2,output1,
 *                            input3,arg4,input5))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    INPUT1            :    - Description : Prediction coefficients
 *                        - Format : int16_t - Q12
 *
 *    INPUT2            :    - Description : Input signal
 *                        - Format : int16_t
 *
 *    INPUT3            :    - Description : Size of filtering
 *                        - Format : int16_t
 *
 *    ARG4                :    - Description : Memory associated with this filtering
 *                        - Format : int16_t
 *
 *        INPUT5            :    - Description :    - flag = 0  -> no update of memory
 *                                    - flag = 1  -> update of memory
 *                            - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Output signal
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::dsp_syn_filt(int16_t * a, int16_t * x, int16_t * y, int16_t lg, int16_t * mem, int16_t update)
{
    int16_t i, j;
    int32_t s;
    int16_t tmp[80];                                                            // This is usually done by memory allocation (lg+PP)
    int16_t *yy;

    // Copy mem[] to yy[]

    yy = tmp;

    for (i=0; i<PP; i++)
        *yy++ = mem[i];

    // Do the filtering.

    for (i = 0; i < lg; i++)
    {
        s = op_load_sh(x[i], (int16_t)12);                                      // a[] is in Q12
        for (j = 1; j <= PP; j++)
            s = op_lmsu0(s, a[j], yy[-j]);

        s     = op_add_sh(s, (int16_t)1, (int16_t)11);                          // Rounding
        *yy++ = op_extract_h(op_lshl(s, (int16_t)4));
    }

    for (i=0; i<lg; i++) y[i] = tmp[i+PP];

    // Update of memory if update==1

    if (update != 0)
        for (i = 0; i < PP; i++) mem[i] = y[lg-PP+i];
}
