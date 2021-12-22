# Worklog

- Created repository
- Added working lwIP version to CMake top-level
- Created "custom" folder for self written example
- Integrated tapif and stuff to custom folder
- Added test example (ping) to writer for now, works with a bugfix!
    - always use `sizeof(struct ip_hdr)` when assuming IPv4, do not rely on macro `PBUF_IP_HLEN`. Fixed this bug in contrib ping example.
- TODO: bridge tap, so that tap devices can ping network or 8.8.8.8