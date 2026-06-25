/*
 * Shared EtherDFS client helpers and future transport API.
 */

#ifndef CORE_SENTINEL
#define CORE_SENTINEL

#ifndef FRAMESIZE
#define FRAMESIZE 1090
#endif

struct edfs_transport_ops {
  int (*init)(void);
  int (*send)(unsigned char far *frame, unsigned int len);
  int (*recv)(unsigned char far *frame, unsigned int frame_max, unsigned int *len);
  void (*shutdown)(void);
};

void copybytes(void far *d, void far *s, unsigned int l);
unsigned short mystrlen(void far *s);
int len_if_no_wildcards(char far *s);
unsigned short bsdsum(unsigned char *dataptr, unsigned short l);
void zerobytes(void *obj, unsigned short l);
int hexpair2int(char *hx);
int string2mac(unsigned char *d, char *mac);
void byte2hex(char *s, unsigned char b);

#endif
