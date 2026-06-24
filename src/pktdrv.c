/*
 * EtherDFS packet driver transport.
 */

#include <i86.h>
#include "clientcore.h"
#include "globals.h"
#include "version.h"
#include "pktdrv.h"

/* set DEBUGLEVEL to 0, 1 or 2 to turn on debug mode with desired verbosity */
#define DEBUGLEVEL 0

/* all the resident code goes to segment 'BEGTEXT' */
#pragma code_seg(BEGTEXT, CODE)

/* this function is called two times by the packet driver. One time for
 * telling that a packet is incoming, and how big it is, so the application
 * can prepare a buffer for it and hand it back to the packet driver. the
 * second call is just to let know that the frame has been copied into the
 * buffer. This is a naked function - I don't need the compiler to get into
 * the way when dealing with packet driver callbacks.
 * IMPORTANT: this function must take care to modify ONLY the registers
 * ES and DI - packet drivers can be easily confused should anything else
 * be modified. */
void __declspec(naked) far pktdrv_recv(void) {
  _asm {
    jmp skip
    SIG db 'p','k','t','r'
    skip:
    push ds
    push bx
    pushf
    mov bx, 0
    mov ds, bx
    cmp ax, 0
    jne secondcall
    cmp cx, FRAMESIZE
    ja nobufferavail
    cmp glob_pktdrv_recvbufflen, 0
    jg nobufferavail
    push ds
    pop es
    mov di, offset glob_pktdrv_recvbuff
    mov glob_pktdrv_recvbufflen, cx
    neg glob_pktdrv_recvbufflen
    jmp restoreandret
  nobufferavail:
    xor bx,bx
    push bx
    push bx
    pop es
    pop di
    jmp restoreandret
  secondcall:
    neg glob_pktdrv_recvbufflen
  restoreandret:
    popf
    pop bx
    pop ds
    retf
  }
}

/* registers a packet driver handle to use on subsequent calls */
int pktdrv_accesstype(void) {
  unsigned char cflag = 0;

  _asm {
    mov ax, 201h
    mov bx, 0ffffh
    mov dl, 0
    mov si, offset glob_pktdrv_sndbuff + 12
    mov cx, 2
    push cs
    pop es
    mov di, offset pktdrv_recv
    mov cflag, 1
    pushf
    cli
    call dword ptr glob_pktdrv_pktcall
    jc badluck
    mov word ptr [glob_data + GLOB_DATOFF_PKTHANDLE], ax
    mov cflag, 0
    badluck:
  }

  if (cflag != 0) return(-1);
  return(0);
}

/* get my own MAC addr. target MUST point to a space of at least 6 chars */
void pktdrv_getaddr(unsigned char *dst) {
  _asm {
    mov ah, 6
    mov bx, word ptr [glob_data + GLOB_DATOFF_PKTHANDLE]
    push ds
    pop es
    mov di, dst
    mov cx, 6
    pushf
    cli
    call dword ptr glob_pktdrv_pktcall
  }
}

int pktdrv_init(unsigned short pktintparam, int nocksum) {
  unsigned short far *intvect = (unsigned short far *)MK_FP(0, pktintparam << 2);
  unsigned short pktdrvfuncoffs = *intvect;
  unsigned short pktdrvfuncseg = *(intvect+1);
  unsigned short rseg = 0, roff = 0;
  char far *pktdrvfunc = (char far *)MK_FP(pktdrvfuncseg, pktdrvfuncoffs);
  int i;
  char sig[8];

  sig[0] = 'P';
  sig[1] = 'K';
  sig[2] = 'T';
  sig[3] = ' ';
  sig[4] = 'D';
  sig[5] = 'R';
  sig[6] = 'V';
  sig[7] = 'R';

  glob_pktdrv_sndbuff[12] = 0xED;
  glob_pktdrv_sndbuff[13] = 0xF5;
  if (nocksum == 0) {
    glob_pktdrv_sndbuff[56] = PROTOVER | 128;
  } else {
    glob_pktdrv_sndbuff[56] = PROTOVER;
  }

  pktdrvfunc += 3;
  for (i = 0; i < 8; i++) if (sig[i] != pktdrvfunc[i]) return(-1);

  glob_data.pktint = pktintparam;

  _asm {
    mov ah, 35h
    mov al, byte ptr [glob_data] + GLOB_DATOFF_PKTINT
    push es
    push bx
    int 21h
    mov rseg, es
    mov roff, bx
    pop bx
    pop es
  }
  glob_pktdrv_pktcall = rseg;
  glob_pktdrv_pktcall <<= 16;
  glob_pktdrv_pktcall |= roff;

  return(pktdrv_accesstype());
}

void pktdrv_free(unsigned long pktcall) {
  pktcall = pktcall;
  _asm {
    mov ah, 3
    mov bx, word ptr [glob_data + GLOB_DATOFF_PKTHANDLE]
    pushf
    cli
    call dword ptr glob_pktdrv_pktcall
  }
}

/* sends query out, as found in glob_pktdrv_sndbuff, and awaits for an answer.
 * this function returns the length of replyptr, or 0xFFFF on error. */
unsigned short sendquery(unsigned char query, unsigned char drive, unsigned short bufflen, unsigned char **replyptr, unsigned short **replyax, unsigned int updatermac) {
  static unsigned char seq;
  unsigned short count;
  unsigned char t;
  unsigned char volatile far *rtc = (unsigned char far *)0x46C;

  drive = glob_data.ldrv[drive];
  bufflen += 60;
  if (bufflen > sizeof(glob_pktdrv_sndbuff)) return(0);
  seq++;
  ((unsigned short *)glob_pktdrv_sndbuff)[26] = bufflen;
  glob_pktdrv_sndbuff[57] = seq;
  glob_pktdrv_sndbuff[58] = drive;
  glob_pktdrv_sndbuff[59] = query;
  if (glob_pktdrv_sndbuff[56] & 128) {
    ((unsigned short *)glob_pktdrv_sndbuff)[27] = bsdsum(glob_pktdrv_sndbuff + 56, bufflen - 56);
  }

  glob_pktdrv_recvbufflen = 0;
  for (count = 5; count != 0; count--) {
    _asm {
      push ax
      push cx
      push dx
      push si
      pushf
      mov ah, 4h
      mov cx, bufflen
      mov si, offset glob_pktdrv_sndbuff
      cli
      call dword ptr glob_pktdrv_pktcall
      pop si
      pop dx
      pop cx
      pop ax
    }

    t = *rtc;
    for (;;) {
      int i;
      if ((t != *rtc) && (t+1 != *rtc) && (*rtc != 0)) break;
      if (glob_pktdrv_recvbufflen < 1) continue;
      if (glob_pktdrv_recvbufflen < 60) goto ignoreframe;
      for (i = 0; i < 6; i++) {
        if (glob_pktdrv_recvbuff[i] != GLOB_LMAC[i]) goto ignoreframe;
        if ((updatermac == 0) && (glob_pktdrv_recvbuff[i+6] != GLOB_RMAC[i])) goto ignoreframe;
      }
      if ((((unsigned short *)glob_pktdrv_recvbuff)[6] != 0xF5EDu) || (glob_pktdrv_recvbuff[57] != seq)) goto ignoreframe;
      if (((unsigned short *)glob_pktdrv_recvbuff)[26] > glob_pktdrv_recvbufflen) goto ignoreframe;
      if (((unsigned short *)glob_pktdrv_recvbuff)[26] < 60) goto ignoreframe;
      glob_pktdrv_recvbufflen = ((unsigned short *)glob_pktdrv_recvbuff)[26];
      if (glob_pktdrv_sndbuff[56] & 128) {
        if (bsdsum(glob_pktdrv_recvbuff + 56, glob_pktdrv_recvbufflen - 56) != (((unsigned short *)glob_pktdrv_recvbuff)[27])) {
          goto ignoreframe;
        }
      }
      *replyptr = glob_pktdrv_recvbuff + 60;
      *replyax = (unsigned short *)(glob_pktdrv_recvbuff + 58);
      if (updatermac != 0) copybytes(GLOB_RMAC, glob_pktdrv_recvbuff + 6, 6);
      return(glob_pktdrv_recvbufflen - 60);
      ignoreframe:
      glob_pktdrv_recvbufflen = 0;
    }
  }
  return(0xFFFFu);
}
