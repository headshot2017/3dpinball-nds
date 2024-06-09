#include "pch.h"
#include "Sound.h"

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

int Sound::num_channels;
bool Sound::enabled_flag = false;
int* Sound::TimeStamps = nullptr;

bool Sound::Init(int channels, bool enableFlag)
{
	soundEnable();
	SetChannels(channels);
	Enable(enableFlag);
	return true;
}

void Sound::Enable(bool enableFlag)
{
	enabled_flag = enableFlag;
}

void Sound::Activate()
{
	
}

void Sound::Deactivate()
{
	
}

void Sound::Close()
{
	delete[] TimeStamps;
	TimeStamps = nullptr;
	soundDisable();
}

void Sound::PlaySound(s16* wavePtr, int time, int size, int samplerate)
{
	if (wavePtr && enabled_flag)
	{
		auto channel = soundPlaySample(wavePtr, SoundFormat_16Bit, size*2, samplerate, 127, 64, false, 0);
		if (channel != -1)
			TimeStamps[channel] = time;
	}
}

s16* Sound::LoadWaveFile(const std::string& lpName)
{
	drwav wavfp;

	if (!drwav_init_file(&wavfp, lpName.c_str(), NULL))
	{
		return 0;
	}

	s16* pSampleData = new s16[(u32)wavfp.totalPCMFrameCount * wavfp.channels];
	if (pSampleData == 0)
	{
		drwav_uninit(&wavfp);
		return 0;
	}
	u32 totalRead = drwav_read_pcm_frames(&wavfp, wavfp.totalPCMFrameCount, pSampleData);
	if (!totalRead)
	{
		drwav_uninit(&wavfp);
		delete[] pSampleData;
		return 0;
	}

	if (wavfp.bitsPerSample == 8) // 8 bit
	{
		s16* _8bitdata = new s16[(u32)wavfp.totalPCMFrameCount * wavfp.channels];
		drwav_u8_to_s16((drwav_int16*)_8bitdata, (drwav_uint8*)pSampleData, wavfp.totalPCMFrameCount);
		delete[] pSampleData;
		pSampleData = _8bitdata;
	}

	return pSampleData;

}

void Sound::FreeSound(s16* wave)
{
	if (wave)
		delete[] wave;
}

void Sound::SetChannels(int channels)
{
	if (channels <= 0)
		channels = 8;

	num_channels = channels;
	delete[] TimeStamps;
	TimeStamps = new int[num_channels]();
}
