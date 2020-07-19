#include "sdecoder.h"
#include <string.h>

using namespace codec_sdecoder;

/**
 * @brief Constructor
 *
 */

sdecoder::sdecoder()
{

}

/**
 * @brief Process a 138 * 2 bytes speech frames and output a 240 * 2 bytes raw speech frame
 *
 */

void sdecoder::process_speech_frame(const int16_t * in_data, int16_t * raw_output)
{
    const int FRAME_LENGTH     = 138;                                           // 138 * 2 bytes
    int16_t data[FRAME_LENGTH] = {0};

    memcpy(data, in_data, FRAME_LENGTH * sizeof(int16_t));

    bits2prm_tetra(data, synth_param);                                          // serial to parameters
    decod_tetra(synth_param, raw_output);                                       // decode
    post_process(raw_output, (int16_t)240);                                     // post processing of synthesis
}


/**
 * @brief Initial lsp values used after each time a reset is executed
 *
 */

const int16_t sdecoder::lspold_init[PP0] = {
    30000, 26000, 21000, 15000, 8000, 0,
    -8000,-15000,-21000,-26000
};

/**
 * @brief Initialize variables for the speech decoder
 *
 */

void sdecoder::init_decod_tetra()
{
    old_T0 = 60;
    for (int16_t idx = 0; idx < 23; idx++)
    {
        old_parm[idx] = 0;
    }

    // Initialize static pointer

    exc = old_exc + PIT_MAX + L_INTER;

    // Initialize global variables

    last_ener_cod = 0;
    last_ener_pit = 0;

    // Static vectors to zero

    for (int16_t idx = 0; idx < PIT_MAX + L_INTER; idx++)
    {
        old_exc[idx] = 0;
    }

    for (int16_t idx = 0; idx < PP0; idx++)
    {
        mem_syn[idx] = 0;
    }

    // Initialisation of lsp values for first frame lsp interpolation

    for (int16_t idx = 0; idx < PP0; idx++)
    {
        lspold[idx] = lspold_init[idx];
    }

    // Compute LPC spectral expansion factors

    dsp_fac_pond(GAMMA3, F_gamma3);
    dsp_fac_pond(GAMMA4, F_gamma4);
}

/**
 * @brief Speech decoder function
 *
 * @param[in]  parm   Synthesis parameter (24 * 16 bit samples)
 * @param[out] synth  Output raw speech (240 * 16 bit samples)
 *
 */

void sdecoder::decod_tetra(int16_t * parm, int16_t * synth)
{
    // LPC coefficients

    int16_t A_t[(PP1) * 4];                                                     // A(z) unquantized for the 4 subframes
    int16_t Ap3[PP1];                                                           // A(z) with spectral expansion
    int16_t Ap4[PP1];                                                           // A(z) with spectral expansion
    int16_t * A;                                                                // Pointer on A_t

    // Other vectors

    int16_t zero_F[L_SUBFR + 64];
    int16_t * F;
    int16_t code[L_SUBFR + 4];

    // Scalars

    int16_t i_subfr;
    int16_t T0, T0_min, T0_max, T0_frac;
    int16_t gain_pit, gain_code, index;
    int16_t sign_code, shift_code;
    int16_t bfi, temp;
    int32_t L_temp;

    // Initialization of F

    F  = &zero_F[64];
    for (int16_t i = 0; i < 64; i++)
        zero_F[i] = 0;

    // Test bfi

    bfi = *parm++;

    if (bfi == 0)
    {
        d_lsp334(&parm[0], lspnew, lspold);                                     // lsp decoding

        for (int16_t i = 0; i < PARM_SIZE; i++)                                 // keep parm[] as old_parm
            old_parm[i] = parm[i];
    }
    else
    {
        for (int16_t i = 1; i < PP0; i++)
            lspnew[i] = lspold[i];

        for (int16_t i = 0; i < PARM_SIZE; i++)                                 // use old parm[]
            parm[i] = old_parm[i];
    }

    parm += 3;                                                                  // Advance synthesis parameters pointer

    // Interpolation of LPC for the 4 subframes

    dsp_int_lpc4(lspold, lspnew, A_t);

    // update the LSPs for the next frame

    for (int16_t i = 0; i < PP0; i++)
        lspold[i] = lspnew[i];

/*------------------------------------------------------------------------*
 *          Loop for every subframe in the analysis frame                 *
 *------------------------------------------------------------------------*
 * The subframe size is L_SUBFR and the loop is repeated L_FRAME/L_SUBFR  *
 *  times                                                                 *
 *     - decode the pitch delay                                           *
 *     - decode algebraic code                                            *
 *     - decode pitch and codebook gains                                  *
 *     - find the excitation and compute synthesis speech                 *
 *------------------------------------------------------------------------*/

    A = A_t;                                                                    // pointer to interpolated LPC parameters

    for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
    {
        index = *parm++;                                                        // pitch index

        if (i_subfr == 0)                                                       // if first subframe
        {
            if (bfi == 0)
            {                                                                   // if bfi == 0 decode pitch
                if (index < 197)
                {
                    // T0 = (index+2)/3 + 19; T0_frac = index - T0*3 + 58;

                    int16_t val;
                    val = op_add(index, (int16_t)2);
                    val = op_mult(val,  (int16_t)10923);                        // 10923 = 1/3 in Q15
                    T0  = op_add(val,   (int16_t)19);

                    val = op_add(T0, op_add(T0, T0) );                          // T0*3
                    val = op_sub((int16_t)58, (int16_t)val);
                    T0_frac = op_add(index, (int16_t)val);
                }
                else
                {
                    T0 = op_sub(index, (int16_t)112);
                    T0_frac = 0;
                }
            }
            else                                                                // bfi == 1
            {
                T0 = old_T0;
                T0_frac = 0;
            }

            // find T0_min and T0_max for other subframes

            T0_min = op_sub(T0, (int16_t)5);
            if (T0_min < PIT_MIN) T0_min = PIT_MIN;
            T0_max = op_add(T0_min, (int16_t)9);
            if (T0_max > PIT_MAX)
            {
                T0_max = PIT_MAX;
                T0_min = op_sub(T0_max, (int16_t)9);
            }
        }
        else                                                                    // other subframes
        {
            if (bfi == 0)                                                       // if bfi == 0 decode pitch
            {
                // T0 = (index+2)/3 - 1 + T0_min;

                int16_t val;
                val = op_add(index, (int16_t)2);
                val = op_mult(val,  (int16_t)10923);                            // 10923 = 1/3 in Q15
                val = op_sub(val,   (int16_t)1);
                T0 = op_add(T0_min, val);

                // T0_frac = index - 2 - i*3;

                val = op_add(val, op_add(val,val));                             // i*3
                T0_frac = op_sub(index , op_add(val, (int16_t)2) );
            }
        }

        /*-------------------------------------------------*
         * - Find the adaptive codebook vector.            *
         *-------------------------------------------------*/

        codec_pred_lt(&exc[i_subfr], T0, T0_frac, L_SUBFR);

        /*-----------------------------------------------------*
         * - Compute noise filter F[].                         *
         * - Decode codebook sign and index.                   *
         * - Find the algebraic codeword.                      *
         *-----------------------------------------------------*/

        dsp_pond_ai(A, F_gamma3, Ap3);
        dsp_pond_ai(A, F_gamma4, Ap4);

        for (int16_t i = 0;   i <= PP0;    i++) F[i] = Ap3[i];
        for (int16_t i = PP1; i < L_SUBFR; i++) F[i] = 0;

        dsp_syn_filt(Ap4, F, F, L_SUBFR, &F[PP1], (int16_t)0);

        // Introduce pitch contribution with fixed gain of 0.8 to F[]

        for (int16_t i = T0; i < L_SUBFR; i++)
        {
            temp = op_mult(F[i-T0], (int16_t)26216);
            F[i] = op_add(F[i], temp);
        }

        index = *parm++;
        sign_code  = *parm++;
        shift_code = *parm++;

        d_d4i60(index, sign_code, shift_code, F, code);

        /*-------------------------------------------------*
         * - Decode pitch and codebook gains.              *
         *-------------------------------------------------*/

        index = *parm++;        // index of energy VQ

        dec_ener(index,bfi,A,&exc[i_subfr],code, L_SUBFR, &gain_pit, &gain_code);

        /*-------------------------------------------------------*
         * - Find the total excitation.                          *
         * - Find synthesis speech corresponding to exc[].       *
         *-------------------------------------------------------*/

        for (int16_t i = 0; i < L_SUBFR;  i++)
        {
            // exc[i] = gain_pit*exc[i] + gain_code*code[i];
            // exc[i]  in Q0   gain_pit in Q12
            // code[i] in Q12  gain_cod in Q0

            L_temp = op_lmult0(exc[i+i_subfr], gain_pit);
            L_temp = op_lmac0(L_temp, code[i], gain_code);
            exc[i+i_subfr] = op_lshr_r(L_temp, (int16_t)12);
        }

        dsp_syn_filt(A, &exc[i_subfr], &synth[i_subfr], L_SUBFR, mem_syn, (int16_t)1);

        A  += PP1;    // interpolated LPC parameters for next subframe
    }

    /*--------------------------------------------------*
     * Update signal for next frame.                    *
     * -> shift to the left by L_FRAME  exc[]           *
     *--------------------------------------------------*/

    for (int16_t i = 0; i < PIT_MAX + L_INTER; i++)
        old_exc[i] = old_exc[i+L_FRAME];

    old_T0 = T0;
}

/**
 *  Convert serial received bits to the encoder parameter vector
 *    USAGE                :    Bits2prm_Tetra(buffer_in,buffer_out)
 *                            (Routine_Name(input1,output1))
 *
 *        INPUT1            :    - Description : Serial bits (137 + bfi)
 *                            - Format : int16_t - .. * 16 bit-samples
 *
 *        OUTPUT1            :    - Description : Encoded parameters
 *                                         (23 parameters + bfi)
 *                            - Format : int16_t - .. * 16 bit-samples
 *
 *    COMMENTS            :    The received parameters are :
 *
 *                        - BFI    bad frame indicator        1 bit
 *
 *                        - LSP    1st codebook             8 bits
 *                            2nd codebook            9 bits
 *                            3nd codebook            9 bits
 *
 *                        - for the 4 subframes :
 *                            pitch delay            8 bits (first)
 *                                            5 bits (others)
 *                            codebook index        14 bits
 *                            pulse global sign        1 bit
 *                            pulse shift            1 bit
 *                            pitch and innovation gains    6 bits
 *
 */

void sdecoder::bits2prm_tetra(int16_t * bits, int16_t * prm)
{
    const int16_t PRM_NO = 23;
    const int16_t bitno[PRM_NO] = {8, 9, 9,                                     // split VQ LSP
                                   8, 14, 1, 1, 6,                              // subframe 1
                                   5, 14, 1, 1, 6,                              // subframe 2
                                   5, 14, 1, 1, 6,                              // subframe 3
                                   5, 14, 1, 1, 6};                             // subframe 4
    *prm++ = *bits++;                                                           // read BFI

    for (int16_t i = 0; i < PRM_NO; i++)
    {
        prm[i] = op_bin2int(bitno[i], bits);
        bits  += bitno[i];
    }
}


/**************************************************************************
 *
 *    ROUTINE                :    Post_Process
 *
 *    DESCRIPTION            :    Post-processing of output speech
 *                            Multiplication by two of output speech
 *                            with saturation
 *
 **************************************************************************
 *
 *    USAGE                :    Post_Process(buffer,Length)
 *                            (Routine_Name(arg1,input2))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *    ARG1                :    - Description : Input speech signal buffer
 *                        - Format : int16_t
 *
 *        INPUT2            :    - Description : Length of signal
 *                            - Format : int16_t
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        ARG1                :    - Description : Output speech signal buffer
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void sdecoder::post_process(int16_t * signal, int16_t lg)
{
    for (int16_t i = 0; i < lg; i++)
    {
        signal[i] = op_add(signal[i], signal[i]);
    }
}
