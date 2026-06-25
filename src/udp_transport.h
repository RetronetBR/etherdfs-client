/*
 * Abstract UDP transport interface for the EtherDFS client branch.
 */

#ifndef UDP_TRANSPORT_SENTINEL
#define UDP_TRANSPORT_SENTINEL

struct udp_transport_ops {
  int (*init)(void);
  int (*send)(unsigned char far *frame, unsigned int len);
  int (*recv)(unsigned char far *frame, unsigned int frame_max, unsigned int *len);
  void (*shutdown)(void);
};

const struct udp_transport_ops *udp_transport_select(const char *backend);

#endif
