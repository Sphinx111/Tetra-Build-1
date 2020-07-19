#include <stdio.h>
#include <stdlib.h>
#include "audio_decoder.h"

//    ./cdecoder out.bit out.cod
//    ./sdecoder out.cod out.raw
//    sox -r 8k -e signed -b 16 out.raw out.wav

/**
 * @brief Constructor
 *
 */

audio_decoder::audio_decoder()
{
    cdec = new codec_cdecoder::cdecoder();
    sdec = new codec_sdecoder::sdecoder();
}

/**
 * @brief Destructor
 *
 */

audio_decoder::~audio_decoder()
{
    delete cdec;
    delete sdec;
}

/**
 * @brief Initialize TETRA decoder
 *
 */

void audio_decoder::init()
{
    g_first_pass_flag = 1;
    bfi1 = 0;
    bfi2 = 0;
    
    sdec->init_decod_tetra();                                                   // (re-)initialize TETRA codec synthesis parameters
}

/**
 * @brief Process TETRA channel frame (contains 2 speech frames)
 *
 *  - input_frame from decoder : 690 * sizeof(int16_t)     = 1380 bytes
 *  - interleaved_coded_array  : 432 * sizeof(int16_t)     =  864 bytes (time-slot length at 7.2 kb/s)
 *  - raw_output               : 240 * 2 * sizeof(int16_t) =  480 * 2 = 960 bytes contains the two speech frames
 *
 * NOTES:
 *  - first frame must be handled by recorder when calling function
 *  - stealing frame must be handled by recorder when calling function
 */

int audio_decoder::process_frame(const int16_t * input_frame, int16_t * raw_output, const int16_t frame_stealing_flag)
{
    // cdecoder part
    
    int16_t interleaved_coded_array[432];                                       // time-slot length at 7.2 kb/s
    int16_t coded_array[432];
    int16_t reordered_array[286];                                               // 2 frames vocoder + 8 + 4

    // TODO Add Frame Stealing Flag handling, for now, frame_stealing_flag is always 0
    // 0 = Inactive,
    // !0 = First Frame in time-slot stolen

    if (!cdec->process_frame(input_frame, interleaved_coded_array))
    {
        printf("invalid frame\n");
        return 0;                                                               // skip this frame
    }
 
    if (frame_stealing_flag)                                                    // FIXME handle frame_stealing_flag in recorder program
    {
        cdec->desinterleaving_signalling(interleaved_coded_array + 216, coded_array + 216);

        // when frame stealing occurs, recopy first half slot
        for (int16_t idx = 0; idx < 216; idx++)
        {
            coded_array[idx] = interleaved_coded_array[idx];
        }
    }
    else
    {
        cdec->desinterleaving_speech(interleaved_coded_array, coded_array);
    }
    bfi1 = frame_stealing_flag;

    // channel decoding
    bfi2 = cdec->channel_decoding(g_first_pass_flag, frame_stealing_flag, coded_array, reordered_array);
    g_first_pass_flag = 0;
    
    if ((frame_stealing_flag == 0) && (bfi2 == 1))
    {
        bfi1 = 1;
    }

    // prepare speech frames, length is 138 * 2 bytes
    int16_t speech_frame1[138] = {0};
    int16_t speech_frame2[138] = {0};

    speech_frame1[0] = bfi1;
    for (int pos = 0; pos < 137; pos++)
    {
        speech_frame1[pos + 1] = reordered_array[pos];
    }

    speech_frame2[0] = bfi2;
    for (int pos = 137; pos < 274; pos++)
    {
        speech_frame2[pos - 136] = reordered_array[pos];
    }

    // sdecoder part

    int16_t data[240] = {0};                                                    // output speech frame length is 240 * 2 bytes

    // process first speech frame
    sdec->process_speech_frame(speech_frame1, data);
    for (int idx = 0; idx < 240; idx++)
    {
        raw_output[idx] = data[idx];
    }

    // process second speech frame
    sdec->process_speech_frame(speech_frame2, data);
    for (int idx = 0; idx < 240; idx++)
    {
        raw_output[idx + 240] = data[idx];
    }

    return 1;
}

/**
 * @brief Process TETRA channel frame (contains 2 speech frames) and output .cod data
 *                to file_out for debugging (./sdecode file.cod file.raw)
 *
 */

int audio_decoder::process_frame_debug(FILE * file_out, const int16_t * input_frame, int16_t * raw_output, const int16_t frame_stealing_flag)
{
    // cdecoder part
    
    int16_t interleaved_coded_array[432];                                       // time-slot length at 7.2 kb/s
    int16_t coded_array[432];
    int16_t reordered_array[286];                                               // 2 frames vocoder + 8 + 4

    if (g_first_pass_flag)                                                      // initialize synthesis parameters only when starting a new record
    {
        sdec->init_decod_tetra();
    }
    
    if (!cdec->process_frame(input_frame, interleaved_coded_array))
    {
        printf("invalid frame\n");
        return 0;                                                               // skip this frame
    }
 
    if (frame_stealing_flag)                                                    // FIXME handle frame_stealing_flag
    {
        cdec->desinterleaving_signalling(interleaved_coded_array + 216, coded_array + 216);

        // when frame stealing occurs, recopy first half slot
        for (int16_t idx = 0; idx < 216; idx++)
        {
            coded_array[idx] = interleaved_coded_array[idx];
        }
    }
    else
    {
        cdec->desinterleaving_speech(interleaved_coded_array, coded_array);
    }
    bfi1 = frame_stealing_flag;

    // channel decoding
    bfi2 = cdec->channel_decoding(g_first_pass_flag, frame_stealing_flag, coded_array, reordered_array);
    g_first_pass_flag = 0;
    
    if ((frame_stealing_flag == 0) && (bfi2 == 1))
    {
        bfi1 = 1;
    }

    // prepare speech frames
    int16_t speech_frame1[138] = {0};
    int16_t speech_frame2[138] = {0};

    int16_t cdec_output[276] = {0};
    int cur_pos = 0;

    speech_frame1[0] = bfi1;
    cdec_output[cur_pos] = bfi1;
    cur_pos++;

    for (int pos = 0; pos < 137; pos++)
    {
        speech_frame1[pos + 1] = reordered_array[pos];
        cdec_output[cur_pos] = reordered_array[pos];
        cur_pos++;
    }

    speech_frame2[0] = bfi2;
    cdec_output[cur_pos] = bfi2;
    cur_pos++;

    for (int pos = 137; pos < 274; pos++)
    {
        speech_frame2[pos - 136] = reordered_array[pos];
        cdec_output[cur_pos] = reordered_array[pos];
        cur_pos++;
    }

    // write .cod file for debugging
    
    if (fwrite(cdec_output, sizeof(int16_t), 276, file_out) != 276)
    {
        printf("unable to write to output file\n");
        return 0;
    }

    // sdecoder part
    
    int16_t data[240];

    // process first speech frame
    sdec->process_speech_frame(speech_frame1, data);
    for (int idx = 0; idx < 240; idx++)
    {
        raw_output[idx] = data[idx];
    }

    // process second speech frame
    sdec->process_speech_frame(speech_frame2, data);
    for (int idx = 0; idx < 240; idx++)
    {
        raw_output[idx + 240] = data[idx];
    }
    
    return 1;
}
