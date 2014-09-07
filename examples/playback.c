/* ----------------------------------------------------------------------- */
/*  This file is part of rld.                                              */
/* ----------------------------------------------------------------------- */
/*  Copyright (c) 2000 stfsux <stfsux@tuxfamily.org>                       */
/*  This work is free. You can redistribute it and/or modify it under the  */
/*  terms of the Do What The Fuck You Want To Public License, Version 2,   */
/*  as published by Sam Hocevar. See the COPYING file for more details.    */
/* ----------------------------------------------------------------------- */

/* crapsynth, you can't do it worst. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <string.h>

#define TRACK_NLINES 64
#define TRACK_NCHANNELS 2
#define TRACK_NPATTERNS 2

unsigned int track[TRACK_NPATTERNS][TRACK_NCHANNELS][TRACK_NLINES] = {
{
    {
        0x20010031 , 0x90040035 , 0x90040038 , 0x90040041 , 0x90040045 , 0x90040041 , 0x90040038 , 0x90040035 , 
        0x90040031 , 0x90040035 , 0x90040038 , 0x90040041 , 0x90040045 , 0x90040041 , 0x90040038 , 0x90040035 , 
        0x90040033 , 0x90040036 , 0x9004003A , 0x90040043 , 0x90040046 , 0x90040043 , 0x9004003A , 0x90040036 , 
        0x90040033 , 0x90040036 , 0x9004003A , 0x90040043 , 0x90040046 , 0x90040043 , 0x9004003A , 0x90040036 , 
        0x90040035 , 0x90040038 , 0x90040041 , 0x90040045 , 0x90040048 , 0x90040045 , 0x90040041 , 0x90040038 , 
        0x90040035 , 0x90040038 , 0x90040041 , 0x90040045 , 0x90040048 , 0x90040045 , 0x90040041 , 0x90040038 , 
        0x90040033 , 0x90040036 , 0x9004003A , 0x90040043 , 0x90040046 , 0x90040043 , 0x9004003A , 0x90040036 , 
        0x90040033 , 0x90040036 , 0x9004003A , 0x90040043 , 0x90040046 , 0x90040043 , 0x9004003A , 0x90040036 , 
        
    },
    {
        0x00000045 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 
        0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 
        0x10040046 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 
        0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 
        0x10040048 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 
        0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 
        0x00000046 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 
        0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x01060000 , 0x02060000 , 0x03060000 , 0x00010045 , 
        
    },

},
{
    {
        0x20010031 , 0x90040035 , 0x90040038 , 0x90040041 , 0x90040045 , 0x90040041 , 0x90040038 , 0x90040035 , 
        0x90040031 , 0x90040035 , 0x90040038 , 0x90040041 , 0x90040045 , 0x90040041 , 0x90040038 , 0x90040035 , 
        0x90040033 , 0x90040036 , 0x9004003A , 0x90040043 , 0x90040046 , 0x90040043 , 0x9004003A , 0x90040036 , 
        0x90040033 , 0x90040036 , 0x9004003A , 0x90040043 , 0x90040046 , 0x90040043 , 0x9004003A , 0x90040036 , 
        0x90040035 , 0x90040038 , 0x90040041 , 0x90040045 , 0x90040048 , 0x90040045 , 0x90040041 , 0x90040038 , 
        0x90040035 , 0x90040038 , 0x90040041 , 0x90040045 , 0x90040048 , 0x90040045 , 0x90040041 , 0x90040038 , 
        0x90040033 , 0x90040036 , 0x9004003A , 0x90040043 , 0x90040046 , 0x90040043 , 0x9004003A , 0x90040036 , 
        0x90040033 , 0x90040036 , 0x9004003A , 0x90040043 , 0x90040046 , 0x90040043 , 0x9004003A , 0x90040036 , 
    },
    {
        0x10010000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x10040043 , 0x10040045 , 
        0x10040046 , 0x00000000 , 0x10040048 , 0x00000000 , 0x10040046 , 0x10040045 , 0x10040043 , 0x10040041 , 
        0x92050046 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 0x92050000 , 
        0x92050048 , 0x00000000 , 0x10040051 , 0x00000000 , 0x2804004C , 0x00000000 , 0x1004004A , 0x00000000 , 
        0x10040048 , 0x92050000 , 0x92050000 , 0x92050000 , 0x00000000 , 0x00000000 , 0x00000000 , 0x00010051 , 
        0x10010000 , 0x00000000 , 0x2004004A , 0x00000000 , 0x10040048 , 0x10040046 , 0x10040045 , 0x00000000 , 
        0x10040046 , 0x92050000 , 0x92040000 , 0x92040000 , 0x92040000 , 0x92040000 , 0x92040000 , 0x92040000 , 
        0x10040048 , 0x91040000 , 0x91040000 , 0x91040000 , 0x10040046 , 0x72040000 , 0x72040000 , 0x72040000 , 
        
    },

},
};

#define TRACKER_SAMPLE_RATE     44100

#define tracker_get_note(x)     (x&0x0F)
#define tracker_get_octave(x)   (((x&0xF0)>>4))
#define tracker_get_sfx(x)      ((x&0xFF000)>>16)
#define tracker_get_args(x)     ((x&0xFF000000)>>24)

const unsigned short freqtable[] = {
0x0030, 0x0033, 0x0036, 0x0039, 0x003d, 0x0040, 0x0044, 0x0048, 0x004d, 0x0051, 0x0056, 0x005b, 
0x0061, 0x0066, 0x006d, 0x0073, 0x007a, 0x0081, 0x0089, 0x0091, 0x009a, 0x00a3, 0x00ad, 0x00b7, 
0x00c2, 0x00cd, 0x00da, 0x00e7, 0x00f4, 0x0103, 0x0112, 0x0123, 0x0134, 0x0146, 0x015a, 0x016e, 
0x0184, 0x019b, 0x01b4, 0x01ce, 0x01e9, 0x0206, 0x0225, 0x0246, 0x0269, 0x028d, 0x02b4, 0x02dd, 
0x0309, 0x0337, 0x0368, 0x039c, 0x03d3, 0x040d, 0x044b, 0x048c, 0x04d2, 0x051b, 0x0569, 0x05bb, 
0x0613, 0x066f, 0x06d1, 0x0739, 0x07a7, 0x081b, 0x0897, 0x0919, 0x09a4, 0x0a37, 0x0ad2, 0x0b77,
};

static unsigned char sintab[32] = {
    0, 24, 49, 74, 97,120,141,161,
    180,197,212,224,235,244,250,253,
    255,253,250,244,235,224,212,197,
    180,161,141,120, 97, 74, 49, 24
};

typedef struct _chn
{
    unsigned short freq;
    unsigned short phase;
    unsigned char vrate;
    unsigned char vdepth;
    unsigned char vpos;
    unsigned char vsign;
    unsigned short portafrq;
    unsigned char portaspeed;
    signed char vol;
    unsigned char sfx;
}chn_t, *pchn_t;

chn_t channels[2];
SDL_AudioSpec format;
 
unsigned int line __attribute__((section(".data")))= 0;
unsigned int pattern __attribute__((section(".data")))= 0;
unsigned int nsamples = 0;
unsigned int speed = 10000;
unsigned int sfxwait = 882;

void play ( void * userdata, uint8_t *stream, int len );

void
 main ( argc, argv )
    int argc;
    char **argv;
{
    format.freq = TRACKER_SAMPLE_RATE;
    format.format = AUDIO_S8;
    format.channels = 1;
    format.samples = 512;
    format.callback = play;
    format.userdata = NULL;

    if ( SDL_OpenAudio ( &format, NULL ) < 0 )
        return;

    SDL_PauseAudio ( 0 );
    SDL_Delay ( 64000 );
    SDL_CloseAudio ();
    return;
}

void
 play ( uerdata, stream, len )
    void *uerdata;
    uint8_t *stream;
    int len;
{
    unsigned int i, j;
    signed short amp = 0;
    for ( i = 0; i < len; i++ )
    {
        amp = 0;
        for ( j = 0; j < TRACK_NCHANNELS; j++ )
        {
            /* Set up channels */
            if ( track[pattern][j][line] )
            {
                if ( tracker_get_note(track[pattern][j][line]) )
                    channels[j].freq = freqtable[tracker_get_note(track[pattern][j][line])+12*tracker_get_octave(track[pattern][j][line])];
                channels[j].sfx = tracker_get_sfx(track[pattern][j][line]);
                switch ( tracker_get_sfx(track[pattern][j][line]) )
                {
                    case 0x01:
                        channels[j].vol = tracker_get_args(track[pattern][j][line]);
                        break;

                    case 0x02:
                    case 0x03:
                        break;

                    case 0x04:
                        channels[j].portaspeed = tracker_get_args(track[pattern][j][line]);
                        break;

                    case 0x05:
                        channels[j].vrate = (tracker_get_args(track[pattern][j][line])&0xF0)>>4;
                        channels[j].vdepth = tracker_get_args(track[pattern][j][line])&0xF;
                        break;

                    case 0x06:
                        break;
                }
            }
        }

        /* Apply effect */
        if ( sfxwait == 0 )
        {
            sfxwait = 882;
            for ( j = 0; j < TRACK_NCHANNELS; j++ )
            {
                switch ( channels[j].sfx )
                {
                    case 0x04:
                        if ( channels[j].portafrq > channels[j].freq )
                        {
                            channels[j].freq += channels[j].portafrq;
                            if ( channels[j].freq > channels[j].portafrq ) channels[j].freq = channels[j].portafrq;
                        }
                        else if ( channels[j].portafrq < channels[j].freq )
                        {
                            channels[j].freq -= channels[j].portafrq;
                            if ( channels[j].freq < channels[j].portafrq ) channels[j].freq = channels[j].portafrq;
                        }
                        break;

                    case 0x05:
                        /* MOD-style */
                        if ( channels[j].vsign == 0 )
                            channels[j].freq += channels[j].vdepth*sintab[channels[j].vpos]>>7;
                        else
                            channels[j].freq -= channels[j].vdepth*sintab[channels[j].vpos]>>7;
                        channels[j].vpos = channels[j].vpos+channels[j].vrate;
                        if ( channels[j].vpos >= 32 )
                        {
                            channels[j].vpos -= 32;
                            channels[j].vsign = channels[j].vsign^1;
                        }
                        break;
                }
            }
        }
        else
            sfxwait--;
        
        /* Mix */
        for ( j = 0; j < TRACK_NCHANNELS; j++ )
        {
            amp += ( channels[j].phase > 0x8000 )?-channels[j].vol:channels[j].vol;
            channels[j].phase += channels[j].freq;
        }

        /* Div by n-channels */
        amp = amp/2;
        
        /* Clamp */
        if ( amp >= 127 )       amp = 127;
        else if ( amp <= -128 ) amp = -128;
        
        stream[i] = amp;

        nsamples++;
        if ( nsamples >= speed )
        {
            for ( j = 0; j < TRACK_NCHANNELS; j++ )
            {
                if ( tracker_get_note(track[pattern][j][line]) )
                    channels[j].portafrq = freqtable[tracker_get_note(track[pattern][j][line])+12*tracker_get_octave(track[pattern][j][line])];
            }
            nsamples = 0;
            line++;
            if ( line >= TRACK_NLINES )
            {
                line = 0;
                pattern++;
                if ( pattern >= TRACK_NPATTERNS )
                    pattern = 0;
            }
        }
    }
    SDL_MixAudio(stream, stream, len, SDL_MIX_MAXVOLUME);
}

