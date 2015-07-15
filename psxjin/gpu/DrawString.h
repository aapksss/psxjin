// Mode flags

#define DSM_NORMAL		0
#define DSM_TRANSPARENT	1

// Draws a string over buff

void DrawString(char *buff, int lPitch, int bpp,
                int x, int y, int w, int h,
                char *str, int len, int mode);
