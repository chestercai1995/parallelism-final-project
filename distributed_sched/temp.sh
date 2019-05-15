!#/bin/bash

taskset 2 ./bin/sm_matmul 1500 &
taskset 3 ./bin/lg_matmul 4 &

