#ifndef CDECODER_H
#define CDECODER_H
#include <stdio.h>
#include <stdlib.h>

namespace codec_cdecoder {

    class cdecoder {
    public:
        cdecoder();
        ~cdecoder();
    
        int process_frame(const int16_t * data, int16_t * interleaved_coded_array);
     
        int16_t bfi(int16_t fs_flag, int16_t * input_frame);
        int16_t channel_decoding(short first_pass, int16_t frame_stealing, int16_t * input_frame, int16_t * output_frame);
        int16_t combination(int16_t a, int16_t b);
        int16_t desinterleaving_signalling(int16_t * input_frame, int16_t * output_frame);
        int16_t desinterleaving_speech(int16_t * input_frame, int16_t * output_frame);
        void    init_rcpc_decoding(void);
        void    rcpc_decoding(int16_t fs_flag, int16_t * input_frame, int16_t * output_frame);
        int16_t unbuild_sensitivity_classes(int16_t fs_flag, int16_t * input_frame, int16_t * output_frame);
        int16_t untransform_class_0(int16_t fs_flag, int16_t * input_frame);

    private:
        // CONSTANTS FOR CONVOLUTIONAL CODING
        static const int16_t K = 5;                                             // Constraint Length
        static const int16_t DECODING_DELAY = 30;                               // Decoding Delay

        // Polynomial generators : these polynomials should be under the format 1 + ... + X**4 to match the decoding procedure :
        static const int16_t G1 = 0x01F;                                        // First Polynomial
        static const int16_t G2 = 0x01B;                                        // Second Polynomial
        static const int16_t G3 = 0x015;                                        // Third Polynomial

        // FRAME FORMAT :
        // Size of the three sensitivity classes 
        // these sizes correspond to the coding of two speech frames at a time

        static const int16_t N0         = 51;                                   // One Frame : Less sensitive class (class 0)
        static const int16_t N1         = 56;                                   // One Frame : Moderate sensitive (class 1)
        static const int16_t N2         = 30;                                   // One Frame : Most sensitive (class2)
        static const int16_t N0_2       = 102;                                  // Two Frames : Less sensitive class (class 0)
        static const int16_t N1_2       = 112;                                  // Two Frames : Moderate sensitive (class 1)
        static const int16_t N2_2       = 60;                                   // Two Frames : Most sensitive (class 2)
        static const int16_t N1_2_CODED = 168;                                  // Two Frames after coding : size of class 1 after coding
        static const int16_t N2_2_CODED = 162;                                  // Two Frames after coding : size of class 2 after coding including CRC bits, (K-1) bits to empty the encoder
        // N0_2 + N1_2_CODED + N2_2_CODED = size of a coded frame (60 ms  at 7200b/s) = 432 = Long_time_slot

        static const int16_t LENGTH_VOCODER_FRAME = 137;                        // 30 ms speech frame at 4567 b/s
        //static const int  LENGTH_TIME_SLOT     432                                                // 60 ms frame at 7200 b/s

        // PUNCTURING CHARACTERISTICS
        static const int16_t PERIOD_PCT = 8;                                    // Puncturing Period

        // CONSTANTS FOR MATRIX INTERLEAVING
        // LINES * COLUMNS = LENGTH_TIME_SLOT = 432
        static const int16_t LINES   = 24;
        static const int16_t COLUMNS = 18;

        // BAD FRAME INDICATOR : CRC (Cyclic Redundant Check) - Definition of the 8 CRC bits
        static const int16_t SIZE_CRC     = 8;                                  // Number of bits of the CRC (Cyclic Redundant Check)
        static const int16_t FS_SIZE_CRC  = 4;                                  // Number of bits of the CRC (Frame Stealing Active)

        static const int16_t SIZE_TAB_CRC1 = 29;                                // Amount of bits taken into account
        static const int16_t SIZE_TAB_CRC2 = 29;                                // In every of the 8 CRC bits
        static const int16_t SIZE_TAB_CRC3 = 29;
        static const int16_t SIZE_TAB_CRC4 = 30;
        static const int16_t SIZE_TAB_CRC5 = 30;
        static const int16_t SIZE_TAB_CRC6 = 29;
        static const int16_t SIZE_TAB_CRC7 = 29;
        static const int16_t SIZE_TAB_CRC8 = 35;

        // IN CASE OF FRAME STEALING :
        // BAD FRAME INDICATOR : CRC (Cyclic Redundant Check)
        // Definition of the 4 CRC bits

        static const int16_t FS_SIZE_TAB_CRC1 = 16;
        static const int16_t FS_SIZE_TAB_CRC2 = 16;
        static const int16_t FS_SIZE_TAB_CRC3 = 16;
        static const int16_t FS_SIZE_TAB_CRC4 = 16;

        static const int16_t N1_CODED = 84;                                     // Size of Class1 (one speech frame) after coding 8/12
        static const int16_t N2_CODED = 81;                                     // Size of Class2 (one speech frame) after coding 8/17

        // const values
    
        static const int16_t TAB0[N0];
        static const int16_t TAB1[N1];
        static const int16_t TAB2[N2];
        static const int16_t A1[PERIOD_PCT * 3];
        static const int16_t A2[PERIOD_PCT * 3];
        static const int16_t FS_A2[PERIOD_PCT * 3];

        static const int16_t TAB_CRC1[SIZE_TAB_CRC1];
        static const int16_t TAB_CRC2[SIZE_TAB_CRC2];
        static const int16_t TAB_CRC3[SIZE_TAB_CRC3];
        static const int16_t TAB_CRC4[SIZE_TAB_CRC4];
        static const int16_t TAB_CRC5[SIZE_TAB_CRC5];
        static const int16_t TAB_CRC6[SIZE_TAB_CRC6];
        static const int16_t TAB_CRC7[SIZE_TAB_CRC7];
        static const int16_t TAB_CRC8[SIZE_TAB_CRC8];
    
        static const int16_t FS_TAB_CRC1[FS_SIZE_TAB_CRC1];
        static const int16_t FS_TAB_CRC2[FS_SIZE_TAB_CRC2];
        static const int16_t FS_TAB_CRC3[FS_SIZE_TAB_CRC3];
        static const int16_t FS_TAB_CRC4[FS_SIZE_TAB_CRC4];

        int16_t Previous[(1 << (K - 1))][2];
        int16_t Best_previous[(1 << (K - 1))][DECODING_DELAY];
        int16_t T1[(1 << (K - 1))][2], T2[(1 << (K - 1))][2], T3[(1 << (K - 1))][2];
        int16_t Score[(1 << (K - 1))];
        int16_t Ex_score[(1 << (K - 1))];
        int16_t Received[3];
        int16_t Msb_bit;
        int16_t M_1;

        // operators
        
        int g_overflow = 0;                                                     // used only in sdecoder tetra_dsp.cc and tetra_codec.cc
        
        static const int32_t MAX_32 = ((int32_t)0x7FFFFFFF);
        static const int32_t MIN_32 = ((int32_t)0x80000000);
        static const int16_t MAX_16 = ((int16_t)0x7FFF);
        static const int16_t MIN_16 = ((int16_t)0x8000);
        static const int BIT_0      = 0;
        static const int BIT_1      = 1;
        static const int MASK       = 1;

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
        void    op_lextract(int32_t L_32, int16_t * hi, int16_t * lo);
        int32_t op_mpy_mix(int16_t hi1, int16_t lo1, int16_t lo2);
        int32_t op_mpy_32(int16_t hi1, int16_t lo1, int16_t hi2, int16_t lo2);

        // Basic functions

        int32_t op_add_sh(int32_t L_var, int16_t var1, int16_t shift);
        int32_t op_add_sh16(int32_t L_var, int16_t var1);
        int16_t op_bin2int(int16_t no_of_bits, int16_t * bitstream);
        void    op_int2bin(int16_t value, int16_t no_of_bits, int16_t * bitstream);
        int32_t op_load_sh(int16_t var1, int16_t shift);
        int32_t op_load_sh16(int16_t var1);
        int32_t op_norm_v(int32_t L_var3, int16_t var1, int16_t * var2);
        int16_t op_store_hi(int32_t L_var1, int16_t var2);
        int32_t op_sub_sh(int32_t L_var, int16_t var1, int16_t shift);
        int32_t op_sub_sh16(int32_t L_var, int16_t var1);
    };
};

#endif // CDECODER_H
