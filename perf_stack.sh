# execute with sudo!

rm /dev/shm/*
PRECONFIGURED_TAPIF=tap1 perf record -F 99 -g ./build/lwip/custom/stack/stack & 
sleep 60
perf script > out.perf

../FlameGraph/stackcollapse-perf.pl out.perf > out.folded
../FlameGraph/flamegraph.pl out.folded > kernel.svg