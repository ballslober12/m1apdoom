/* M1AP system layer. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include "doomdef.h"
#include "m_misc.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "d_net.h"
#include "g_game.h"

int mb_used = 6;
extern int myargc;
extern char **myargv;
extern int M1AP_InputInit(void);
extern void M1AP_InputShutdown(void);
extern void M1AP_InputTick(void);
extern int M1AP_InputQuit(void);

ticcmd_t emptycmd;

int I_GetHeapSize(void) { return mb_used * 1024 * 1024; }

byte *I_ZoneBase(int *size)
{
    *size = mb_used * 1024 * 1024;
    return (byte *)malloc(*size);
}

int I_GetTime(void)
{
    struct timeval tv;
    static long base_sec = -1;
    gettimeofday(&tv,0);
    if (base_sec < 0) base_sec = tv.tv_sec;
    return (int)((tv.tv_sec-base_sec)*TICRATE + (tv.tv_usec*TICRATE)/1000000);
}

void I_Init(void)
{
    fprintf(stderr,"[DOOM][SYS] M1AP 1-core/32MB profile\n");
    if (!M1AP_InputInit()) I_Error("LGNC initialization failed");
    I_InitSound();
}

void I_StartFrame(void) { }
void I_StartTic(void) { M1AP_InputTick(); }
ticcmd_t *I_BaseTiccmd(void) { return &emptycmd; }

void I_WaitVBL(int count)
{
    if (count > 0) usleep((useconds_t)(count * (1000000 / TICRATE)));
}

void I_BeginRead(void) { }
void I_EndRead(void) { }

byte *I_AllocLow(int length)
{
    byte *p=(byte *)malloc((size_t)length);
    if (p) memset(p,0,(size_t)length);
    return p;
}

void I_Tactile(int on,int off,int total)
{ (void)on; (void)off; (void)total; }

void I_Quit(void)
{
    fprintf(stderr,"[DOOM] shutting down\n");
    M1AP_InputShutdown();
    I_ShutdownSound();
    I_ShutdownMusic();
    I_ShutdownGraphics();
    exit(0);
}

void I_Error(char *error,...)
{
    va_list ap;
    fprintf(stderr,"\n[DOOM][FATAL] ");
    va_start(ap,error);
    vfprintf(stderr,error,ap);
    va_end(ap);
    fprintf(stderr,"\n");
    I_Quit();
}
