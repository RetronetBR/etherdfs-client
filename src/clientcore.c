/*
 * Shared helpers for the EtherDFS DOS client.
 */

#include "clientcore.h"

/* copies l bytes from *s to *d */
void copybytes(void far *d, void far *s, unsigned int l) {
  while (l != 0) {
    l--;
    *(unsigned char far *)d = *(unsigned char far *)s;
    d = (unsigned char far *)d + 1;
    s = (unsigned char far *)s + 1;
  }
}

unsigned short mystrlen(void far *s) {
  unsigned short res = 0;
  while (*(unsigned char far *)s != 0) {
    res++;
    s = ((unsigned char far *)s) + 1;
  }
  return(res);
}

/* returns -1 if the NULL-terminated s string contains any wildcard (?, *)
 * character. otherwise returns the length of the string. */
int len_if_no_wildcards(char far *s) {
  int r = 0;
  for (;;) {
    switch (*s) {
      case 0: return(r);
      case '?':
      case '*': return(-1);
    }
    r++;
    s++;
  }
}

/* computes a BSD checksum of l bytes at dataptr location */
unsigned short bsdsum(unsigned char *dataptr, unsigned short l) {
  unsigned short cksum = 0;
  _asm {
    cld           /* clear direction flag */
    xor bx, bx    /* bx will hold the result */
    xor ax, ax
    mov cx, l
    mov si, dataptr
    iterate:
    lodsb         /* load a byte from DS:SI into AL and INC SI */
    ror bx, 1
    add bx, ax
    dec cx        /* DEC CX + JNZ could be replaced by a single LOOP */
    jnz iterate   /* instruction, but DEC+JNZ is 3x faster (on 8086) */
    mov cksum, bx
  }
  return(cksum);
}

void zerobytes(void *obj, unsigned short l) {
  unsigned char *o = obj;
  while (l-- != 0) {
    *o = 0;
    o++;
  }
}

/* expects a hex string of exactly two chars "XX" and returns its value, or -1
 * if invalid */
int hexpair2int(char *hx) {
  unsigned char h[2];
  unsigned short i;
  /* translate hx[] to numeric values and validate */
  for (i = 0; i < 2; i++) {
    if ((hx[i] >= 'A') && (hx[i] <= 'F')) {
      h[i] = hx[i] - ('A' - 10);
    } else if ((hx[i] >= 'a') && (hx[i] <= 'f')) {
      h[i] = hx[i] - ('a' - 10);
    } else if ((hx[i] >= '0') && (hx[i] <= '9')) {
      h[i] = hx[i] - '0';
    } else {
      return(-1);
    }
  }
  i = h[0];
  i <<= 4;
  i |= h[1];
  return(i);
}

/* translates an ASCII MAC address into a 6-bytes binary string */
int string2mac(unsigned char *d, char *mac) {
  int i, v;
  /* is it exactly 17 chars long? */
  for (i = 0; mac[i] != 0; i++);
  if (i != 17) return(-1);
  /* are nibble pairs separated by colons? */
  for (i = 2; i < 16; i += 3) if (mac[i] != ':') return(-1);
  /* translate each byte to its numeric value */
  for (i = 0; i < 16; i += 3) {
    v = hexpair2int(mac + i);
    if (v < 0) return(-1);
    *d = v;
    d++;
  }
  return(0);
}

/* translates an unsigned byte into a 2-characters string containing its hex
 * representation. s needs to be at least 3 bytes long. */
void byte2hex(char *s, unsigned char b) {
  char h[16];
  unsigned short i;
  for (i = 0; i < 10; i++) h[i] = '0' + i;
  for (; i < 16; i++) h[i] = ('A' - 10) + i;
  s[0] = h[b >> 4];
  s[1] = h[b & 15];
  s[2] = 0;
}
