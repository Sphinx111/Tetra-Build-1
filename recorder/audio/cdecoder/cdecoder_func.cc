#include "cdecoder.h"

using namespace codec_cdecoder;

// ARRAYS FOR INITIALIZATION OF THE CHANNEL CODING
// Definition of the sensitivity classes : 3 classes

/* Class 0
 *  51 terms : Non protected class
 *      Least sensitive bits
 */

const int16_t cdecoder::TAB0[N0] = {
    35, 36, 37, 38, 39, 40, 41, 42, 43, 47,
    48, 56, 61, 62, 63, 64, 65, 66, 67, 68,
    69, 70, 74, 75, 83, 88, 89, 90, 91, 92,
    93, 94, 95, 96, 97, 101, 102, 110, 115, 116,
    117, 118, 119, 120, 121, 122, 123, 124, 128, 129,
    137};

/* Class 1
 *   56 terms : Protected by Rate 8/12
 *     Ranked with the most important bits at the boundaries,
 *     and least important at the middle
 */

const int16_t cdecoder::TAB1[N1] = {
     58,  85, 112,  54,  81, 108, 135,  50,  77, 104,
    131,  45,  72,  99, 126,  55,  82, 109, 136,   5,
     13,  34,   8,  16,  17,  22,  23,  24,  25,  26,
      6,  14,   7,  15,  60,  87, 114,  46,  73, 100,
    127,  44,  71,  98, 125,  33,  49,  76, 103, 130,
     59,  86, 113,  57,  84, 111};

/* Class 2
 *   30 terms : Protected by Rate 8/18
 *     Ranked from least to most important bits
 */

const int16_t cdecoder::TAB2[N2] = {
    18, 19,  20,  21,                                                           /* LSF3 */
    31, 32,                                                                     /* Pitch0 */
    53, 80, 107, 134,                                                           /* Energie 3 */
    1,   2,   3,   4,                                                           /* LSF1 */
    9,  10,  11,  12,                                                           /* LSF2 */
    27, 28,  29,  30,                                                           /* Pitch0 */
    52, 79, 106, 133,                                                           /* Energie 4 */
    51, 78, 105, 132};                                                          /* Energie 5 */

// PUNCTURING CHARACTERISTICS
// Puncturing Tables for the two protected classes (Class 1 and 2)

// Puncturing Table for Class 1 : Rate 8/12 (2/3)

const int16_t cdecoder::A1[PERIOD_PCT * 3] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

// Puncturing Table for Class 2 : Rate 8/18

const int16_t cdecoder::A2[PERIOD_PCT * 3] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 1, 0, 0, 0};

// In case of Frame Stealing, Puncturing Table for Class 2 : Rate 8/17

const int16_t cdecoder::FS_A2[PERIOD_PCT * 3] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0};

// BAD FRAME INDICATOR : CRC (Cyclic Redundant Check) - Definition of the 8 CRC bits

/* Ranks of the bits in Class2 protected by the First bit of the CRC */
const int16_t cdecoder::TAB_CRC1[SIZE_TAB_CRC1] = {
     1,  5,  8,  9, 13, 15, 16, 17, 19, 21,
    22, 24, 25, 31, 32, 35, 36, 38, 40, 43,
    44, 45, 48, 49, 50, 51, 53, 54, 56};

/* Ranks of the bits in Class 2 protected by the Second bit of the CRC */
const int16_t cdecoder::TAB_CRC2[SIZE_TAB_CRC2] = {
     2,  6,  9, 10, 14, 16, 17, 18, 20, 22,
    23, 25, 26, 32, 33, 36, 37, 39, 41, 44,
    45, 46, 49, 50, 51, 52, 54, 55, 57};

/* Ranks of the bits in Class 2 protected by the Third bit of the CRC */
const int16_t cdecoder::TAB_CRC3[SIZE_TAB_CRC3] = {
     3,  7, 10, 11, 15, 17, 18, 19, 21, 23,
    24, 26, 27, 33, 34, 37, 38, 40, 42, 45,
    46, 47, 50, 51, 52, 53, 55, 56, 58};

/* Ranks of the bits in Class 2 protected by the Fourth bit of the CRC */
const int16_t cdecoder::TAB_CRC4[SIZE_TAB_CRC4] = {
     1,  4,  5,  9, 11, 12, 13, 15, 17, 18,
    20, 21, 27, 28, 31, 32, 34, 36, 39, 40,
    41, 44, 45, 46, 47, 49, 50, 52, 57, 59};

/* Ranks of the bits in Class 2 protected by the Fifth bit of the CRC */
const int16_t cdecoder::TAB_CRC5[SIZE_TAB_CRC5] = {
     2,  5,  6, 10, 12, 13, 14, 16, 18, 19,
    21, 22, 28, 29, 32, 33, 35, 37, 40, 41,
    42, 45, 46, 47, 48, 50, 51, 53, 58, 60};

/* Ranks of the bits in Class 2 protected by the Sixth bit of the CRC */
const int16_t cdecoder::TAB_CRC6[SIZE_TAB_CRC6] = {
     3,  6,  7, 11, 13, 14, 15, 17, 19, 20,
    22, 23, 29, 30, 33, 34, 36, 38, 41, 42,
    43, 46, 47, 48, 49, 51, 52, 54, 59};

/* Ranks of the bits in Class 2 protected by the Seventh bit of the CRC */
const int16_t cdecoder::TAB_CRC7[SIZE_TAB_CRC7] = {
     4,  7,  8, 12, 14, 15, 16, 18, 20, 21,
    23, 24, 30, 31, 34, 35, 37, 39, 42, 43,
    44, 47, 48, 49, 50, 52, 53, 55, 60};

/* Ranks of the bits in Class 2 protected by the Eighth bit of the CRC */
const int16_t cdecoder::TAB_CRC8[SIZE_TAB_CRC8] = {
     1,  2,  3,  4,  8, 13, 14, 16, 19, 20,
    22, 23, 25, 26, 27, 28, 29, 30, 32, 33,
    34, 36, 37, 40, 41, 42, 44, 48, 50, 53,
    56, 57, 58, 59, 60};

/* IN CASE OF FRAME STEALING :
 *  BAD FRAME INDICATOR : CRC (Cyclic Redundant Check)
 *  Definition of the 4 CRC bits
 */

/* Ranks of the bits in Class2 protected by the First bit of the CRC */
const int16_t cdecoder::FS_TAB_CRC1[FS_SIZE_TAB_CRC1] = {
    1, 4, 5, 7, 9, 10, 11, 12, 16, 19,
    20, 22, 24, 25, 26, 27 };

/* Ranks of the bits in Class 2 protected by the Second bit of the CRC */
const int16_t cdecoder::FS_TAB_CRC2[FS_SIZE_TAB_CRC2] = {
    1, 2, 4, 6, 7, 8, 9, 13, 16, 17,
    19, 21, 22, 23, 24, 28};

/* Ranks of the bits in Class 2 protected by the Third bit of the CRC */
const int16_t cdecoder::FS_TAB_CRC3[FS_SIZE_TAB_CRC3] = {
    2, 3, 5, 7, 8, 9, 10, 14, 17, 18,
    20, 22, 23, 24, 25, 29};

/* Ranks of the bits in Class 2 protected by the Fourth bit of the CRC */
const int16_t cdecoder::FS_TAB_CRC4[FS_SIZE_TAB_CRC4] = {
    3, 4, 6, 8, 9, 10, 11, 15, 18, 19,
    21, 23, 24, 25, 26, 30};


/**************************************************************************
 *
 *    ROUTINE                :    Bfi
 *
 *    DESCRIPTION            :    Computation of the Bad Frame Indicator
 *                            (CRC based) of a frame
 *
 **************************************************************************
 *
 *    USAGE                :    Bfi(flag,buffer)
 *                            (Routine_Name(arg1,arg2))
 *
 *    ARGUMENT(S)            :
 *
 *        ARG1                :    - Description :    - (flag = 0) : standard mode
 *                                - (flag  0) : frame stealing activated
 *                        - Format : int16_t
 *
 *    ARG2                :    - Description : One ordered frame after decoding
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    BFI = 0 --> OK
 *                        BFI = 1 --> Bad Frame Flag set
 *
 *    COMMENTS            :    8 Crc bits are located at the end  of the Input_Frame
 *                        CRC computation is carried out on Class 2 only
 *                        as defined by arrays TAB_CRC1
 *                        (length SIZE_TAB_CRC1)
 *                        to TAB_CRC8 (length SIZE_TAB_CRC8)
 *
 **************************************************************************/

int16_t cdecoder::bfi(int16_t stolen_frame_flag, int16_t input_frame[])
{
    int16_t bad_frame_flag = 0;

    if (!stolen_frame_flag)                                                               // not stealing frame
    {
        /* The class protected by the CRC (Class 2) starts at index N0_2 + N1_2 */

        // First Bit of the CRC
        int16_t temp = 0;

        for (int16_t i = 0; i < SIZE_TAB_CRC1; i++)
            temp = op_add(temp, input_frame[N0_2 + N1_2 + TAB_CRC1[i] - 1]);

        // The CRC bit is the LSB of temp

        temp = temp & 1;

        // Comparison of the CRC bit just computed to the one contained in the Input_Frame

        if (op_sub(temp, input_frame[N0_2 + N1_2 + N2_2]) != 0)
            bad_frame_flag = 1;

        // Second Bit of the CRC
        temp = 0;
        for (int16_t i = 0; i < SIZE_TAB_CRC2; i++)
            temp = op_add(temp, input_frame[N0_2 + N1_2 + TAB_CRC2[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0_2 + N1_2 + N2_2 + 1]) != 0)
            bad_frame_flag = 1;

        // Third Bit of the CRC
        temp = 0;
        for (int16_t i = 0; i < SIZE_TAB_CRC3; i++)
            temp = op_add(temp, input_frame[N0_2 + N1_2 + TAB_CRC3[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0_2 + N1_2 + N2_2 + 2]) != 0)
            bad_frame_flag = 1;

        // Fourth Bit of the CRC
        temp = 0;
        for (int16_t i = 0; i < SIZE_TAB_CRC4; i++)
            temp = op_add(temp, input_frame[N0_2 + N1_2 + TAB_CRC4[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0_2 + N1_2 + N2_2 + 3]) != 0)
            bad_frame_flag = 1;

        // Fifth Bit of the CRC
        temp = 0;
        for (int16_t i = 0; i < SIZE_TAB_CRC5; i++)
            temp = op_add(temp, input_frame[N0_2 + N1_2 + TAB_CRC5[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0_2 + N1_2 + N2_2 + 4]) != 0)
            bad_frame_flag = 1;

        // Sixth Bit of the CRC
        temp = 0;
        for (int16_t i = 0; i < SIZE_TAB_CRC6; i++)
            temp = op_add(temp, input_frame[N0_2 + N1_2 + TAB_CRC6[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0_2 + N1_2 + N2_2 + 5]) != 0)
            bad_frame_flag = 1;

        // Seventh Bit of the CRC
        temp = 0;
        for (int16_t i = 0; i < SIZE_TAB_CRC7; i++)
            temp = op_add(temp, input_frame[N0_2 + N1_2 + TAB_CRC7[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0_2 + N1_2 + N2_2 + 6]) != 0)
            bad_frame_flag = 1;

        // Eighth Bit of the CRC
        temp = 0;
        for (int16_t i = 0; i < SIZE_TAB_CRC8; i++)
            temp = op_add(temp, input_frame[N0_2 + N1_2 + TAB_CRC8[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0_2 + N1_2 + N2_2 + 7]) != 0)
            bad_frame_flag = 1;
    }
    else                                                                        // fs_flag
    {
        /* The class protected by the CRC (Class 2) starts at index N0 + N1 */

        // First Bit of the CRC
        int16_t temp = 0;

        for (int16_t i = 0; i < FS_SIZE_TAB_CRC1; i++)
            temp = op_add(temp, input_frame[N0 + N1 + FS_TAB_CRC1[i] - 1]);

        // The CRC bit is the LSB of temp

        temp = temp & 1;

        // Comparison of the CRC bit just computed to the one contained in the Input_Frame

        if (op_sub(temp, input_frame[N0 + N1 + N2]) != 0)
            bad_frame_flag = 1;

        // Second Bit of the CRC
        temp = 0;

        for (int16_t i = 0; i < FS_SIZE_TAB_CRC2; i++)
            temp = op_add(temp, input_frame[N0 + N1 + FS_TAB_CRC2[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0 + N1 + N2 + 1]) != 0)
            bad_frame_flag = 1;

        // Third Bit of the CRC
        temp = 0;
        for (int16_t i = 0; i < FS_SIZE_TAB_CRC3; i++)
            temp = op_add(temp, input_frame[N0 + N1 + FS_TAB_CRC3[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0 + N1 + N2 + 2]) != 0)
            bad_frame_flag = 1;

        // Fourth Bit of the CRC
        temp = 0;
        for (int16_t i = 0; i < FS_SIZE_TAB_CRC4; i++)
            temp = op_add(temp, input_frame[N0 + N1 + FS_TAB_CRC4[i] - 1]);

        temp = temp & 1;

        if (op_sub(temp, input_frame[N0 + N1 + N2 + 3]) != 0)
            bad_frame_flag = 1;
    }

    return bad_frame_flag;
}

/**************************************************************************
 *
 *    ROUTINE                :    Combination
 *
 *    DESCRIPTION            :    Computes the (convolutional) coded bit for a given
 *                            polynomial generator and a given state of the encoder
 *
 **************************************************************************
 *
 *    USAGE                :    Combination(gene,state)
 *                            (Routine_Name(input1,input2))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *        INPUT1            :    - Description : Polynomial generator
 *                            - Format : int16_t
 *
 *        INPUT2            :    - Description : State of the encoder
 *                            - Format : int16_t
 *
 *    RETURNED VALUE        :    1 coded bit
 *
 **************************************************************************/

int16_t cdecoder::combination(int16_t A, int16_t B)
{
    int16_t comb = -1;
    int16_t temp1 = A & B;

    for (int16_t i = 0; i <= (K - 1); i++)
    {
        int16_t temp2 = op_shl( (int16_t)1,i);
        int16_t temp3 = temp1 & temp2;
        if (temp3 != 0)
            comb = op_negate(comb);
    }

    return comb;
}

/**************************************************************************
 *
 *    ROUTINE                :    Desinterleaving_Signalling
 *
 *    DESCRIPTION            :    Signalling Channel-type Desinterleaving
 *                        of a single frame (216 bits)
 *
 **************************************************************************
 *
 *    USAGE                :    Desinterleaving_Signalling(buffer_in,buffer_out)
 *                            (Routine_Name(input1,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *        INPUT1            :    - Description : One interleaved frame
 *                            - Format : 216 * 16 bit-samples
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *    OUTPUT1            :    - Description : One matrix desinterleaved frame
 *                            - Format : 216 * 16 bit-samples
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

int16_t cdecoder::desinterleaving_signalling(int16_t * input_frame, int16_t * output_frame)

{
    static const int16_t K3_const = 216;
    static const int16_t K_const  = 216;
    static const int16_t a_const  = 101;

    for (int16_t i = 0; i < K3_const; i++)
    {
        int16_t k = (int16_t)((int32_t)((int32_t)a_const * (int32_t)(i + 1)) % K_const);
        output_frame[i] = input_frame[k];
    }

    return 0;
}


/**************************************************************************
 *
 *    ROUTINE                :    Desinterleaving_Speech
 *
 *    DESCRIPTION            :    Desinterleaving of an interleaved frame (432 bits)
 *
 **************************************************************************
 *
 *    USAGE                :    Desinterleaving_Speech(buffer_in,buffer_out)
 *                            (Routine_Name(input1,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *        INPUT1            :    - Description : One interleaved frame
 *                            - Format : 432 * 16 bit-samples
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *    OUTPUT1            :    - Description : One matrix desinterleaved frame
 *                            - Format : 432 * 16 bit-samples
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

int16_t cdecoder::desinterleaving_speech(int16_t * input_frame, int16_t * output_frame)
{
    for (int16_t col = 0; col < COLUMNS; col++)
    {
        for (int16_t row = 0; row < LINES; row++)
        {
            output_frame[row * COLUMNS + col] = input_frame[col * LINES + row];
        }
    }

    return 0;
}


/**************************************************************************
 *
 *    ROUTINE                :    Init_Rcpc_Decoding
 *
 *    DESCRIPTION            :    Initialization for convolutional rate compatible
 *                            punctured decoding
 *
 **************************************************************************
 *
 *    USAGE                :    Init_Rcpc_Decoding()
 *                            (Routine_Name)
 *
 *    ARGUMENT(S)            :    None
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

void cdecoder::init_rcpc_decoding()
{
    // Number of states in the Viterbi Lattice
    int16_t M = op_shl((int16_t)1, (int16_t)(K - 1));

    // Last State of the Viterbi Lattice

    M_1     = op_sub(M,(int16_t)1);
    Msb_bit = op_shl((int16_t)1, (int16_t)(K - 2));

    int16_t lsb_bits = Msb_bit - 1;

    // Description of the Lattice : Loop on Arrival_State
    for (int16_t arrival_state = 0; arrival_state <= M_1; arrival_state++)      // Loop on the lattice states
    {
        // Computation of the MSB for the Arrival State

        int16_t msb = arrival_state & Msb_bit;

        // Computation of the (K - 1) MSBs for the Starting State

        int16_t msbs_starting_state = arrival_state & lsb_bits;

        // Loop on Lsb, LSB of the Starting State

        for (int16_t lsb = 0; lsb <= 1; lsb++)
        {
            int16_t starting_state = op_add(op_shl(msbs_starting_state, (int16_t)1), lsb);

            Previous[arrival_state][lsb] = starting_state;

            // Ttransition bits T1, T2, T3

            int16_t involved_bits = op_add(op_shl(msb,(int16_t)1), starting_state);

            T1[arrival_state][lsb] = cdecoder::combination(involved_bits, (int16_t)G1);
            T2[arrival_state][lsb] = cdecoder::combination(involved_bits, (int16_t)G2);
            T3[arrival_state][lsb] = cdecoder::combination(involved_bits, (int16_t)G3);
        }
    } // end Loop on arrival_state
}


/**************************************************************************
 *
 *    ROUTINE                :    Rcpc_Decoding
 *
 *    DESCRIPTION            :    Convolutional rate compatible punctured decoding
 *                            of a frame
 *
 **************************************************************************
 *
 *    USAGE                :    Rcpc_Decoding(flag,buffer_in,buffer_out)
 *                            (Routine_Name(input1,input2,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *        INPUT1            :    - Description :    - (flag = 0) : standard mode
 *                                - (flag  0) : frame stealing activated
 *                        - Format : int16_t
 *
 *        INPUT2            :    - Description : Frame to be decoded
 *                            - Format : 432 * 16 bit-samples
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : Decoded frame
 *                            - Format : 286 * 16 bit-samples
 *
 *    RETURNED VALUE        :    None
 *
 *    COMMENTS            :    - Decoding of one frame : [-127 ... +127]
 *                            -127 : 1 almost sure
 *                            0    : 50% probability
 *                            +127 : 0 almost sure
 *                        - For the first bits, classical
 *                          (punctured) Viterbi decoding ie one bit
 *                          is decoded at a time,
 *                        - For the very last bits, block decoding.
 *
 **************************************************************************/

void cdecoder::rcpc_decoding(int16_t stolen_frame_flag, int16_t * input_frame, int16_t * output_frame)
{
    int16_t Size_Class0;
    int16_t Size_Class1;
    int16_t Size_Class2;
    int16_t Size_Coded_Class1;
    int16_t Size_Coded_Class2;
    int16_t Size_Error_Control;

    if (!stolen_frame_flag)
    {
        Size_Class0        = N0_2;
        Size_Class1        = N1_2;
        Size_Class2        = N2_2;
        Size_Coded_Class1  = N1_2_CODED;
        Size_Coded_Class2  = N2_2_CODED;
        Size_Error_Control = SIZE_CRC;
    }
    else
    {
        Size_Class0        = N0;
        Size_Class1        = N1;
        Size_Class2        = N2;
        Size_Coded_Class1  = N1_CODED;
        Size_Coded_Class2  = N2_CODED;
        Size_Error_Control = FS_SIZE_CRC;
    }

    // Recopy of Class 0 (unprotected)
    for (int16_t j = 0; j < Size_Class0; j++)
        output_frame[j] = input_frame[j];

    // Init of Nber_decoded_bits, number of bits decoded
    int16_t Nber_decoded_bits = 0;

    // Init of Step
    int16_t Step          = -1;
    int16_t Step_bis      = -1;
    int16_t Decoder_ready =  0;                                                 // Decoder not ready yet (Step_bis < DECODING_DELAY)

    // Init of the scores (scores of the current Step)
    // Starting State 0 is favoured

    Score[0] = 0;
    for (int16_t j = 1; j <= M_1; j++)
        Score[j] = - 16000;                                                     // M states

    // Decoding of Class 1

    // Init of Index for Puncturing: 0 <= Index_puncturing < PERIOD_PCT
    int16_t Index_puncturing = 0;
    int16_t Chosen_score;
    int16_t Chosen_lsb;
    int16_t Best = 0;                                                           // LT fixed uninitialized state

    for (int16_t index = 0; index < Size_Coded_Class1;)                         // loop on coded class 1
    {
        // loading of array Received with the received data after symetrization
        // if Punctured data Received[data] = 0 else Received[data] = -(2 * data - 1)

        Received[0] = op_negate(op_sub(op_shl(input_frame[Size_Class0 + index], (int16_t)1), (int16_t)1));
        index++;

        if ((A1[Index_puncturing + PERIOD_PCT]) != 0)
        {
            Received[1] = op_negate(op_sub(op_shl(input_frame[Size_Class0 + index], (int16_t)1), (int16_t)1));
            index++;
        }
        else
        {
            Received[1] = 0;
        }


        Received[2] = 0;

        Index_puncturing++;
        if (op_sub(Index_puncturing,(int16_t)PERIOD_PCT) == 0)
            Index_puncturing = 0;

        // Determination of the state of highest score in the current Step

        Step++;
        Step_bis++;

        if (op_sub(Step,(int16_t)DECODING_DELAY) == 0) Step = 0;

        // Computation of the best predecessor of each state and determination of the best state

        int16_t Maximum = -32768;
        for (int16_t j = 0; j <= M_1; j++)
            Ex_score[j] = Score[j];

        // Loop on the states
        for (int16_t arrival_state_sym = ((M_1 + 1) / 2); arrival_state_sym <= M_1; arrival_state_sym++)
        {
            for (int16_t lsb = 0; lsb <= 1; lsb++)                                      // loop on LSB
            {
                // For every state : Estimation of the best predecessor between the one of LSB=0 and the one of LSB=1

                int16_t Essai = Ex_score[Previous[arrival_state_sym][lsb]];

                // Accumulation of the transition due to the received data
                Essai = op_add(Essai, (int16_t)((T1[arrival_state_sym][lsb] > 0) ? Received[0] : op_negate(Received[0])));
                Essai = op_add(Essai, (int16_t)((T2[arrival_state_sym][lsb] > 0) ? Received[1] : op_negate(Received[1])));
                Essai = op_add(Essai, (int16_t)((T3[arrival_state_sym][lsb] > 0) ? Received[2] : op_negate(Received[2])));

                // Search of the best predecessor (predecessor with the highest score)

                if (lsb == 0)
                {
                    Chosen_score = Essai;
                    Chosen_lsb = lsb;
                }
                else
                {
                    if (op_sub(Essai,Chosen_score) > 0)
                    {
                        Chosen_score = Essai;
                        Chosen_lsb = lsb;
                    }
                }
            } // Loop on lsb

            // The score of the current state is the best among the two (lsb=0,1)
            Score[arrival_state_sym] = Chosen_score;
            Best_previous[arrival_state_sym][Step] = Previous[arrival_state_sym][Chosen_lsb];

            // Search of the best arrival state
            if (op_sub(Chosen_score, Maximum) > 0)
            {
                Best    = arrival_state_sym;
                Maximum = Chosen_score;
            }

            int16_t Arrival_state = op_sub(arrival_state_sym, (int16_t)((M_1 + 1) / 2));

            for (int16_t lsb = 0; lsb <= 1; lsb++)
            {
                // For every state : Estimation of the best predecessor between the one of LSB=0 and the one of LSB=1
                int16_t Essai = Ex_score[Previous[Arrival_state][lsb]];

                // Accumulation of the transition due to the received data
                Essai = op_add(Essai,(int16_t)((T1[Arrival_state][lsb] > 0) ? Received[0] : op_negate(Received[0])));
                Essai = op_add(Essai,(int16_t)((T2[Arrival_state][lsb] > 0) ? Received[1] : op_negate(Received[1])));
                Essai = op_add(Essai,(int16_t)((T3[Arrival_state][lsb] > 0) ? Received[2] : op_negate(Received[2])));

                // Search of the best predecessor (predecessor with the highest score)
                if (lsb == 0)
                {
                    Chosen_score = Essai;
                    Chosen_lsb = lsb;
                }
                else
                {
                    if (op_sub(Essai, Chosen_score) > 0)
                    {
                        Chosen_score = Essai;
                        Chosen_lsb = lsb;
                    }
                }
            }

            // The score of the current state is the best among the two (Lsb=0,1)
            Score[Arrival_state] = Chosen_score;
            Best_previous[Arrival_state][Step] = Previous[Arrival_state][Chosen_lsb];

            // Search of the best arrival state
            if(op_sub(Chosen_score, Maximum) > 0)
            {
                Best = Arrival_state;
                Maximum = Chosen_score;
            }
        } // Loop on the states


        // To avoid overflow, substraction of the highest score
        for (int16_t j = 0; j <= M_1; j++)
            Score[j] = op_sub(Score[j],Maximum);

        // Check that the decoder is ready
        if (op_sub(Step,(int16_t)(DECODING_DELAY - 1)) == 0) Decoder_ready = 1;

        if (op_sub(Decoder_ready, (int16_t)1) == 0)
        {
            // Normal procedure : decoding of one bit at a time
            int16_t Pointer = Step;                                             // rank of the bit being decoded : 0 <= Step_bis < Nber_decoded_bits

            for (int16_t j = 1; j <= DECODING_DELAY - 1; j++)
            {
                Best = Best_previous[Best][Pointer];
                Pointer--;
                if (Pointer < 0)
                    Pointer = DECODING_DELAY - 1;
            } // End Decoding of one bit
            int32_t L_temp = op_ldeposit_l(Best);
            L_temp = op_lshl(L_temp, (int16_t)13);
            output_frame[Size_Class0 + Nber_decoded_bits] = op_extract_h(L_temp);
            Nber_decoded_bits++;

            //End Normal Decoding procedure

        } // End if Decoder_ready
    } // End Loop on class 1

    // --- end decoding class 1 ---

    // --- start decoding of Class 2 ---

    // Init of Index for Puncturing: 0 <= Index_puncturing < PERIOD_PCT
    Index_puncturing = 0;

    for (int16_t index = 0; index < Size_Coded_Class2;)                         // Loop on coded class 2
    {
        // Loading of array Received with the received data after symetrization
        // If Punctured data Received[data] = 0 else Received[data] = -(2*data - 1)

        Received[0] = op_negate(op_sub(op_shl(input_frame[Size_Class0 + Size_Coded_Class1 + index], (int16_t)1), (int16_t)1));
        index++;

        Received[1] = op_negate(op_sub(op_shl(input_frame[Size_Class0 + Size_Coded_Class1 + index], (int16_t)1), (int16_t)1));
        index++;

        if (!stolen_frame_flag)
        {
            if ((A2[Index_puncturing + 2 * PERIOD_PCT]) != 0)
            {
                Received[2] = op_negate(op_sub(op_shl(input_frame[Size_Class0 + Size_Coded_Class1 + index], (int16_t)1), (int16_t)1));
                index++;
            }
            else
                Received[2] = 0;
        }
        else
        {
            if ((FS_A2[Index_puncturing + 2 * PERIOD_PCT]) != 0)
            {
                Received[2] = op_negate(op_sub(op_shl(input_frame[Size_Class0 + Size_Coded_Class1 + index], (int16_t)1), (int16_t)1));
                index++;
            }
            else
            {
                Received[2] = 0;
            }
        }

        Index_puncturing++;
        if (op_sub(Index_puncturing, (int16_t)PERIOD_PCT) == 0)
            Index_puncturing = 0;

        // Determination of the state of highest score in the current Step

        Step++;
        Step_bis++;
        if (op_sub(Step,(int16_t)DECODING_DELAY) == 0) Step = 0;

        // Computation of the best predecessor of each state and determination of the best state

        int16_t Maximum = -32768;
        for (int16_t j = 0; j <= M_1; j++)
            Ex_score[j] = Score[j];

        // Loop on the states
        for (int16_t Arrival_state_sym = ((M_1 + 1)/2); Arrival_state_sym <= M_1; Arrival_state_sym++)
        {

            // Arrival_state_sym
            for (int16_t Lsb = 0; Lsb <= 1; Lsb++)                              // Loop on LSB
            {
                // For every state : Estimation of the best predecessor between the one of LSB=0 and the one of LSB=1
                int16_t Essai = Ex_score[Previous[Arrival_state_sym][Lsb]];

                // Accumulation of the transition due to the received data
                Essai = op_add(Essai, (int16_t)((T1[Arrival_state_sym][Lsb] > 0) ? Received[0] : op_negate(Received[0])));
                Essai = op_add(Essai, (int16_t)((T2[Arrival_state_sym][Lsb] > 0) ? Received[1] : op_negate(Received[1])));
                Essai = op_add(Essai, (int16_t)((T3[Arrival_state_sym][Lsb] > 0) ? Received[2] : op_negate(Received[2])));

                // Search of the best predecessor (predecessor with the highest score)
                if (Lsb == 0)
                {
                    Chosen_score = Essai;
                    Chosen_lsb = Lsb;
                }
                else
                {
                    if (op_sub(Essai, Chosen_score) > 0)
                    {
                        Chosen_score = Essai;
                        Chosen_lsb = Lsb;
                    }
                }
            } // Loop on Lsb

            // The score of the current state is the best among the two (Lsb=0,1)
            Score[Arrival_state_sym] = Chosen_score;
            Best_previous[Arrival_state_sym][Step] = Previous[Arrival_state_sym][Chosen_lsb];

            if (op_sub(Chosen_score, Maximum) > 0)
            {
                Best = Arrival_state_sym;
                Maximum = Chosen_score;
            }

            int16_t Arrival_state = op_sub(Arrival_state_sym,(int16_t)((M_1 + 1) / 2));

            for (int16_t Lsb = 0; Lsb <= 1; Lsb++)
            {
                // For every state : Estimation of the best predecessor between the one of LSB=0 and the one of LSB=1
                int16_t Essai = Ex_score[Previous[Arrival_state][Lsb]];

                // Accumulation of the transition due to the received data
                Essai = op_add(Essai, (int16_t)((T1[Arrival_state][Lsb] > 0) ? Received[0] : op_negate(Received[0])));
                Essai = op_add(Essai, (int16_t)((T2[Arrival_state][Lsb] > 0) ? Received[1] : op_negate(Received[1])));
                Essai = op_add(Essai, (int16_t)((T3[Arrival_state][Lsb] > 0) ? Received[2] : op_negate(Received[2])));

                // Search of the best predecessor (predecessor with the highest score)
                if (Lsb == 0)
                {
                    Chosen_score = Essai;
                    Chosen_lsb = Lsb;
                }
                else
                {
                    if (op_sub(Essai, Chosen_score) > 0)
                    {
                        Chosen_score = Essai;
                        Chosen_lsb = Lsb;
                    }
                }

            }

            // The score of the current state is the best among the two (Lsb=0,1)
            Score[Arrival_state] = Chosen_score;
            Best_previous[Arrival_state][Step] = Previous[Arrival_state][Chosen_lsb];

            // Search of the best arrival state
            if (op_sub(Chosen_score, Maximum) > 0)
            {
                Best = Arrival_state;
                Maximum = Chosen_score;
            }
        } // Loop on the states

        // To avoid overflow, substraction of the highest score
        for (int16_t j = 0; j <= M_1; j++)
            Score[j] = op_sub(Score[j], Maximum);

        // Check that the decoder is ready

        // NOTE: For the very last bits of the frame, we make a block decoding :
        //       DECODING_DELAY bits are decoded at a time

        if (op_sub(Step_bis,(int16_t)(Size_Class1 + Size_Class2 + Size_Error_Control + (K - 1) - 1)) == 0)
        {

            // At the end, we use the fact that the last state of the encoder is zero
            Best = 0;
            int16_t Pointer_bis = Step_bis;
            int16_t Pointer = Step;

            // Block decoding
            for (int16_t j = 1; j <= DECODING_DELAY; j++)
            {
                int32_t L_temp = op_ldeposit_l(Best);
                L_temp = op_lshl(L_temp,(int16_t)13);
                output_frame[Size_Class0 + Pointer_bis] = op_extract_h(L_temp);
                Best = Best_previous[Best][Pointer];
                Nber_decoded_bits++;
                Pointer_bis--;
                Pointer--;
                if (Pointer < 0)
                    Pointer = DECODING_DELAY - 1;
            }
            Nber_decoded_bits = op_sub(Nber_decoded_bits,(int16_t)(K - 1));

            return;
        } // End block decoding
        else
        {
            // Normal procedure : decoding of one bit at a time
            int16_t Pointer = Step;

            for (int16_t j = 1; j <= DECODING_DELAY - 1; j++)
            {
                Best = Best_previous[Best][Pointer];
                Pointer--;
                if (Pointer < 0)
                    Pointer = DECODING_DELAY - 1;
            }
            int32_t L_temp = op_ldeposit_l(Best);
            L_temp = op_lshl(L_temp,(int16_t)13);
            output_frame[Size_Class0 + Nber_decoded_bits] = op_extract_h(L_temp);
            Nber_decoded_bits++;
        }
        // End Normal Decoding procedure
        // End if Decoder_ready
    } // End Loop on class 2

    // --- End Decoding class 2 ---
}

/**************************************************************************
 *
 *    ROUTINE                :    Read_Tetra_File
 *
 *    DESCRIPTION            :    Read a file in the TETRA hardware test
 *                            frame format
 *
 **************************************************************************
 *
 *    USAGE                :    Read_Tetra_File(file_pointer,buffer)
 *                            (Routine_Name(input1,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *        INPUT1            :    - Description : File pointer for the TETRA format file
 *                            - Format : FILE
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *        OUTPUT1            :    - Description : buffer containing the TETRA frame
 *                            - Format : 432 * 16 bit-samples
 *
 *    RETURNED VALUE        :    0 if process correct, -1 if EOF
 *
 *    COMMENTS            :    Data read from the TETRA file are short integers
 *
 **************************************************************************/

// short cdecoder::read_tetra_file(FILE * fin, int16_t * array)
// {
//     static short b_first_call = 1;
//     static short block[690] = {0};                                              // LT fixed uninitialized array
//     short * ptr_block;
//     short * start_of_array;

//     /*
//      * Return value: -1  EOF
//      *                0  array filled with TETRA frame
//      */

//     start_of_array = array;

//     // if first call to this routine, then skip past any header
//     if (b_first_call == 1)
//     {
//         while ((*block) != 0x6b21)                                              // magic value
//         {
//             if (fread(block, sizeof(int16_t), 1, fin) != 1)
//             {
//                 return -1;
//             }
//         }

//         b_first_call = 0;

//         if (fread (block + 1, sizeof(int16_t), 689, fin) != 689)
//         {
//             return -1;
//         }

//     }
//     else                                                                        // Read in TETRA frame
//     {
//         if (fread (block, sizeof(int16_t), 690, fin) != 690)
//         {
//             return -1;
//         }
//     }

//     // Copy first valid block

//     ptr_block = block + 1;
//     for (int16_t i = 0; i < 114; i++) *array++ = *ptr_block++;

//     // Copy second valid block

//     ptr_block = block + 161 - 45;
//     for (int16_t i = 0; i < 114; i++) *array++ = *ptr_block++;

//     // Copy third valid block

//     ptr_block = block + 321 - 45 - 45;
//     for (int16_t i = 0; i < 114; i++) *array++ = *ptr_block++;

//     // Copy fourth valid block

//     ptr_block = block + 481 - 45 - 45 - 45;
//     for (int16_t i = 0; i < 90; i++) *array++ = *ptr_block++;

//     array = start_of_array;

//     for (int16_t i = 0; i < 432; i++)
//     {
//         if ((array[i] & 0x0080) == 0x0080)                                      // LT FIXME never do this with signed values
//         {
//             array[i] = array[i] | 0xFF00;
//         }

//         if ((array[i] > 127) || (array[i] < -127))
//         {
//             printf("Input soft bit out of range\n");
//         }

//     }

//     return 0;
// }


/**************************************************************************
 *
 *    ROUTINE                :    Unbuild_Sensitivity_Classes
 *
 *
 *    DESCRIPTION            :    Rebuilds two concatened speech frames from a single
 *                            frame ordered in three sensitivity classes
 *
 **************************************************************************
 *
 *    USAGE                :    Unbuild_Sensitivity_Classes(flag,buffer_in, buffer_out)
 *                            (Routine_Name(input1,input2,output1))
 *
 *    INPUT ARGUMENT(S)        :
 *
 *        INPUT1            :    - Description :    - (flag = 0) : standard mode
 *                                - (flag  0) : frame stealing activated
 *                            - Format : int16_t
 *
 *        INPUT2            :    - Description : One ordered frame
 *                            - Format : 274 * 16 bit-samples
 *
 *    OUTPUT ARGUMENT(S)        :
 *
 *    OUTPUT1            :    - Description : Two concatened speech frames
 *                                      (length of one speech frame
 *                                      given by LENGTH_VOCODER_FRAME)
 *                        - Format : 274 * 16 bit-samples
 *
 *    RETURNED VALUE        :    None
 *
 *    COMMENTS            :    The way reordering is carried out is defined by
 *                        arrays TAB0 (length N0), TAB1 (length N1) and
 *                        TAB2 (length N2)
 *
 ************************************************************************/

int16_t cdecoder::unbuild_sensitivity_classes(int16_t stolen_frame_flag, int16_t * input_frame, int16_t * output_frame)
{
    if (!stolen_frame_flag)
    {
        /* Class 0 : */
        for (int16_t i = 0; i < N0; i++)
        {
            output_frame[TAB0[i] - 1] = input_frame[2 * i];
            output_frame[LENGTH_VOCODER_FRAME + TAB0[i] - 1] = input_frame[2 * i + 1];
        }

        /* Class 1 : */
        for (int16_t i = 0; i < N1; i++)
        {
            output_frame[TAB1[i] - 1] = input_frame[N0_2 + 2 * i];
            output_frame[LENGTH_VOCODER_FRAME + TAB1[i] - 1] = input_frame[N0_2 + 2 * i + 1];
        }


        /* Class 2 : */
        for (int16_t i = 0; i < N2; i++)
        {
            output_frame[TAB2[i] - 1] = input_frame[N0_2 + N1_2 + 2 * i];
            output_frame[LENGTH_VOCODER_FRAME + TAB2[i] - 1] = input_frame[N0_2 + N1_2 + 2 * i + 1];
        }

    }
    else                                                                        // If stolen_frame_flag
    {

        /* Class 0 : */
        for (int16_t i = 0; i < N0; i++)
        {
            output_frame[TAB0[i] - 1] = input_frame[i];
        }

        /* Class 1 : */
        for (int16_t i = 0; i < N1; i++)
        {
            output_frame[TAB1[i] - 1] = input_frame[N0 + i];
        }

        /* Class 2 : */
        for (int16_t i = 0; i < N2; i++)
        {
            output_frame[TAB2[i] - 1] = input_frame[N0 + N1 + i];
        }

    }

    return 0;
}


/**************************************************************************
 *
 *    ROUTINE                :    Untransform_Class_0
 *
 *    DESCRIPTION            :    Transformation ("decoding") of class 0 of the frame
 *                        127 >= x >= 0 --> 0
 *                        0 > x >= -127 --> 1
 *                        the remaining of the frame is not modified
 *
 **************************************************************************
 *
 *    USAGE                :    Untransform_Class_0(flag,buffer)
 *                            (Routine_Name(arg1,arg2))
 *
 *    ARGUMENT(S)            :
 *
 *        ARG1                :    - Description :    - (flag = 0) : standard mode
 *                                - (flag  0) : frame stealing activated
 *                        - Format : int16_t
 *
 *        ARG2                :    - Description : Ordered Frame (sensitivity classes)
 *                                      before or after decoding
 *                                      First bits  = class 0
 *                                    = Unprotected bits
 *                        - Format : 286 * 16 bit-samples
 *
 *    RETURNED VALUE        :    None
 *
 **************************************************************************/

int16_t cdecoder::untransform_class_0(int16_t stolen_frame_flag, int16_t * input_frame)
{
    int16_t Size_Class0;

    if (!stolen_frame_flag)
        Size_Class0 = N0_2;
    else
        Size_Class0 = N0;

    for (int16_t i = 0; i < Size_Class0; i++)
    {
        if (input_frame[i] >= 0)
            input_frame[i] = 0;
        else
            input_frame[i] = 1;
    }

    return 0;
}
