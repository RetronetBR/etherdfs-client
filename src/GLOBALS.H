/*
 * This file is part of the etherdfs project.
 * http://etherdfs.sourceforge.net
 *
 * Copyright (C) 2017 Mateusz Viste
 *
 * Contains all global variables used by etherdfs.
 */

#ifndef GLOBALS_SENTINEL
#define GLOBALS_SENTINEL

/* required size (in bytes) of the data segment - this must be bigh enough as
 * to accomodate all "DATA" segments AND the stack, which will be located at
 * the very end of the data segment. packet drivers tend to require a stack
 * of several hundreds bytes at least - 1K should be safe... It is important
 * that DATASEGSZ can contain a stack of AT LEAST the size of the stack used
 * by the transient code, since the transient part of the program will switch
 * to it and expects the stack to not become corrupted in the process */
#define DATASEGSZ 3500

/* a few globals useful only for debug messages */
#if DEBUGLEVEL > 0
static unsigned short dbg_xpos = 0;
static unsigned short far *dbg_VGA = (unsigned short far *)(0xB8000000l);
static unsigned char dbg_hexc[16] = "0123456789ABCDEF";
#define dbg_startoffset 80*16
#endif

/* whenever the tsrshareddata structure changes, offsets below MUST be
 * adjusted (these are required by assembly routines) */
#define GLOB_DATOFF_PREV2FHANDLERSEG 0
#define GLOB_DATOFF_PREV2FHANDLEROFF 2
#define GLOB_DATOFF_PSPSEG 4
#define GLOB_DATOFF_PKTHANDLE 6
#define GLOB_DATOFF_PKTINT 8
struct tsrshareddata {
/*offs*/
/*  0 */ unsigned short prev_2f_handler_seg; /* seg:off of the previous 2F handler */
/*  2 */ unsigned short prev_2f_handler_off; /* (so I can call it for all queries  */
                                            /* that do not relate to my drive     */
/*  4 */ unsigned short pspseg;    /* segment of the program's PSP block */
/*  6 */ unsigned short pkthandle; /* handler returned by the packet driver */
/*  8 */ unsigned char pktint;     /* software interrupt of the packet driver */

         unsigned char ldrv[26]; /* local to remote drives mappings (0=A:, 1=B, etc */
};

extern struct tsrshareddata glob_data;

/* global variables related to packet driver management and handling frames */
extern unsigned char glob_pktdrv_sndbuff[FRAMESIZE]; /* this not only is my send-frame buffer, but I also use it to store permanently lmac, rmac, ethertype and PROTOVER at proper places */
extern unsigned long glob_pktdrv_pktcall;     /* vector address of the pktdrv interrupt */

/* a few definitions for data that points to my sending buffer */
#define GLOB_LMAC (glob_pktdrv_sndbuff + 6) /* local MAC address */
#define GLOB_RMAC (glob_pktdrv_sndbuff)     /* remote MAC address */

#endif
