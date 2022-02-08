#!/bin/bash
clear
tmux clear-history;

rm /dev/shm/*
PRECONFIGURED_TAPIF=tap1 ./build/lwip/custom/stack/stack
