/* M1AP single-player network backend. Network games are deliberately absent. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "i_net.h"
#include "d_net.h"
#include "doomstat.h"
#include "i_system.h"

int DOOMPORT = 5030;
int sendsocket = -1;
int insocket = -1;
void (*netget)(void);
void (*netsend)(void);

void I_InitNetwork(void)
{
    doomcom = (doomcom_t *)malloc(sizeof(*doomcom));
    if (!doomcom) I_Error("out of memory for doomcom");
    memset(doomcom,0,sizeof(*doomcom));
    doomcom->id = DOOMCOM_ID;
    doomcom->ticdup = 1;
    doomcom->extratics = 0;
    doomcom->numplayers = 1;
    doomcom->numnodes = 1;
    doomcom->consoleplayer = 0;
    doomcom->deathmatch = false;
    netgame = false;
    fprintf(stderr,"[DOOM][NET] single-player backend\n");
}

void I_NetCmd(void) { doomcom->remotenode = -1; }
