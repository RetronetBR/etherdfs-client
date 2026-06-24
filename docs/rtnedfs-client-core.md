# Retronet EtherDFS Client Core Extraction

This branch is the client-side counterpart of the server refactor.

## Goal

Split the DOS client into three layers:

- DOS redirector glue
- EtherDFS protocol logic
- packet driver transport

This will make it possible to keep the current raw Ethernet mode while adding a UDP transport later.

## Current source layout

The implementation started in `src/ETHERDFS.C`, but the first refactor step has already split out shared helpers and packet-driver transport:

- `src/clientcore.c`
  - shared helpers reused across modules
  - string and checksum helpers

- `src/pktdrv.c`
  - packet driver transport
  - MAC discovery
  - frame send/receive callbacks
  - `sendquery()`

- `src/ETHERDFS.C`
  - DOS redirector logic
  - filesystem operation translation
  - install/uninstall path
  - TSR residency

## Proposed split

### `client_proto.c/.h`

Responsibilities:

- build and parse EtherDFS requests
- manage `seq`
- compute BSD checksum
- validate replies
- expose per-operation request helpers

### `pktdrv.c/.h`

Responsibilities:

- packet driver registration
- receive callback
- send raw Ethernet frames
- MAC discovery
- request/response retries

### `client_redirector.c/.h`

Responsibilities:

- INT 2F handler
- DOS SDA/CDS/SFT/DTA integration
- map DOS calls to protocol helpers

### `client_core.c/.h`

Responsibilities:

- shared client session state
- retry/backoff
- server endpoint selection
- transport abstraction

## Extraction order

1. Keep trimming `ETHERDFS.C` until it only contains redirector and TSR glue.
2. Move the request framing API into a thinner protocol layer if needed.
3. Introduce an abstract transport interface.
4. Add a UDP transport once the core is stable.

## Notes

- The DOS redirector code is tightly coupled to `SDA`, `CDS` and `SFT`.
- The protocol code can be reused almost as-is for UDP.
- The transport layer is the part that changes the most between raw Ethernet and UDP.
