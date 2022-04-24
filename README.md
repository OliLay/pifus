# pifus - A Predictable Interface for a User Space IP Stack

pifus is an interface for the lwIP stack. 
It is based on a completion I/O model and therefore allows async I/O using lwIP with an API similar to BSD sockets.
pifus supports predictability features, such as setting priorities for sockets so that real-time traffic can be 
prioritizes.
Applications can use the pifus library to connect to lwIP.

## Build
```bash
mkdir build && cd build
cmake ..
make
```

Then, the lwIP stack with the pifus backend is built in `lwip/custom/stack/stack`. 

## Usage
Applications that want to use the stack may use the pifus library. 
See `api/benchmarks` for example applications.

Use the environment variable `PRECONFIGURED_TAPIF=tapX` to set the TAP interface 
that the stack should use.