/* M1AP DirectFB video backend for Linux Doom 1.10.
 * 320x200 native software renderer -> RGB565 system surface -> hardware StretchBlit.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "doomdef.h"
#include <directfb.h>

static IDirectFB *dfb;
static IDirectFBSurface *primary;
static IDirectFBSurface *framebuffer;
static int screen_w, screen_h;
static unsigned short pal565[256];
static int video_ready;
static int palette_ready;
static unsigned fps_frames;
static int fps_last_tic;

static void die_dfb(const char *where, DFBResult r)
{
    fprintf(stderr, "[DOOM][DFB] %s failed: %s (%d)\n", where, DirectFBErrorString(r), (int)r);
    exit(1);
}
#define DFB_OKAY(x) do { DFBResult _r=(x); if (_r != DFB_OK) die_dfb(#x,_r); } while (0)

void I_InitGraphics(void)
{
    DFBSurfaceDescription sd;
    DFBResult r;
    int argc = myargc;
    char **argv = myargv;

    if (video_ready) return;
    fprintf(stderr, "[DOOM][VIDEO] DirectFB init, renderer 320x200 RGB565\n");
    r = DirectFBInit(&argc, &argv);
    if (r != DFB_OK) die_dfb("DirectFBInit", r);
    DFB_OKAY(DirectFBCreate(&dfb));
    DFB_OKAY(dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN));

    memset(&sd, 0, sizeof(sd));
    sd.flags = DSDESC_CAPS;
    sd.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    DFB_OKAY(dfb->CreateSurface(dfb, &sd, &primary));
    DFB_OKAY(primary->GetSize(primary, &screen_w, &screen_h));
    fprintf(stderr, "[DOOM][VIDEO] display %dx%d\n", screen_w, screen_h);

    memset(&sd, 0, sizeof(sd));
    sd.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT | DSDESC_CAPS;
    sd.width = SCREENWIDTH;
    sd.height = SCREENHEIGHT;
    sd.pixelformat = DSPF_RGB16;
    sd.caps = DSCAPS_SYSTEMONLY;
    DFB_OKAY(dfb->CreateSurface(dfb, &sd, &framebuffer));
    DFB_OKAY(primary->SetBlittingFlags(primary, DSBLIT_NOFX));
    video_ready = 1;
}

void I_ShutdownGraphics(void)
{
    if (framebuffer) { framebuffer->Release(framebuffer); framebuffer = NULL; }
    if (primary) { primary->Release(primary); primary = NULL; }
    if (dfb) { dfb->Release(dfb); dfb = NULL; }
    video_ready = 0;
}

/* Translate the 8-bit Doom framebuffer into RGB565. This table lookup is
 * deliberately branchless and avoids 64k palette calculations per frame. */
void I_FinishUpdate(void)
{
    void *dst;
    int pitch;
    int y;
    const byte *src = screens[0];
    unsigned short *out;
    DFBRectangle dest;

    if (!video_ready) return;
    if (!palette_ready) {
        /* Safe grayscale fallback until PLAYPAL arrives. */
        int i;
        for (i=0;i<256;i++) {
            unsigned v=(unsigned)i >> 3;
            pal565[i]=(unsigned short)((v<<11)|(v<<5)|v);
        }
        palette_ready=1;
    }

    DFB_OKAY(framebuffer->Lock(framebuffer, DSLF_WRITE, &dst, &pitch));
    out = (unsigned short *)dst;
    for (y=0; y<SCREENHEIGHT; y++) {
        const byte *s = src + y*SCREENWIDTH;
        unsigned short *d = (unsigned short *)((unsigned char *)out + y*pitch);
        int x;
        for (x=0; x<SCREENWIDTH; x++) d[x] = pal565[s[x]];
    }
    framebuffer->Unlock(framebuffer);

    dest.x=0; dest.y=0; dest.w=screen_w; dest.h=screen_h;
    DFB_OKAY(primary->StretchBlit(primary, framebuffer, NULL, &dest));
    /* Don't force a software wait here. The TV/DirectFB driver controls the
     * actual scanout; this keeps CPU time available to Doom's renderer. */
    DFB_OKAY(primary->Flip(primary, NULL, 0));

    /* One cheap once-per-second diagnostic; no per-frame terminal spam. */
    fps_frames++;
    {
        int now = I_GetTime();
        if (fps_last_tic == 0) fps_last_tic = now;
        if (now - fps_last_tic >= TICRATE) {
            fprintf(stderr, "[DOOM][FPS] display frames in last second: %u\n", fps_frames);
            fps_frames = 0;
            fps_last_tic = now;
        }
    }
}

void I_UpdateNoBlit(void) { }

void I_ReadScreen(byte *scr)
{
    memcpy(scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}

void I_SetPalette(byte *palette)
{
    int i;
    for (i=0; i<256; i++) {
        unsigned r = gammatable[usegamma][palette[i*3+0]];
        unsigned g = gammatable[usegamma][palette[i*3+1]];
        unsigned b = gammatable[usegamma][palette[i*3+2]];
        pal565[i] = (unsigned short)(((r & 0xf8) << 8) |
                                     ((g & 0xfc) << 3) |
                                     (b >> 3));
    }
    palette_ready=1;
}

void I_StartTic(void);
