/* LGNC-only input backend. No DirectFB input device and no X11/SDL input. */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "lgnc.h"
#include "d_event.h"
#include "doomdef.h"
#include "m_argv.h"
#include "i_system.h"
extern void D_PostEvent(event_t *ev);

#define PRESS 0
#define RELEASE 1
#define REPEAT 2
#define STALE 0.50

static volatile unsigned char keys[256];
static volatile uint32_t last_seen_ms[256];
static volatile int should_quit;
static unsigned char sent[256];

static uint32_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000u + (unsigned)(ts.tv_nsec / 1000000u));
}

static BOOLEAN key_cb(UINT32 code, LGNC_KEY_COND_T cond, LGNC_ADDITIONAL_INPUT_INFO_T *extra)
{
    (void)extra;
    if (code < 256) {
        if (cond == PRESS || cond == REPEAT) {
            keys[code] = 1;
            last_seen_ms[code] = now_ms();
        } else if (cond == RELEASE) {
            keys[code] = 0;
        }
    }
    return 1;
}

static BOOLEAN mouse_cb(SINT32 x,SINT32 y,UINT32 buttons,LGNC_KEY_COND_T cond,LGNC_ADDITIONAL_INPUT_INFO_T *extra)
{
    (void)x; (void)y; (void)buttons; (void)cond; (void)extra;
    return 1;
}

static LGNC_STATUS_T msg_cb(int type, UINT32 a, UINT8 *b, UINT16 c)
{
    (void)a; (void)b; (void)c;
    if (type == LGNC_MSG_TERMINATE) should_quit = 1;
    if (type == LGNC_MSG_FOCUS_OUT) {
        memset((void *)keys, 0, sizeof(keys));
    }
    return LGNC_HANDLED;
}

static LGNC_SYSTEM_CALLBACKS_T callbacks = { msg_cb, key_cb, mouse_cb, 0 };

int M1AP_InputInit(void)
{
    memset((void *)keys,0,sizeof(keys));
    memset((void *)last_seen_ms,0,sizeof(last_seen_ms));
    memset(sent,0,sizeof(sent));
    should_quit=0;
    fprintf(stderr,"[DOOM][LGNC] initializing remote input\n");
    if (LGNC_SYSTEM_Initialize(myargc,myargv,&callbacks) != 0) {
        fprintf(stderr,"[DOOM][LGNC] initialization failed\n");
        return 0;
    }
    return 1;
}

void M1AP_InputShutdown(void) { LGNC_SYSTEM_Finalize(); }
int M1AP_InputQuit(void) { return should_quit; }

static int held(unsigned code)
{
    if (code >= 256 || !keys[code]) return 0;
    if ((uint32_t)(now_ms() - last_seen_ms[code]) > 500u) {
        keys[code]=0;
        return 0;
    }
    return 1;
}

static void emit_key(unsigned physical, int doomkey)
{
    event_t ev;
    unsigned char h = (unsigned char)held(physical);
    if (h == sent[physical]) return;
    sent[physical] = h;
    ev.type = h ? ev_keydown : ev_keyup;
    ev.data1 = doomkey;
    ev.data2 = ev.data3 = 0;
    D_PostEvent(&ev);
}

void M1AP_InputTick(void)
{
    /* Movement */
    emit_key(KEY_UP,    KEY_UPARROW);
    emit_key(KEY_DOWN,  KEY_DOWNARROW);
    emit_key(KEY_LEFT,  KEY_LEFTARROW);
    emit_key(KEY_RIGHT, KEY_RIGHTARROW);

    /* Actions: OK=fire, Num2=use, Num1/3=strafe, Num4=strafe modifier. */
    emit_key(KEY_OK,    KEY_RCTRL);
    emit_key(KEY_NUM0+2, ' ');
    emit_key(KEY_NUM0+1, ',');
    emit_key(KEY_NUM0+3, '.');
    emit_key(KEY_NUM0+4, KEY_RALT);

    /* Weapon selection: Num5..Num9 -> weapons 1..5, Num0 -> weapon 6. */
    emit_key(KEY_NUM0+5, '1');
    emit_key(KEY_NUM0+6, '2');
    emit_key(KEY_NUM0+7, '3');
    emit_key(KEY_NUM0+8, '4');
    emit_key(KEY_NUM0+9, '5');
    emit_key(KEY_NUM0+0, '6');

    /* Back/Exit behaves like Escape. */
    emit_key(KEY_BACK, KEY_ESCAPE);
    if (should_quit) I_Quit();
}
