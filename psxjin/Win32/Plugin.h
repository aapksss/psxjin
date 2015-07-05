// Windows-specific plugin functions
#ifndef __PLUGIN_H__
#define __PLUGIN_H__

typedef long (CALLBACK* NETopen)(HWND);

void PSXjin_MOV_FrameCountSaveState();
void PSXjin_MOV_FrameCountLoadState();

#endif /* __PLUGIN_H__ */
