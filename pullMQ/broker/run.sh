#!/bin/sh
export BROKER_HOST="onmax"
make clean
make
<<<<<<< HEAD
./broker 12312
=======
ltrace ./broker 12302
>>>>>>> a88061565070884c8c679c182313dd25588d65a5
