#include "sdecoder.h"

using namespace codec_sdecoder;

/**
 * Decode gains of pitch and innovative codebook
 *
 *    INPUT1            :    - Description : Index of energy quantizer
 *                        - Format : int16_t
 *
 *    INPUT2            :    - Description : Bad frame indicator
 *                        - Format : int16_t
 *
 *    INPUT3            :    - Description : LPC filter
 *                        - Format : int16_t
 *
 *    INPUT4            :    - Description : Adaptive codebook
 *                        - Format : int16_t
 *
 *        INPUT5            :    - Description : Innovation codebook
 *                            - Format : int16_t
 *
 *        INPUT6            :    - Description : Subframe length
 *                            - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Quantized pitch gain
 *                            - Format : int16_t
 *
 *        OUTPUT2            :    - Description : Quantized code gain
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    Index of energy quantizer
 *
 */

int16_t sdecoder::dec_ener(int16_t index, int16_t bfi,
                           int16_t * lpc_filter, int16_t * prd_lt, int16_t * code, int16_t subframe_len,
                           int16_t * gain_pit, int16_t * gain_cod)
{
    // Energy of impulse response of 1/lpc_filter(z) for length = 60

    int32_t val      = dsp_lpc_gain(lpc_filter);
    int16_t exp_lpc  = op_norm_l(val);
    int16_t ener_lpc = op_extract_h(op_lshl(val, exp_lpc));

    // Energy of adaptive codebook

    val = 1;                                                                    // Avoid case of all-zeros
    for (int16_t i = 0; i < subframe_len; i++)
        val = op_lmac0(val, prd_lt[i], prd_lt[i]);

    int16_t exp_plt  = op_norm_l(val);
    int16_t ener_plt = op_extract_h(op_lshl(val, exp_plt));

    // ener_plt = Log2(ener_plt * ener_lpc)

    val = op_lmult0(ener_plt, ener_lpc);
    exp_plt = op_add(exp_plt, exp_lpc);

    int16_t  vexp, vfrac;
    Log2(val, &vexp, &vfrac);

    val = op_load_sh16(vexp);
    val = op_add_sh(val, vfrac, (int16_t)1);                                    // Log2(ener_plt*ener_lpc) in Q16
    val = op_sub_sh16(val, exp_plt);                                            // subtract exponant of ener_plt

    // Input on 15 bits
    val = op_add_sh(val, (int16_t)1710, (int16_t)8);                            // +6.68 in Q16 for other scaling
    val = op_lshr(val, (int16_t)8);                                             // Convert from Q16 to Q8
    ener_plt = op_extract_l(val);

    // Energy coming from code

    val = 0;
    for (int16_t i = 0; i < subframe_len; i++)
        val = op_lmac0(val, code[i], code[i]);

    int16_t ener_c = op_extract_h(val);

    // ener_c = Log2(ener_c * ener_lpc)

    val = op_lmult0(ener_c, ener_lpc);

    Log2(val, &vexp, &vfrac);

    val = op_load_sh16(vexp);
    val = op_add_sh(val, vfrac, (int16_t)1);                                    // Log2(ener_plt*ener_lpc) in Q16
    val = op_sub_sh16(val, exp_lpc);                                            // subtract exponant of ener_lpc

    // Input on 15 bits
    val = op_sub_sh(val, (int16_t)4434, (int16_t)8);                            //-17.32 in Q16 for other scaling
    val = op_lshr(val, (int16_t)8);                                             // Convert from Q16 to Q8
    ener_c = op_extract_l(val);

    /*
     * Test for bfi.
     *
     *  if (bfi != 0)
     *    ->last_ener_pit -= 0.5
     *    ->last_ener_cod -= 0.5
     *  else
     *  {
     *    decode new last_ener_pit et last_ener_cod
     *  }
     */

    if (bfi != 0)
    {
        last_ener_pit = op_sub(last_ener_pit, (int16_t)128);                    // -0.5 in Q8
        if (last_ener_pit < 0) last_ener_pit = 0;

        last_ener_cod = op_sub(last_ener_cod, (int16_t)128);                    // -0.5 in Q8
        if (last_ener_cod < 0) last_ener_cod = 0;
    }
    else
    {
        /*
         * Prediction on pitch energy.
         *  pred_pit = 0.50 * last_ener_pit + 0.25 * last_ener_cod - 3.0
         *  if (pred_pit < 0.0) pred_pit = 0.0;
         */

        val = op_load_sh(last_ener_pit, (int16_t)8);                            // .5 last_ener_pit in Q9
        val = op_add_sh(val,last_ener_cod, (int16_t)7);                         //+.25 last_ener_code in Q9
        val = op_sub_sh(val, (int16_t)768, (int16_t)9);                         // -3.0 in Q9
        if (val < 0) val = 0;
        int16_t pred_pit = op_store_hi(val, (int16_t)7);                        // result in Q8

        /*
         * Prediction on code energy.
         *  pred_cod = 0.50 * last_ener_cod + 0.25 * last_ener_pit - 3.0
         *  if (pred_cod < 0.0) pred_cod = 0.0;
         */

        val = op_load_sh(last_ener_cod, (int16_t)8);                            // .5 last_ener_cod in Q9
        val = op_add_sh(val, last_ener_pit, (int16_t)7);                        //+.25 last_ener_pit in Q9
        val = op_sub_sh(val, (int16_t)768, (int16_t)9);                         // -3.0 in Q9
        if (val < 0) val = 0;
        int16_t pred_cod = op_store_hi(val, (int16_t)7);                        // result in Q8

        /*
         * Read quantized values.
         */

        int16_t j = op_shl(index, (int16_t)1);
        last_ener_pit = op_add(T_QUA_ENER[j],     pred_pit);
        last_ener_cod = op_add(T_QUA_ENER[j + 1], pred_cod);

        // Limit energies -> for transmission errors

        if (op_sub(last_ener_pit, (int16_t)6912) > 0) last_ener_pit = 6912;     // 6912 = 27 in Q8
        if (op_sub(last_ener_cod, (int16_t)6400) > 0) last_ener_cod = 6400;     // 6400 = 25 in Q8
    }

    /*
     *
     *  Compute the quantized pitch gain.
     *
     *     temp = 0.5 * (last_ener_pit - ener_plt);
     *     temp = pow(2.0, temp);
     *     if (temp > 1.2) temp = 1.2;
     *     *gain_pit = temp;
     *
     */

    val = op_load_sh(last_ener_pit, (int16_t)6);                                // last_ener_pit/2 in Q15
    val = op_sub_sh(val, ener_plt, (int16_t)6);                                 // - ener_plt/2 in Q15
    val = op_add_sh(val, (int16_t)12, (int16_t)15);                             // to have gain in Q12
    op_lextract(val, &vexp, &vfrac);
    val = pow2(vexp, vfrac);
    if (op_lsub(val, (int16_t)4915) > 0) val = 4915;                            // 4915 = 1.2 in Q12
    *gain_pit = op_extract_l(val);

    /*
     *
     *  Compute the innovative code gain.
     *
     *     temp = 0.5 * (last_ener_cod - ener_c);
     *     *gain_cod = pow(2.0, temp);
     *
     */

    val = op_load_sh(last_ener_cod, (int16_t)6);                                // last_ener_cod/2 in Q15
    val = op_sub_sh(val, ener_c, (int16_t)6);                                   // - ener_c/2 in Q15
    op_lextract(val, &vexp, &vfrac);
    val = pow2(vexp, vfrac);
    *gain_cod = op_extract_l(val);

    return index;
}

/**
 * Decode innovative codebook d4i60_16
 *
 *    INPUT1            :    - Description : Index of codebook
 *                        - Format : int16_t
 *
 *    INPUT2            :    - Description : Sign of codebook
 *                        - Format : int16_t
 *
 *    INPUT3            :    - Description : Shift of codebook
 *                        - Format : int16_t
 *
 *    INPUT4            :    - Description : Noise filter
 *                        - Format : int16_t - (must be preceeded by 64 zeros)
 *
 *        OUTPUT1            :    - Description : Innovative vector
 *                            - Format : int16_t
 *
 */

void sdecoder::d_d4i60(int16_t index, int16_t sign, int16_t shift, int16_t * F, int16_t * cod)
{
    // Position of the 4 impulses

    const int16_t LCODE  = 60;
    const int16_t Q11_GAIN_I0 = 2896;                                           // Gain for impulse 0 = sqrt(2) = 1.4142 = 2896 in Q11
    
    int16_t pos0 = op_shl((int16_t)(index & 31), (int16_t)1);

    int16_t pos1 = op_shr((int16_t)(index & 224), (int16_t)2);
    pos1 = op_add(pos1, (int16_t)2);

    int16_t pos2 = op_shr((int16_t)(index & 1792), (int16_t)5);
    pos2 = op_add(pos2, (int16_t)4);

    int16_t pos3 = op_shr((int16_t)(index & 14336), (int16_t)8);
    pos3 = op_add(pos3, (int16_t)6);

    // Convolve code with F[]
    // cod[i] = p0[i] * gain_i0  - p1[i] + p2[i] - p3[i];

    F -= shift;                                                                 // Operations on pointers
    int16_t * p0 = F - pos0;
    int16_t * p1 = F - pos1;
    int16_t * p2 = F - pos2;
    int16_t * p3 = F - pos3;

    if (sign == 0)
    {
        for (int16_t i = 0; i < LCODE; i++)
        {
            int32_t val  = op_lmult0(p0[i], (int16_t)Q11_GAIN_I0);
            val  = op_sub_sh(val, p1[i], (int16_t)11);
            val  = op_add_sh(val, p2[i], (int16_t)11);
            val  = op_sub_sh(val, p3[i], (int16_t)11);
            cod[i] = op_store_hi(val, (int16_t)5);
        }
    }
    else
    {
        for (int16_t i = 0; i < LCODE; i++)
        {
            int32_t val  = op_lmult0(p0[i], (int16_t)Q11_GAIN_I0);
            val  = op_sub_sh(val, p1[i], (int16_t)11);
            val  = op_add_sh(val, p2[i], (int16_t)11);
            val  = op_sub_sh(val, p3[i], (int16_t)11);
            val  = op_lnegate(val);
            cod[i] = op_store_hi(val, (int16_t)5);
        }
    }
}

/**
 * Decoding: Split vector quantization of LSP parameters
 *
 *    INPUT1            :    - Description : indices of the three selected
 *                                      codebook entries
 *                        - Format : int16_t
 *
 *        INPUT2            :    - Description : Previous LSP values
 *                                          (in the cosine domain)
 *                            - Format : int16_t
 *
 *        OUTPUT1            :    - Description : Quantized LSPs (in the cosine domain)
 *                            - Format : int16_t
 *
 */

void sdecoder::d_lsp334(int16_t * index, int16_t * lsp, int16_t * lsp_old)
{
    int32_t pos = index[0] * 3;
    lsp[0] = ARRAY1_CLSP[pos];
    lsp[1] = ARRAY1_CLSP[pos + 1];
    lsp[2] = ARRAY1_CLSP[pos + 2];

    pos = index[1] * 3;
    lsp[3] = ARRAY2_CLSP[pos];
    lsp[4] = ARRAY2_CLSP[pos + 1];
    lsp[5] = ARRAY2_CLSP[pos + 2];
    
    pos = index[2] * 4;
    lsp[6] = ARRAY3_CLSP[pos];
    lsp[7] = ARRAY3_CLSP[pos + 1];
    lsp[8] = ARRAY3_CLSP[pos + 2];
    lsp[9] = ARRAY3_CLSP[pos + 3];
    
    // NOTE: this is bullshit programming method
    // const int16_t * p = &ARRAY1_CLSP[ index[0] * 3];
    // lsp[0] = *p++ ;
    // lsp[1] = *p++ ;
    // lsp[2] = *p++ ;

    // p      = &ARRAY2_CLSP[ index[1] * 3];
    // lsp[3] = *p++ ;
    // lsp[4] = *p++ ;
    // lsp[5] = *p++ ;

    // p      = &ARRAY3_CLSP[ index[2] * 4];
    // lsp[6] = *p++ ;
    // lsp[7] = *p++ ;
    // lsp[8] = *p++ ;
    // lsp[9] = *p++ ;

    // Minimum distance between lsp[2] and lsp[3]

    int16_t val = 917;                                                          // 917 = 0.028 in Q15 = 50hz around 1000hz
    val = op_sub(val, lsp[2]);
    val = op_add(val, lsp[3]);                                                  // val = 0.028 - (lsp[2]-lsp[3])
    if (val > 0)
    {
        val = op_shr(val, (int16_t)1);
        lsp[2] = op_add(lsp[2], val);
        lsp[3] = op_sub(lsp[3], val);
    }

    // Minimum distance between lsp[5] and lsp[6]

    val = 1245;                                                                 // 1245= 0.038 in Q15 = 50hz around 1600hz
    val = op_sub(val, lsp[5]);
    val = op_add(val, lsp[6]);                                                  // val = 0.038 - (lsp[5]-lsp[6])
    if (val > 0)
    {
        val = op_shr(val,(int16_t)1);
        lsp[5] = op_add(lsp[5], val);
        lsp[6] = op_sub(lsp[6], val);
    }

    // Verify if lsp[] are still in order

    val = 0;
    for (int16_t i = 0; i < 9; i++)
    {
        if (op_sub(lsp[i], lsp[i + 1]) <= 0)
        {
            val = 1;
        }
    }

    // If lsp[] are not in order keep old lsp[]

    if (val != 0)
    {
        for (int16_t i = 0; i < 10; i++)
            lsp[i] = lsp_old[i];
    }
}

/**************************************************************************
 *
 *    ROUTINE                :    Inter32_M1_3
 *
 *    DESCRIPTION            :    Fractional interpolation -1/3 with 32 coefficients
 *
 **************************************************************************
 *
 *    USAGE                :    Inter32_M1_3(buffer_in)
 *                            (Routine_Name(input1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *        INPUT1            :    - Description : Buffer containing the function
 *                                          to be interpolated
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :    None
 *
 *    RETURNED VALUE        :    - Description : Interpolated value
 *                        - Format : int16_t
 *
 *    COMMENTS            :    For long term prediction, it must be noted that
 *                            exc[-(T0-1/3)] corresponds to exc[-T0+1/3]
 *
 **************************************************************************/

int16_t sdecoder::inter32_m1_3(int16_t * x)
{
    const int16_t coef[32] = {
          -49,    66,   -96,   142,  -207,   294,  -407,   553,  -739,
          981, -1304,  1758, -2452,  3688, -6669, 27072, 13496, -5287,  3179,
        -2182,  1587, -1185,   893,  -672,   500,  -366,   263,  -183,   125,
          -84,    59,   -47};

    int32_t s = 0;

    for (int16_t i = 0; i < 32; i++)
    {
        s = op_lmac0(s, x[i - 15], coef[i]);
    }

    s = op_ladd(s, s);
    int16_t ret = op_etsi_round(s);
    
    return ret;
}

/**************************************************************************
 *
 *    ROUTINE                :    Inter32_1_3
 *
 *    DESCRIPTION            :    Fractional interpolation 1/3 with 32 coefficients
 *
 **************************************************************************
 *
 *    USAGE                :    Inter32_1_3(buffer_in)
 *                            (Routine_Name(input1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *        INPUT1            :    - Description : Buffer containing the function
 *                                          to be interpolated
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :    None
 *
 *    RETURNED VALUE        :    - Description : Interpolated value
 *                        - Format : int16_t
 *
 *    COMMENTS            :    For long term prediction, it must be noted that
 *                            exc[-(T0+1/3)] corresponds to exc[-T0-1/3]
 *
 **************************************************************************/

int16_t sdecoder::inter32_1_3(int16_t * x)
{
    const int16_t coef[32] = {
          -47,    59,   -84,   125,  -183,   263,  -366,   500,  -672,   893,
        -1185,  1587, -2182,  3179, -5287, 13496, 27072, -6669,  3688, -2452,
         1758, -1304,   981,  -739,   553,  -407,   294,  -207,   142,   -96,
           66,   -49};

    int32_t s = 0;
    for (int16_t i = 0; i < 32; i++)
    {
        s = op_lmac0(s, x[i - 16], coef[i]);
    }

    s = op_ladd(s, s);
    int16_t ret = op_etsi_round(s);
    
    return ret;
}


/**************************************************************************
 *
 *    ROUTINE                :    Pred_Lt
 *
 *    DESCRIPTION            :    Compute the result of long term prediction with
 *                            fractional interpolation
 *
 **************************************************************************
 *
 *    USAGE                :    Pred_Lt(buffer,T0,frac,L_subfr)
 *                            (Routine_Name(arg1,input2,input3,input4))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    ARG1                :    - Description : Excitation vector
 *                        - Format : int16_t
 *
 *    INPUT2            :    - Description : Pitch lag
 *                        - Format : int16_t
 *
 *    INPUT3            :    - Description : Fraction of pitch lag : (-1, 0, 1)  / 3
 *                        - Format : int16_t - Q12
 *
 *    INPUT4            :    - Description : Length of subframe
 *                        - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        ARG1                :    - Description : Interpolated signal contained in
 *                                         buffer[0..L_subfr-1]
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::codec_pred_lt(int16_t * exc, int16_t T0, int16_t frac, int16_t L_subfr)
{
    if (frac == 0)
    {
        for (int16_t i = 0; i < L_subfr; i++)
        {
            exc[i] = exc[i - T0];
        }
    }
    else if (op_sub(frac, (int16_t)1) == 0)
    {
        for (int16_t i = 0; i < L_subfr; i++)
        {
            exc[i] = inter32_1_3(&exc[i - T0]);
        }
    }
    else if (op_sub(frac, (int16_t)-1) == 0)
    {
        for (int16_t i = 0; i < L_subfr; i++)
        {
            exc[i] = inter32_m1_3(&exc[i - T0]);
        }
    }
}
