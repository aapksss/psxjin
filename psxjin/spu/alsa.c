#include "stdafx.h"

#define _IN_OSS

#include "externals.h"

#ifndef _WINDOWS

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>

// Small Linux time helper...only used for watchdog

unsigned long timeGetTime()
{
	struct timeval tv;
	gettimeofday(&tv, 0);                                 // Well, maybe there are better ways
	return tv.tv_sec * 1000 + tv.tv_usec/1000;            // to do that, but at least it works
	
	// Check if this is the best way
}

// OSS globals

#define ALSA_MEM_DEF
#include "alsa.h"
static snd_pcm_t *handle = NULL;
static snd_pcm_uframes_t buffer_size;

// Setup sound

void SetupSound(void)
{
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_status_t *status;
	int pspeed;
	int pchannels;
	int format;
	int buffer_time;
	int period_time;
	int err;

	if (iDisStereo) pchannels=1;
	else pchannels=2;

	pspeed=44100;
	format=SND_PCM_FORMAT_S16_LE;
	buffer_time=500000;
	period_time=buffer_time/4;

	if ((err=snd_pcm_open(&handle, "default",
	                      SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK))<0)
	{
		printf("Audio open error: %s\n", snd_strerror(err));
		return;
	}

	if ((err=snd_pcm_nonblock(handle, 0))<0)
	{
		printf("Can't set blocking mode: %s\n", snd_strerror(err));
		return;
	}

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);
	if ((err=snd_pcm_hw_params_any(handle, hwparams))<0)
	{
		printf("Broken configuration for this PCM: %s\n", snd_strerror(err));
		return;
	}

	if ((err=snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED))<0)
	{
		printf("Access type not available: %s\n", snd_strerror(err));
		return;
	}

	if ((err=snd_pcm_hw_params_set_format(handle, hwparams, format))<0)
	{
		printf("Sample format not available: %s\n", snd_strerror(err));
		return;
	}

	if ((err=snd_pcm_hw_params_set_channels(handle, hwparams, pchannels))<0)
	{
		printf("Channels count not available: %s\n", snd_strerror(err));
		return;
	}

	if ((err=snd_pcm_hw_params_set_rate_near(handle, hwparams, &pspeed, 0))<0)
	{
		printf("Rate not available: %s\n", snd_strerror(err));
		return;
	}

	if ((err=snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, 0))<0)
	{
		printf("Buffer time error: %s\n", snd_strerror(err));
		return;
	}

	if ((err=snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, 0))<0)
	{
		printf("Period time error: %s\n", snd_strerror(err));
		return;
	}

	if ((err=snd_pcm_hw_params(handle, hwparams))<0)
	{
		printf("Unable to install hw params: %s\n", snd_strerror(err));
		return;
	}

	snd_pcm_status_alloca(&status);
	if ((err=snd_pcm_status(handle, status))<0)
	{
		printf("Unable to get status: %s\n", snd_strerror(err));
		return;
	}

	buffer_size=snd_pcm_status_get_avail(status);
}

// Remove sound

void RemoveSound(void)
{
	if (handle != NULL)
	{
		snd_pcm_drop(handle);
		snd_pcm_close(handle);
		handle = NULL;
	}
}

// Get bytes buffered

unsigned long SoundGetBytesBuffered(void)
{
	unsigned long l;

	if (handle == NULL)                                // Failed to open?
		return SOUNDSIZE;
	l = snd_pcm_avail_update(handle);
	if (l<0) return 0;
	if (l<buffer_size/2)                                // Can we write in at least the half of fragments?
		l=SOUNDSIZE;                                   // No? Wait.
	else l=0;                                           // else go on

	return l;
}

// Feed sound data

void SoundFeedStreamData(unsigned char* pSound,long lBytes)
{
	if (handle == NULL) return;

	if (snd_pcm_state(handle) == SND_PCM_STATE_XRUN)
		snd_pcm_prepare(handle);
	snd_pcm_writei(handle,pSound,
	               iDisStereo == 1 ? lBytes/2 : lBytes/4);
}

#endif
