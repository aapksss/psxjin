#include "stdafx.h"

#define _IN_OSS

#include "externals.h"

#ifndef _WINDOWS

// Small Linux time helper...only used for watchdog

unsigned long timeGetTime()
{
	struct timeval tv;
	gettimeofday(&tv, 0);                                 // Well, maybe there are better ways
	return tv.tv_sec * 1000 + tv.tv_usec/1000;            // to do that, but at least it works
	
	// We should find said better way
}

// OSS globals

#define OSS_MEM_DEF
#include "oss.h"
static int oss_audio_fd = -1;
extern int errno;

// Setup sound

void SetupSound(void)
{
	int pspeed=44100;
	int pstereo;
	int format;
	int fragsize = 0;
	int myfrag;
	int oss_speed, oss_stereo;

	if (iDisStereo) pstereo=OSS_MODE_MONO;
	else           pstereo=OSS_MODE_STEREO;

	oss_speed = pspeed;
	oss_stereo = pstereo;

	if ((oss_audio_fd=open("/dev/dsp",O_WRONLY,0))==-1)
	{
		printf("Sound device not available\n");
		return;
	}

	if (ioctl(oss_audio_fd,SNDCTL_DSP_RESET,0)==-1)
	{
		printf("Sound reset failed\n");
		return;
	}

// We use 64 fragments with 1024 bytes each

	fragsize=10;
	myfrag=(63<<16)|fragsize;

	if (ioctl(oss_audio_fd,SNDCTL_DSP_SETFRAGMENT,&myfrag)==-1)
	{
		printf("Sound set fragment failed\n");
		return;
	}

	format = AFMT_S16_LE;

	if (ioctl(oss_audio_fd,SNDCTL_DSP_SETFMT,&format) == -1)
	{
		printf("Sound format not supported\n");
		return;
	}

	if (format!=AFMT_S16_LE)
	{
		printf("Sound format not supported\n");
		return;
	}

	if (ioctl(oss_audio_fd,SNDCTL_DSP_STEREO,&oss_stereo)==-1)
	{
		printf("Stereo mode not supported\n");
		return;
	}

	if (oss_stereo!=1)
	{
		iDisStereo=1;
	}

	if (ioctl(oss_audio_fd,SNDCTL_DSP_SPEED,&oss_speed)==-1)
	{
		printf("Sound frequency not supported\n");
		return;
	}

	if (oss_speed!=pspeed)
	{
		printf("Sound frequency not supported\n");
		return;
	}
}

// Remove sound

void RemoveSound(void)
{
	if (oss_audio_fd != -1 )
	{
		close(oss_audio_fd);
		oss_audio_fd = -1;
	}
}

// Get bytes buffered

unsigned long SoundGetBytesBuffered(void)
{
	audio_buf_info info;
	unsigned long l;

	if (oss_audio_fd == -1) return SOUNDSIZE;
	if (ioctl(oss_audio_fd,SNDCTL_DSP_GETOSPACE,&info)==-1)
		l=0;
	else
	{
		if (info.fragments<(info.fragstotal>>1))            // Can we write in at least the half of fragments?
			l=SOUNDSIZE;                                   // No? Wait
		else l=0;                                           // else go on
	}

	return l;
}

// Feed sound data

void SoundFeedStreamData(unsigned char* pSound,long lBytes)
{
	if (oss_audio_fd == -1) return;
	write(oss_audio_fd,pSound,lBytes);
}

#endif
