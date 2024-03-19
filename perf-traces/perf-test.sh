perf stat --repeat 10 -e branches,cycles,instructions,context-switches \
 ./qtest -v 0 -f perf-traces/trace.cmd