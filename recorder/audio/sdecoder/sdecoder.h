#ifndef SDECODER_H
#define SDECODER_H
#include <stdint.h>
#include <stdlib.h>

namespace codec_sdecoder {

    class sdecoder {
    public:
        sdecoder();

        void process_speech_frame(const int16_t * data, int16_t * raw_output);

        // functions in sdecoder file
        void init_decod_tetra();
        void decod_tetra(int16_t parm[], int16_t synth[]);

        // functions in tetra_codec file
        void bits2prm_tetra(int16_t bits[], int16_t prm[]);
        void post_process(int16_t signal[], int16_t lg);

        void codec_pred_lt(int16_t exc[], int16_t T0, int16_t frac, int16_t L_subfr);
        int16_t dec_ener(int16_t index, int16_t bfi, int16_t A[], int16_t prd_lt[], int16_t code[], int16_t L_subfr, int16_t *gain_pit, int16_t *gain_cod);
        void d_d4i60(int16_t index,int16_t sign,int16_t shift, int16_t F[], int16_t cod[]);
        void d_lsp334(int16_t indice[], int16_t lsp[], int16_t old_lsp[]);
        int16_t inter32_m1_3(int16_t x[]);
        int16_t inter32_1_3(int16_t x[]);

    private:
        static const int PARAM_SIZE     = 24;
        int16_t synth_param[PARAM_SIZE] = {0};                                  // synthesis parameters

        static const int16_t DIM_DIC1 =   3;
        static const int16_t DIM_DIC2 =   3;
        static const int16_t DIM_DIC3 =   4;
        static const int16_t LEN_DIC1 = 256;
        static const int16_t LEN_DIC2 = 512;
        static const int16_t LEN_DIC3 = 512;

        static const int16_t ARRAY1_CLSP[LEN_DIC1 * DIM_DIC1];                  // 768 elements
        static const int16_t ARRAY2_CLSP[LEN_DIC2 * DIM_DIC2];                  // 1536 elements
        static const int16_t ARRAY3_CLSP[LEN_DIC3 * DIM_DIC3];                  // 2048 elements
        static const int16_t T_QUA_ENER[64 * 2];

        //--------------------------------------------------------
        //   Decoder constants parameters.
        //
        //   L_frame     : Frame size.
        //   L_subfr     : Sub-frame size.
        //   p           : LPC order.
        //   pp1         : LPC order+1
        //   pit_min     : Minimum pitch lag.
        //   pit_max     : Maximum pitch lag.
        //   L_inter     : Length of filter for interpolation
        //   parm_size   : Lenght of vector parm[]
        //--------------------------------------------------------

        static const int16_t L_FRAME   = 240;
        static const int16_t L_SUBFR   = 60;
        static const int16_t PP0       = 10;
        static const int16_t PP1       = 11;
        static const int16_t PIT_MIN   = 20;
        static const int16_t PIT_MAX   = 143;
        static const int16_t L_INTER   = 15;
        static const int16_t PARM_SIZE = 23;

        //--------------------------------------------------------
        //   LPC bandwidth expansion factors for noise filter.
        //      In Q15 = 0.75, 0.85
        //--------------------------------------------------------

        static const int16_t GAMMA3 = 24576;
        static const int16_t GAMMA4 = 27853;

        // Excitation vector

        int16_t old_exc[L_FRAME + PIT_MAX + L_INTER];
        int16_t * exc;

        // Spectral expansion factors

        int16_t F_gamma3[PP0];
        int16_t F_gamma4[PP0];

        // Lsp (Line spectral pairs in the cosine domain)

        static const int16_t lspold_init[PP0];
        int16_t lspold[PP0];
        int16_t lspnew[PP0];

        // Initial lsp values used after each time a reset is executed

        // Filter's memory

        int16_t mem_syn[PP0];

        // Default parameters

        int16_t old_parm[PARM_SIZE];
        int16_t old_T0;

        int16_t last_ener_cod;
        int16_t last_ener_pit;

        // operators

        int16_t g_overflow = 0;                                                 // used only in sdecoder tetra_dsp.cc and tetra_codec.cc

        static const int32_t MAX_32 = ((int32_t)0x7FFFFFFF);
        static const int32_t MIN_32 = ((int32_t)0x80000000);
        static const int16_t MAX_16 = ((int16_t)0x7FFF);
        static const int16_t MIN_16 = ((int16_t)0x8000);
        static const int16_t BIT_0      = 0;
        static const int16_t BIT_1      = 1;
        static const int16_t MASK       = 1;

        static const int16_t POW2[16];
        int16_t op_sature(int32_t lval1);

        int16_t op_abs_s(int16_t var1);                                         // Short abs,           1
        int16_t op_add(int16_t var1, int16_t var2);                             // Short add,           1
        int16_t op_div_s(int16_t var1, int16_t var2);                           // Short division,     18
        int16_t op_extract_h(int32_t L_var1);                                   // Extract high,        1
        int16_t op_extract_l(int32_t L_var1);                                   // Extract low,         1
        int16_t op_mult(int16_t var1, int16_t var2);                            // Short mult,          1
        int16_t op_mult_r(int16_t var1, int16_t var2);                          // Mult with round,     2
        int16_t op_negate(int16_t var1);                                        // Short negate,        1
        int16_t op_norm_l(int32_t L_var1);                                      // Long norm,          30
        int16_t op_norm_s(int16_t var1);                                        // Short norm,         15
        int16_t op_etsi_round(int32_t L_var1);                                  // Round,               1
        int16_t op_shl(int16_t var1, int16_t var2);                             // Short shift left,    1
        int16_t op_shr(int16_t var1, int16_t var2);                             // Short shift right,   1
        int16_t op_sub(int16_t var1, int16_t var2);                             // Short sub,           1

        int32_t op_labs(int32_t L_var1);                                        // Long abs,            3
        int32_t op_ladd(int32_t L_var1, int32_t L_var2);                        // Long add,            2
        int32_t op_ldeposit_h(int16_t var1);                                    // 16 bit var1 -> MSB   2
        int32_t op_ldeposit_l(int16_t var1);                                    // 16 bit var1 -> LSB,  2
        int32_t op_lmac(int32_t L_var3, int16_t var1, int16_t var2);            // Mac,                 1
        int32_t op_lmac0(int32_t L_var3, int16_t var1, int16_t var2);           // no shift             1
        int32_t op_lmsu(int32_t L_var3, int16_t var1, int16_t var2);            // Msu,                 1
        int32_t op_lmsu0(int32_t L_var3, int16_t var1, int16_t var2);           // no shift             1
        int32_t op_lmult(int16_t var1, int16_t var2);                           // Long mult,           1
        int32_t op_lmult0(int16_t var1, int16_t var2);                          // Long mult no shift,  1
        int32_t op_lnegate(int32_t L_var1);                                     // Long negate,         2
        int32_t op_lshl(int32_t L_var1, int16_t var2);                          // Long shift left,     2
        int32_t op_lshr(int32_t L_var1, int16_t var2);                          // Long shift right,    2
        int32_t op_lshr_r(int32_t L_var1, int16_t var2);                        // L_shr with round,    3
        int32_t op_lsub(int32_t L_var1, int32_t L_var2);                        // Long sub,            2

        // Extended precision functions

        int32_t op_div_32(int32_t L_num, int16_t hi, int16_t lo);
        int32_t op_lcomp(int16_t hi, int16_t lo);
        void    op_lextract(int32_t L_32, int16_t *hi, int16_t *lo);
        int32_t op_mpy_mix(int16_t hi1, int16_t lo1, int16_t lo2);
        int32_t op_mpy_32(int16_t hi1, int16_t lo1, int16_t hi2, int16_t lo2);

        // Basic functions

        int32_t op_add_sh(int32_t L_var, int16_t var1, int16_t shift);
        int32_t op_add_sh16(int32_t L_var, int16_t var1);
        int16_t op_bin2int(int16_t no_of_bits, int16_t *bitstream);
        void    op_int2bin(int16_t value, int16_t no_of_bits, int16_t *bitstream);
        int32_t op_load_sh(int16_t var1, int16_t shift);
        int32_t op_load_sh16(int16_t var1);
        int32_t op_norm_v(int32_t L_var3, int16_t var1, int16_t *var2);
        int16_t op_store_hi(int32_t L_var1, int16_t var2);
        int32_t op_sub_sh(int32_t L_var, int16_t var1, int16_t shift);
        int32_t op_sub_sh16(int32_t L_var, int16_t var1);

        // General signal processing
        static const int16_t GRID_POINTS = 60;
        static const int16_t L_WINDOW = 256;

        // pp = local LPC order, NC = PP/2
        static const int16_t PP = ((int16_t)10);
        static const int16_t NC = ((int16_t)5);

        // Length for local impulse response
        static const int16_t LLG = ((int16_t)60);

        static const int16_t grid[GRID_POINTS + 1];
        static const int16_t LAG_H[10];
        static const int16_t LAG_L[10];
        static const int16_t WIND[L_WINDOW];

        int16_t old_A[PP + 1] = {4096,0,0,0,0,0,0,0,0,0,0};

        void    dsp_autocorr(int16_t x[], int16_t p, int16_t r_h[], int16_t r_l[]);
        void    dsp_az_lsp(int16_t a[], int16_t lsp[], int16_t old_lsp[]);
        void    dsp_back_fil(int16_t x[], int16_t h[], int16_t y[], int16_t L);
        int16_t dsp_chebps(int16_t x, int16_t f[], int16_t n);
        void    dsp_convolve(int16_t x[], int16_t h[], int16_t y[], int16_t L);
        void    dsp_fac_pond(int16_t gamma, int16_t fac[]);
        void    dsp_get_lsp_pol(int16_t *lsp, int32_t *f);
        void    dsp_int_lpc4(int16_t lsp_old[], int16_t lsp_new[], int16_t a_4[]);
        void    dsp_lag_window(int16_t p, int16_t r_h[], int16_t r_l[]);
        void    dsp_levin_32(int16_t Rh[], int16_t Rl[], int16_t A[]);
        int32_t dsp_lpc_gain(int16_t a[]);
        void    dsp_lsp_az(int16_t lsp[], int16_t a[]);
        void    dsp_pond_ai(int16_t a[], int16_t fac[], int16_t a_exp[]);
        void    dsp_residu(int16_t a[], int16_t x[], int16_t y[], int16_t lg);
        void    dsp_syn_filt(int16_t a[], int16_t x[], int16_t y[], int16_t lg, int16_t mem[], int16_t update);

        // Mathematic functions

        static const int16_t tab_inv_sqrt[49];
        static const int16_t tab_log2[33];
        static const int16_t tab_pow2[33];

        int32_t inv_sqrt(int32_t L_x);
        void   Log2(int32_t L_x, int16_t *exponant, int16_t *fraction);
        int32_t pow2(int16_t exponant, int16_t fraction);
    };
}

#endif /* SDECODER_H */
