#ifndef __I_VIDEO__
#define __I_VIDEO__
#include "doomtype.h"
void I_InitGraphics(void);
void I_ShutdownGraphics(void);
void I_SetPalette(byte *palette);
void I_UpdateNoBlit(void);
void I_FinishUpdate(void);
void I_WaitVBL(int count);
void I_ReadScreen(byte *scr);
void I_BeginRead(void);
void I_EndRead(void);
void I_StartFrame(void);
void I_StartTic(void);
#endif
