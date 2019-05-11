#!/usr/bin/env bash

set -eu

pname=${1:-kthreadd}  # default to 'kthreadd'
for pid in $(pgrep "${pname}"); do
    echo "PID: ${pid} (${pname})"
    for tid in $(pgrep -P "${pid}" | tr '\n' ' '); do
        taskset -cp "${tid}"
    done
done
