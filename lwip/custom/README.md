# Custom (benchmark and pifus)

## Structure
- `Benchmark` folder contains example applications and benchmarks for assessing functionality / performance of lwIP and `pifus`
- `pifus` folder consists of the code for the binary that contains `lwip` and the stack-side API for `pifus`

## Build
Build via toplevel (`lwip` folder), see instructions there

## Usage
1. Execute `./setup-tapif`

    This will setup the tap interfaces (2 currently) and a bridge. Additionally adds `iptables` rules so that the tap interfaces can also talk to each other on IP layer.

1. Start the applications you want to use. 

    Set or pass the environment variable `PRECONFIGURED_TAPIF` with the interface name of the tap interface as value.

