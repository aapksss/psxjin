#ifndef _OSS_SOUND_H
#define _OSS_SOUND_H

#ifdef OSS_MEM_DEF
#define OSS_MEM_EXTERN
#else
#define OSS_MEM_EXTERN extern
#endif

OSS_MEM_EXTERN int sound_buffer_size;

#define OSS_MODE_STEREO	1
#define OSS_MODE_MONO		0

#define OSS_SPEED_44100	44100

#endif // _OSS_SOUND_H
