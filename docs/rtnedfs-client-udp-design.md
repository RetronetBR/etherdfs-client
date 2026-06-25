# Retronet EtherDFS Client UDP Design

This branch is reserved for UDP-related transport work on the DOS client.

The stable `etherdfs.exe` TSR remains the raw-Ethernet client used by the
current branch that is ready for merge into `main`.

## Goal

Add UDP transport support without mixing it into the packet-driver path.

The target runtime stack is:

```text
DOS TSR / redirector
  -> transport backend
  -> Watt-32, mTCP, or uIP
  -> UDP/IP
  -> EtherDFS server
```

## Branch layout

- `feature/rtnedfs-client-core`
  - stable DOS client
  - raw Ethernet transport
  - ready for merge into `main`

- `feature/rtnedfs-client-udp`
  - UDP transport work
  - backend adapters for Watt-32, mTCP, and uIP
  - no raw-Ethernet changes unless shared code is promoted to `core`

## Planned files

- `src/udp_transport.h`
  - abstract UDP transport operations

- `src/udp_watt32.c`
  - Watt-32 backend

- `src/udp_mtcp.c`
  - mTCP backend

- `src/udp_uip.c`
  - uIP backend

## Notes

- The client does not gain a Linux-style daemon.
- UDP stays inside the DOS client as an alternate transport layer.
- Raw Ethernet remains the default path until a UDP backend is complete.
