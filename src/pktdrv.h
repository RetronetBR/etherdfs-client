/*
 * EtherDFS packet driver transport.
 */

#ifndef PKTDRV_SENTINEL
#define PKTDRV_SENTINEL

void __declspec(naked) far pktdrv_recv(void);
int pktdrv_accesstype(void);
void pktdrv_getaddr(unsigned char *dst);
int pktdrv_init(unsigned short pktintparam, int nocksum);
void pktdrv_free(unsigned long pktcall);
unsigned short sendquery(unsigned char query, unsigned char drive, unsigned short bufflen, unsigned char **replyptr, unsigned short **replyax, unsigned int updatermac);

#endif
