#include "cdecoder.h"

using namespace codec_cdecoder;

/**
 * @brief Constructor
 *
 */

cdecoder::cdecoder()
{

}

/**
 * @brief Destructor
 *
 */

cdecoder::~cdecoder()
{

}

/**
 * @brief Process one frame of 690 * 2 bytes from data to interleaved_coded_array[432]
 *
 * @return 0 if frame is invalid, 1 otherwise
 *
 * NOTES:
 *   - frame must be 1380 bytes length = 690 * int16
 *   - frame must start with "magic word" 0x6b21
 */

int cdecoder::process_frame(const int16_t * data, int16_t * interleaved_coded_array)
{
    if (data[0] != 0x6b21) return 0;                                            // invalid frame, stop processing here

    int cur_pos  = 0;                                                           // current position in coded array
    int data_pos = 1;                                                           // skip 0x6b21 first magic word

    for (int pos = data_pos; pos < data_pos + 114; pos++)
    {
        interleaved_coded_array[cur_pos] = data[pos];
        cur_pos++;
    }

    data_pos += 114;
    data_pos += 1;                                                              // skip 0x6b22 magic word

    for (int pos = data_pos; pos < data_pos + 114; pos++)
    {
        interleaved_coded_array[cur_pos] = data[pos];
        cur_pos++;
    }

    data_pos += 114;
    data_pos += 1;                                                              // skip 0x6b23 magic word

    for (int pos = data_pos; pos < data_pos + 114; pos++)
    {
        interleaved_coded_array[cur_pos] = data[pos];
        cur_pos++;
    }

    data_pos += 114;
    data_pos += 1;                                                              // skip 0x6b24 magic word

    for (int pos = data_pos; pos < data_pos + 90; pos++)                        // no more information after 0x0367, we are at pos 0x0368 = (432 16bit samples + 4 16bit special words) * 2 bytes
    {
        interleaved_coded_array[cur_pos] = data[pos];
        cur_pos++;
    }

    for (int16_t pos = 0; pos < 432; pos++)
    {
        if ((interleaved_coded_array[pos] & 0x0080) == 0x0080)                  // LT FIXME never do this with signed values
        {
            interleaved_coded_array[pos] = interleaved_coded_array[pos] | 0xFF00;
        }

        if ((interleaved_coded_array[pos] > 127) || (interleaved_coded_array[pos] < -127))
        {
            printf("input soft bit out of range\n");
        }

    }

    return 1;
}
/**
 *
 * @brief Speech channel decoding
 *
 * @param[in]  first_pass      true only if first call to this routine
 * @param[in]  frame_stealing  0 = standard mode, 1 = frame stealing mode activated
 * @param[in]  input_frame     frame to be decoded (432 * 2 bytes). In frame stealing mode, only the second half is decoded
 * @param[out] output_frame    two concatenated speech frames (2 * 137 * 2 bytes). If stealing mode, only the second speech frame is valid (FIXME garbage in first speech frame ?)
 * @return                     0 = frame is valid, 1 = bad frame indicator flag
 *
 */

int16_t cdecoder::channel_decoding(short first_pass, int16_t frame_stealing, int16_t * input_frame, int16_t * output_frame)
{
    int16_t decoded_array[286];

    if (first_pass)                                                             // init channel-decoder
    {
        cdecoder::init_rcpc_decoding();
    }

    if (!frame_stealing)                                                        // decoding normal mode
    {
        cdecoder::rcpc_decoding(frame_stealing, input_frame, decoded_array);
    }
    else                                                                        // decoding stealing frame mode
    {
        cdecoder::rcpc_decoding(frame_stealing, input_frame + 216, decoded_array);
    }

    int16_t bad_frame_indicator = cdecoder::bfi(frame_stealing, decoded_array);

    cdecoder::untransform_class_0(frame_stealing, decoded_array);               // "decoding" for non-protected class (class 0)

    if (!frame_stealing)                                                        // reordering of the three classes in two speech frames
    {
        cdecoder::unbuild_sensitivity_classes(frame_stealing, decoded_array, output_frame);
    }
    else                                                                        // reordering of the three classes in one speech frame
    {
        cdecoder::unbuild_sensitivity_classes(frame_stealing, decoded_array, output_frame + 137);
    }

    return bad_frame_indicator;
}

/*
 * Input frame format:
 *
 * Magic words:
 *   0x6b21: 230 - 2 magic word bytes = 228 bytes = 114 * int16
 *   0x6b22
 *   0x6b23
 *   0x6b24
 *   0x6b25
 *   0x6b26
 *
00000000  21 6b 7f 00 7f 00 81 ff  81 ff 7f 00 81 ff 7f 00  |!k..............|
00000010  81 ff 81 ff 81 ff 81 ff  81 ff 81 ff 81 ff 7f 00  |................|
00000020  81 ff 81 ff 7f 00 7f 00  7f 00 7f 00 7f 00 7f 00  |................|
00000030  7f 00 81 ff 7f 00 81 ff  81 ff 7f 00 81 ff 7f 00  |................|
00000040  81 ff 81 ff 81 ff 81 ff  81 ff 81 ff 81 ff 81 ff  |................|
00000050  7f 00 81 ff 7f 00 81 ff  81 ff 7f 00 7f 00 7f 00  |................|
00000060  81 ff 7f 00 7f 00 81 ff  81 ff 7f 00 81 ff 7f 00  |................|
00000070  7f 00 7f 00 7f 00 7f 00  7f 00 7f 00 7f 00 81 ff  |................|
00000080  81 ff 7f 00 7f 00 7f 00  81 ff 81 ff 7f 00 7f 00  |................|
00000090  7f 00 81 ff 81 ff 81 ff  81 ff 7f 00 81 ff 81 ff  |................|
000000a0  81 ff 7f 00 7f 00 7f 00  7f 00 7f 00 81 ff 81 ff  |................|
000000b0  81 ff 81 ff 7f 00 81 ff  7f 00 81 ff 7f 00 7f 00  |................|
000000c0  81 ff 7f 00 7f 00 7f 00  7f 00 7f 00 81 ff 7f 00  |................|
000000d0  81 ff 7f 00 81 ff 81 ff  81 ff 7f 00 7f 00 7f 00  |................|
000000e0  7f 00 81 ff 7f 00 22 6b  7f 00 81 ff 81 ff 7f 00  |......"k........|
000000f0  7f 00 7f 00 81 ff 7f 00  81 ff 7f 00 7f 00 81 ff  |................|
00000100  81 ff 81 ff 7f 00 81 ff  7f 00 7f 00 7f 00 7f 00  |................|
00000110  81 ff 7f 00 81 ff 7f 00  7f 00 7f 00 7f 00 7f 00  |................|
00000120  7f 00 7f 00 81 ff 81 ff  7f 00 7f 00 81 ff 7f 00  |................|
00000130  7f 00 7f 00 7f 00 7f 00  81 ff 7f 00 81 ff 7f 00  |................|
00000140  81 ff 81 ff 7f 00 7f 00  7f 00 7f 00 81 ff 7f 00  |................|
00000150  7f 00 81 ff 81 ff 81 ff  81 ff 81 ff 81 ff 7f 00  |................|
00000160  7f 00 81 ff 81 ff 7f 00  81 ff 7f 00 81 ff 81 ff  |................|
00000170  7f 00 81 ff 81 ff 7f 00  81 ff 81 ff 81 ff 7f 00  |................|
00000180  7f 00 81 ff 7f 00 81 ff  7f 00 81 ff 81 ff 7f 00  |................|
00000190  81 ff 7f 00 7f 00 7f 00  81 ff 81 ff 81 ff 7f 00  |................|
000001a0  81 ff 7f 00 7f 00 7f 00  81 ff 81 ff 7f 00 7f 00  |................|
000001b0  7f 00 7f 00 81 ff 81 ff  7f 00 81 ff 81 ff 7f 00  |................|
000001c0  7f 00 81 ff 81 ff 81 ff  81 ff 7f 00 23 6b 7f 00  |............#k..|
000001d0  7f 00 7f 00 81 ff 81 ff  81 ff 7f 00 7f 00 81 ff  |................|
000001e0  7f 00 7f 00 7f 00 81 ff  81 ff 7f 00 81 ff 81 ff  |................|
000001f0  7f 00 7f 00 81 ff 81 ff  7f 00 7f 00 7f 00 7f 00  |................|
00000200  7f 00 81 ff 81 ff 7f 00  81 ff 7f 00 7f 00 81 ff  |................|
00000210  7f 00 7f 00 7f 00 7f 00  81 ff 81 ff 81 ff 81 ff  |................|
00000220  7f 00 7f 00 7f 00 7f 00  81 ff 7f 00 7f 00 7f 00  |................|
00000230  7f 00 7f 00 7f 00 7f 00  81 ff 7f 00 7f 00 81 ff  |................|
00000240  7f 00 7f 00 81 ff 7f 00  81 ff 7f 00 81 ff 7f 00  |................|
00000250  81 ff 7f 00 7f 00 7f 00  7f 00 7f 00 7f 00 7f 00  |................|
00000260  81 ff 7f 00 81 ff 7f 00  7f 00 7f 00 81 ff 7f 00  |................|
00000270  7f 00 81 ff 81 ff 81 ff  81 ff 7f 00 7f 00 81 ff  |................|
00000280  81 ff 7f 00 7f 00 7f 00  7f 00 81 ff 7f 00 7f 00  |................|
00000290  7f 00 7f 00 81 ff 7f 00  7f 00 7f 00 81 ff 81 ff  |................|
000002a0  7f 00 81 ff 81 ff 7f 00  81 ff 7f 00 81 ff 81 ff  |................|
000002b0  7f 00 24 6b 7f 00 81 ff  7f 00 81 ff 81 ff 7f 00  |..$k............|
000002c0  7f 00 7f 00 7f 00 7f 00  81 ff 81 ff 81 ff 81 ff  |................|
000002d0  81 ff 7f 00 7f 00 7f 00  81 ff 81 ff 7f 00 7f 00  |................|
000002e0  81 ff 7f 00 7f 00 81 ff  7f 00 7f 00 7f 00 7f 00  |................|
000002f0  7f 00 7f 00 7f 00 81 ff  81 ff 7f 00 81 ff 81 ff  |................|
00000300  81 ff 7f 00 7f 00 7f 00  81 ff 81 ff 81 ff 7f 00  |................|
00000310  81 ff 81 ff 7f 00 81 ff  7f 00 7f 00 7f 00 7f 00  |................|
00000320  7f 00 7f 00 81 ff 7f 00  7f 00 7f 00 7f 00 81 ff  |................|
00000330  81 ff 7f 00 81 ff 7f 00  81 ff 7f 00 81 ff 81 ff  |................|
00000340  7f 00 81 ff 81 ff 81 ff  7f 00 7f 00 81 ff 81 ff  |................|
00000350  7f 00 7f 00 81 ff 81 ff  7f 00 81 ff 7f 00 7f 00  |................|
00000360  81 ff 7f 00 7f 00 7f 00  00 00 00 00 00 00 00 00  |................|
00000370  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000380  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000390  00 00 00 00 00 00 00 00  25 6b 00 00 00 00 00 00  |........%k......|
000003a0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000003b0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000003c0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000003d0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000003e0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000003f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000400  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000410  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000420  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000430  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000440  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000450  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000460  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000470  00 00 00 00 00 00 00 00  00 00 00 00 00 00 26 6b  |..............&k|
00000480  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000490  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000004a0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000004b0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000004c0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000004d0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000004e0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000004f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000500  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000510  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000520  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000530  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000540  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000550  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000560  00 00 00 00 21 6b 81 ff  7f 00 81 ff 81 ff 81 ff  |....!k..........|
                      ^ new frame starts here
*/
