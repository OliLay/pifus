#!/bin/bash
clear
tmux clear-history;
rm /dev/shm/*
PRECONFIGURED_TAPIF=tap0 ./build/lwip/custom/benchmark/reader/reader
