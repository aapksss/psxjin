void SetupSound(void);
void RemoveSound(void);
unsigned long SoundGetBytesBuffered(void);
void SoundFeedStreamData(unsigned char* pSound,long lBytes);

#ifndef _WINDOWS
unsigned long timeGetTime();
#endif
