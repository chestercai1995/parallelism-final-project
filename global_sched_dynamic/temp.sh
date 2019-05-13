!#/bin/bash

taskset 1 ./bin/matmul_aware_norecur 2 &
taskset 2 ./bin/streaming 2 &

