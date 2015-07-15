#ifndef _ALSA_SOUND_H
#define _ALSA_SOUND_H

#ifdef ALSA_MEM_DEF
#define ALSA_MEM_EXTERN
#else
#define ALSA_MEM_EXTERN extern
#endif

ALSA_MEM_EXTERN int sound_buffer_size;

#endif // _ALSA_SOUND_H
