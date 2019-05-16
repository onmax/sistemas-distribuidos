#!/bin/sh
export BROKER_HOST="onmax"
make clean
make
ltrace ./broker 12302