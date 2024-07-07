#ifdef __cplusplus
extern "C" {
#endif

#ifndef __mp3_shared_h__
#define __mp3_shared_h__

#include "mp3dec.h"

typedef vu32 DSTIME;

typedef enum {
        MP3_IDLE=0,
        MP3_STARTING=1,
        MP3_PLAYING=2,
        MP3_PAUSING=3,
        MP3_RESUMING=4,
        MP3_PAUSED=5,
        MP3_STOPPING=6,
        MP3_ERROR=0xffffffff
} mp3_player_state;

typedef struct {
        vu32    flag;
        vs32    rate;
        vs32    filesize;
        vs32    loop;
        u8      *buffer;
        u16     *audioLeft;
        u16     *audioRight;

	DSTIME soundtime;

} mp3_player;

enum {
        MP3_MSG_START=0,
        MP3_MSG_STOP=1,
        MP3_MSG_PAUSE=2,
        MP3_MSG_RESUME=3,
        MP3_MSG_VOLUME,
        MP3_MSG_ERROR=0xffffffff,
};

typedef struct {
		u32 type;
		union
		{
			volatile mp3_player	*player;
			int	slot;
			u32	volume;
        };
} mp3_msg;


#define OUTBUF_SIZE     (MAX_NCHAN * MAX_NGRAN * MAX_NSAMP)
#define MP3_FILE_BUFFER_SIZE (8 * 1024)

#define MP3_AUDIO_BUFFER_SAMPS (8 * 1024)
#define MP3_AUDIO_BUFFER_SIZE (MP3_AUDIO_BUFFER_SAMPS*2)

#ifdef ARM9

#include <stdio.h>

int mp3_init();
int mp3_play(const char *filename,int loop=0,float loopsec=0);
int mp3_play_file(FILE *file, int loop=0, float loopsec=0);
void mp3_fill_buffer();
int mp3_pause();
int mp3_resume();
int mp3_stop();
int mp3_set_volume(int volume);
bool mp3_is_playing();

#endif // ARM9

#ifdef ARM7

void mp3_init();
void mp3_process();
void AS_StereoDesinterleave(s16 *input, s16 *outputL, s16 *outputR, u32 samples);

#endif // ARM7

#endif // __mp3_shared_h__

#ifdef __cplusplus
}
#endif
