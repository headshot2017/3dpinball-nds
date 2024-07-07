#include <nds.h>
#include <string.h>
#include <malloc.h>
//#include <nds/fifocommon.h>
//#include <nds/fifomessages.h>
#include "mp3dec.h"
#include "mp3_shared.h"

void enableSound();
int getFreeChannel(void);


MP3FrameInfo mp3FrameInfo;
HMP3Decoder hMP3Decoder = 0;

volatile mp3_player                     *mp3;
int                                     mp3_loop;
int                                     mp3_channelLeft;
int                                     mp3_channelRight;
int                                     mp3_bytesleft;
int                                     mp3_volume;
u8                                      *mp3_readPtr;

volatile mp3_player_state		mp3_state;

DSTIME                          ds_sound_start;
DSTIME                          soundtime;
DSTIME                          paintedtime;

//vs32 mp3_debug = 0;
u16 *outbuf = 0;//[OUTBUF_SIZE/2];


DSTIME ds_time()
{
        u16 time1 = TIMER1_DATA;
        u32 time2 = TIMER2_DATA;

        return (time2 << 16) + time1;
}

void ds_set_timer(int rate) {
        if(rate == 0) {
                TIMER_CR(0) = 0;
                TIMER_CR(1) = 0;
                TIMER_CR(2) = 0;
        } else {
                TIMER_DATA(0) = 0x10000 - (0x1000000 / rate) * 2;
                TIMER_CR(0) = TIMER_ENABLE | TIMER_DIV_1;
                TIMER_DATA(1) = 0;
                TIMER_CR(1) = TIMER_ENABLE | TIMER_CASCADE | TIMER_DIV_1;
                TIMER_DATA(2) = 0;
                TIMER_CR(2) = TIMER_ENABLE | TIMER_CASCADE | TIMER_DIV_1;
        }
}

DSTIME ds_sample_pos() {
        DSTIME v;

        v = (ds_time() - ds_sound_start);

        return v;
}

void S_TransferPaintBuffer(int count)
{
        int     out_idx;
        int     out_mask;
        s16     *p;
//        int     step;
        short   *outL = (short *) mp3->audioLeft;
        short   *outR = (short *) mp3->audioRight;


        p = (s16 *)outbuf;
        out_mask = MP3_AUDIO_BUFFER_SAMPS - 1;
        out_idx = paintedtime & out_mask;
//        step = 3 - 1;
        if(out_idx + count > MP3_AUDIO_BUFFER_SAMPS)
        {
                while (count > 0)
                {
                    outL[out_idx] = *p++;
                    outR[out_idx] = *p++;
                    out_idx = (out_idx + 1) & out_mask;
                    count -= 2;
                }
        }
        else
        {
            outL += out_idx;
            outR += out_idx;
            AS_StereoDesinterleave(p,outL,outR,count/2);
        }
}

void mp3_stop()
{
	if (mp3 == 0) return;
	mp3 = 0;

	ds_set_timer(0);
	SCHANNEL_CR(mp3_channelLeft) = 0;
	SCHANNEL_CR(mp3_channelRight) = 0;
	//free((void *)&SCHANNEL_SOURCE(mp3_channelLeft));
	//free((void *)&SCHANNEL_SOURCE(mp3_channelRight));
	SCHANNEL_SOURCE(mp3_channelLeft) = 0;
	SCHANNEL_SOURCE(mp3_channelRight) = 0;
	mp3_channelLeft = -1;
	mp3_state = MP3_IDLE;
}

int mp3_frame() {
        int offset,err;

        // if mp3 is set to loop indefinitely, don't bother with how many data is left
        if(mp3_loop && mp3_bytesleft < 2*MAINBUF_SIZE)
                mp3_bytesleft += MP3_FILE_BUFFER_SIZE*2;


        /* find start of next MP3 frame - assume EOF if no sync found */
        offset = MP3FindSyncWord((u8 *)(mp3_readPtr), mp3_bytesleft);
        if (offset < 0) {
                //mp3_debug = 4;
				mp3->flag = 4;
				mp3_stop();
                return 1;
        }

        mp3_readPtr += offset;
        mp3_bytesleft -= offset;

        err = MP3Decode(hMP3Decoder, (u8 **)&mp3_readPtr, &mp3_bytesleft, (s16*)outbuf, 0);
        if (err) {
                /* error occurred */
                switch (err) {
                case ERR_MP3_INDATA_UNDERFLOW:
						//iprintf("ERR_MP3_INDATA_UNDERFLOW\n");
                        //outOfData = 1;
                        break;
                case ERR_MP3_MAINDATA_UNDERFLOW:
						//iprintf("ERR_MP3_MAINDATA_UNDERFLOW\n");
                        /* do nothing - next call to decode will provide more mainData */
                        return 1;
                case ERR_MP3_FREE_BITRATE_SYNC:
                default:
						//iprintf("DEFAULT\n");
                        //outOfData = 1;
                        break;
                }
                //mp3_debug = err;
                return 0;
        }
        /* no error */
        MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
        S_TransferPaintBuffer(mp3FrameInfo.outputSamps);
        paintedtime += (mp3FrameInfo.outputSamps>>1);

        return 1;
}

void mp3_frames(DSTIME endtime, u8 firstFrames)
{
	while (paintedtime < endtime)
	{
		if (!firstFrames && ds_sample_pos() > endtime + mp3FrameInfo.samprate - (mp3FrameInfo.samprate>>1))
		{
			// if mp3_fill_buffer() was not called on the ARM9 yet (or some other weird bug), stop here and ask for more data
			mp3_readPtr = 0;
			mp3->flag = 2;
			return;
		}

		mp3_frame();
		if (mp3->flag == 4) break; // mp3 ended, stop here

		// check if we moved onto the 2nd file data buffer, if so move it to the 1st one and request a refill
		if(mp3_readPtr > (mp3->buffer +  MP3_FILE_BUFFER_SIZE + (MP3_FILE_BUFFER_SIZE>>1)))
		{
			mp3_readPtr = mp3_readPtr - MP3_FILE_BUFFER_SIZE;
			memcpy((void *)mp3_readPtr, (void *)(mp3_readPtr + MP3_FILE_BUFFER_SIZE), MP3_FILE_BUFFER_SIZE - (mp3_readPtr-mp3->buffer));
			mp3->flag = 1;
		}
	}
}




int mp3_playing()
{
	if (mp3->flag == 3)
	{
		mp3->flag = 0;
		mp3_readPtr = mp3->buffer;
	}
	else if (!mp3_readPtr)
		return 0;

	DSTIME endtime;

	soundtime = ds_sample_pos();

	// check to make sure that we haven't overshot
	if (paintedtime < soundtime)
		paintedtime = soundtime;

	// mix ahead of current position
	endtime = soundtime + (mp3FrameInfo.samprate>>4);

	mp3_frames(endtime, 0);

	if (mp3) mp3->soundtime = soundtime;

	return 0;
}

void mp3_pause() {
        if(mp3 == 0 || mp3_channelLeft == -1) {
                mp3_state = MP3_IDLE;
                return;
        }
        ds_set_timer(0);
        SCHANNEL_CR(mp3_channelLeft) = 0;
        SCHANNEL_CR(mp3_channelRight) = 0;
        mp3_channelLeft = -1;
        mp3_state = MP3_PAUSED;
}

int mp3_resume() {

        if(mp3 == 0 || mp3_channelLeft != -1) {
                //mp3->debug = 42;
                mp3_state = MP3_IDLE;
                return 1;
        }

        paintedtime = 0;
        memset((void *)mp3->audioLeft,0,MP3_AUDIO_BUFFER_SIZE);
        memset((void *)mp3->audioRight,0,MP3_AUDIO_BUFFER_SIZE);
        mp3_frames(MP3_AUDIO_BUFFER_SAMPS>>1, 1);

        MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
        mp3->rate = mp3FrameInfo.samprate;

        mp3_channelLeft = getFreeChannel();

        //mp3->audio[4] = mp3->audio[5] = 0x7fff;       // force a pop for debugging

        SCHANNEL_SOURCE(mp3_channelLeft) = (u32)mp3->audioLeft;
        SCHANNEL_REPEAT_POINT(mp3_channelLeft) = 0;
        SCHANNEL_LENGTH(mp3_channelLeft) = (MP3_AUDIO_BUFFER_SIZE)>>2;
        SCHANNEL_TIMER(mp3_channelLeft) = 0x10000 - (0x1000000 / mp3->rate);

        // "lock" (silent) this channel, so that next getFreeChannel call gives a different one...
        SCHANNEL_CR(mp3_channelLeft) = SCHANNEL_ENABLE | SOUND_VOL(0) | SOUND_PAN(0) | (SOUND_FORMAT_16BIT) | (SOUND_REPEAT);
        mp3_channelRight = getFreeChannel();
        SCHANNEL_CR(mp3_channelLeft) = 0;

        SCHANNEL_SOURCE(mp3_channelRight) = (u32)mp3->audioRight;
        SCHANNEL_REPEAT_POINT(mp3_channelRight) = 0;
        SCHANNEL_LENGTH(mp3_channelRight) = (MP3_AUDIO_BUFFER_SIZE)>>2;
        SCHANNEL_TIMER(mp3_channelRight) = 0x10000 - (0x1000000 / mp3->rate);

        SCHANNEL_CR(mp3_channelLeft) = SCHANNEL_ENABLE | SOUND_VOL(mp3_volume) | SOUND_PAN(0) | (SOUND_FORMAT_16BIT) | (SOUND_REPEAT);
        SCHANNEL_CR(mp3_channelRight) = SCHANNEL_ENABLE | SOUND_VOL(mp3_volume) | SOUND_PAN(127) | (SOUND_FORMAT_16BIT) | (SOUND_REPEAT);

        ds_set_timer(mp3->rate);
        ds_sound_start = ds_time();

        mp3_state = MP3_PLAYING;
        return 0;
}

int mp3_starting() {
        int cb = OUTBUF_SIZE*sizeof(u16);

        if(hMP3Decoder <= 0) {
				int error;
                if ((hMP3Decoder = MP3InitDecoder(&error)) <= 0 ) {
                        mp3_state = MP3_IDLE;
                        fifoSendValue32(FIFO_USER_01, error);
                        return 0;
                }
        }

        if(outbuf == 0) {
                do {
                        outbuf = (u16 *)malloc(cb);
                        if(outbuf) {
                                break;
                        }
                        cb -= 128;
                } while(cb > 0);
        }

        mp3_bytesleft = mp3->filesize;
        mp3_readPtr = mp3->buffer;
        mp3_loop = mp3->loop;

        mp3_resume();

        fifoSendValue32(FIFO_USER_01, cb);
        return 1;
}

void mp3_resuming() {
        if(mp3 == 0) {
                mp3_state = MP3_IDLE;
                fifoSendValue32(FIFO_USER_01, 0);
                return;
        }

        mp3_resume();
        fifoSendValue32(FIFO_USER_01, 0);
}

void mp3_pausing() {
        if(mp3 == 0) {
                mp3_state = MP3_IDLE;
                fifoSendValue32(FIFO_USER_01, 0);
                return;
        }
        mp3_pause();
        fifoSendValue32(FIFO_USER_01, 0);
}

void mp3_stopping() {
        if(mp3 == 0) {
                mp3_state = MP3_IDLE;
                fifoSendValue32(FIFO_USER_01, 0);
                return;
        }
        mp3_stop();
        fifoSendValue32(FIFO_USER_01, 1);
}

void mp3_process() {
        switch(mp3_state) {
        case MP3_STARTING:
                mp3_starting();
                break;
        case MP3_PLAYING:
                mp3_playing();
                break;
        case MP3_PAUSING:
                mp3_pausing();
                break;
        case MP3_RESUMING:
                mp3_resuming();
                break;
        case MP3_STOPPING:
                mp3_stopping();
                break;
		case MP3_PAUSED:
                if (mp3->flag == 3)
                {
                    mp3->flag = 0;
                    mp3_readPtr = mp3->buffer;
                    mp3_resume();
                }
                break;
        case MP3_IDLE:
        case MP3_ERROR:
                break;
        }
}

void mp3_set_volume(int volume) {

        if(volume < 0)
                volume = 0;
        if(volume > 127)
                volume = 127;

        mp3_volume = volume;

        if(mp3_channelLeft == -1) {
                fifoSendValue32(FIFO_USER_01, 0);
                return;
        }
        SCHANNEL_CR(mp3_channelLeft) = (SCHANNEL_CR(mp3_channelLeft) & ~0xFF)|volume;
        SCHANNEL_CR(mp3_channelRight) = (SCHANNEL_CR(mp3_channelRight) & ~0xFF)|volume;
        fifoSendValue32(FIFO_USER_01, 0);
}

//---------------------------------------------------------------------------------
void mp3_DataHandler(int bytes, void *user_data) {
//---------------------------------------------------------------------------------
	mp3_msg msg;

	fifoGetDatamsg(FIFO_USER_01, bytes, (u8*)&msg);

	switch(msg.type)
	{
		case MP3_MSG_START:
			mp3 = msg.player;
			mp3_state = MP3_STARTING;
			break;

		case MP3_MSG_PAUSE:
			mp3_state = MP3_PAUSING;
			break;

		case MP3_MSG_RESUME:
			mp3_state = MP3_RESUMING;
			break;

		case MP3_MSG_STOP:
			mp3_state = MP3_STOPPING;
			break;

		case MP3_MSG_VOLUME:
			mp3_set_volume(msg.volume);
			break;
	}
}

void mp3_init() {
        mp3_state = MP3_IDLE;
        hMP3Decoder = 0;
        mp3_channelLeft = -1;
        mp3 = 0;
        mp3_bytesleft = 0;
        mp3_volume = 127;

        enableSound();
        fifoSetDatamsgHandler(FIFO_USER_01, mp3_DataHandler, 0);
}
