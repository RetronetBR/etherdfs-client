/*
 * UDP transport selector skeleton for the EtherDFS client branch.
 *
 * This file does not implement a DOS TCP/IP backend yet. It only defines the
 * future selection point for Watt-32, mTCP and uIP adapters.
 */

#include "udp_transport.h"

const struct udp_transport_ops *udp_transport_select(const char *backend) {
  backend = backend;
  return((const struct udp_transport_ops *)0);
}
