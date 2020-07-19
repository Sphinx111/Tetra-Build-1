/*
 *  tetra-kit
 *  Copyright (C) 2020  LarryTh <dev@logami.fr>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H
#include <stdlib.h>
#include <cdecoder.h>
#include <sdecoder.h>

class audio_decoder {
public:
    audio_decoder();
    ~audio_decoder();

    void init();

    int process_frame(const int16_t * input_frame, int16_t * raw_output, const int16_t frame_stealing_flag);                        // input 690 * int16_t, raw output 480 * int16_t
    int process_frame_debug(FILE * file_out, const int16_t * input_frame, int16_t * raw_output, const int16_t frame_stealing_flag); // output .cod to file_out for debugging purpose

    short g_first_pass_flag = 1;
    int16_t bfi1 = 0;                                                           // bad frame indicator: 0 = data is correct, 1 = frame is corrupted
    int16_t bfi2 = 0;

    codec_cdecoder::cdecoder * cdec;                                            // cdecoder wrapper
    codec_sdecoder::sdecoder * sdec;                                            // sdecoder wrapper
};

#endif /* AUDIO_DECODER_H */
