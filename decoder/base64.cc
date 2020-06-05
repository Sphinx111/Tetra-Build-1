/*
 *	base64.c - by Joe DF (joedf@ahkscript.org)
 *	Released under the MIT License
 *
 *	See "base64.h", for more information.
 *
 *	Thank you for inspiration:
 *	http://www.codeproject.com/Tips/813146/Fast-base-functions-for-encode-decode
 *
 *      Modified by Larry 2020-05-13
 */
#include "base64.h"

// Base64 char table - used internally for encoding
static unsigned char b64_chr[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static uint32_t b64_int(uint32_t ch)
{
    // ASCII to base64_int
    // 65-90  Upper Case  >>  0-25
    // 97-122 Lower Case  >>  26-51
    // 48-57  Numbers     >>  52-61
    // 43     Plus (+)    >>  62
    // 47     Slash (/)   >>  63
    // 61     Equal (=)   >>  64~
    if (ch == 43)
	return 62;
    if (ch == 47)
	return 63;
    if (ch == 61)
	return 64;
    if ((ch > 47) && (ch < 58))
	return ch + 4;
    if ((ch > 64) && (ch < 91))
	return ch - 'A';
    if ((ch > 96) && (ch < 123))
	return (ch - 'a') + 26;

    return 0;
}

uint32_t b64e_size(uint32_t in_size)
{
    // size equals 4 * floor((1 / 3) * (in_size + 2));
    uint32_t len = 0;

    for (uint32_t idx = 0; idx < in_size; idx++)
    {
        if (idx % 3 == 0)
        {
            len++;
        }
    }
    return 4 * len;
}

uint32_t b64d_size(uint32_t in_size)
{
    return (3 * in_size) / 4;
}

unsigned int b64_encode(const unsigned char * in, uint32_t in_len, unsigned char * out)
{
    uint32_t data[3];
    uint32_t byt = 0;                                                           // can be 0, 1 or 2 since 3 bytes in encode
    uint32_t pos = 0;                                                           // current output character position

    for (uint32_t idx = 0; idx < in_len; idx++)
    {
        data[byt++] = *(in + idx);

        if (byt == 3)                                                           // process data
        {
            out[pos + 0] = b64_chr[ (data[0] & 0xFF) >> 2 ];
            out[pos + 1] = b64_chr[((data[0] & 0x03) << 4) + ((data[1] & 0xF0) >> 4)];
            out[pos + 2] = b64_chr[((data[1] & 0x0F) << 2) + ((data[2] & 0xC0) >> 6)];
            out[pos + 3] = b64_chr[  data[2] & 0x3F];
            byt  = 0;                                                           // reset byte position
            pos += 4;
        }
    }

    if (byt)                                                                    // there is remaining data to process
    {
        if (byt == 1)
        {
            data[1] = 0;
        }

        out[pos + 0] = b64_chr[ (data[0] & 0xFF) >> 2];
        out[pos + 1] = b64_chr[((data[0] & 0x03) << 4) + ((data[1] & 0xF0) >> 4)];

        if (byt == 2)
        {
            out[pos + 2] = b64_chr[((data[1] & 0x0F) << 2)];
        }
        else                                                                    // add padding
        {
            out[pos + 2] = '=';
        }

        out[pos + 3] = '=';                                                     // padding
        pos += 4;
    }

    out[pos] = '\0';                                                            // final character

    return pos;                                                                 // encoded characters count (including final \0)
}

unsigned int b64_decode(const unsigned char* in, unsigned int in_len, unsigned char* out)
{
    uint32_t byt = 0;                                                           // can be 0, 1, 2 or 3 since 4 bytes in decode
    uint32_t pos = 0;                                                           // current output character position
    uint32_t data[4];

    for (uint32_t idx = 0; idx < in_len; idx++)
    {
        data[byt++] = b64_int(*(in + idx));

        if (byt == 4)
        {
            out[pos + 0] = ((data[0] & 0xFF) << 2) + ((data[1] & 0x30) >> 4);

            if (data[2] != 64)
            {
                out[pos + 1] = ((data[1] & 0x0F) << 4) + ((data[2] & 0x3C) >> 2);

                if (data[3] != 64)
                {
                    out[pos + 2] = ((data[2] & 0x03) << 6) + data[3];
                    pos += 3;
                }
                else
                {
                    pos += 2;
                }
            }
            else
            {
                pos += 1;
            }
            byt = 0;
        }
    }

    return pos;                                                                 // decoded characters count
}

#if 0
unsigned int b64_encodef(char *InFile, char *OutFile)
{

    FILE *pInFile = fopen(InFile,"rb");
    FILE *pOutFile = fopen(OutFile,"wb");

    unsigned int i=0;
    unsigned int j=0;
    unsigned int c=0;
    unsigned int s[4];

    if ((pInFile == NULL) || (pOutFile == NULL) ) {
        if (pInFile!=NULL){fclose(pInFile);}
        if (pOutFile!=NULL){fclose(pOutFile);}
        return 0;
    }

    while(c!=EOF) {
        c=fgetc(pInFile);
        if (c == EOF)
            break;
        s[j++]=c;
        if (j == 3) {
            fputc(b64_chr[ (s[0]&255)>>2 ],pOutFile);
            fputc(b64_chr[ ((s[0]&0x03)<<4)+((s[1]&0xF0)>>4) ],pOutFile);
            fputc(b64_chr[ ((s[1]&0x0F)<<2)+((s[2]&0xC0)>>6) ],pOutFile);
            fputc(b64_chr[ s[2]&0x3F ],pOutFile);
            j=0; i+=4;
        }
    }

    if (j) {
        if (j == 1)
            s[1] = 0;
        fputc(b64_chr[ (s[0]&255)>>2 ],pOutFile);
        fputc(b64_chr[ ((s[0]&0x03)<<4)+((s[1]&0xF0)>>4) ],pOutFile);
        if (j == 2)
            fputc(b64_chr[ ((s[1]&0x0F)<<2) ],pOutFile);
        else
            fputc('=',pOutFile);
        fputc('=',pOutFile);
        i+=4;
    }

    fclose(pInFile);
    fclose(pOutFile);

    return i;
}

unsigned int b64_decodef(char *InFile, char *OutFile)
{

    FILE *pInFile = fopen(InFile,"rb");
    FILE *pOutFile = fopen(OutFile,"wb");

    unsigned int c=0;
    unsigned int j=0;
    unsigned int k=0;
    unsigned int s[4];

    if ((pInFile == NULL) || (pOutFile == NULL) ) {
        if (pInFile!=NULL){fclose(pInFile);}
        if (pOutFile!=NULL){fclose(pOutFile);}
        return 0;
    }

    while(c!=EOF) {
        c=fgetc(pInFile);
        if (c == EOF)
            break;
        s[j++]=b64_int(c);
        if (j == 4) {
            fputc(((s[0]&255)<<2)+((s[1]&0x30)>>4),pOutFile);
            if (s[2]!=64) {
                fputc(((s[1]&0x0F)<<4)+((s[2]&0x3C)>>2),pOutFile);
                if ((s[3]!=64)) {
                    fputc(((s[2]&0x03)<<6)+(s[3]),pOutFile); k+=3;
                } else {
                    k+=2;
                }
            } else {
                k+=1;
            }
            j=0;
        }
    }

    fclose(pInFile);
    fclose(pOutFile);

    return k;
}
#endif
