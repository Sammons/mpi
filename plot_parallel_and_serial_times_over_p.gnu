set terminal png nocrop enhanced
set output 'mpi.parallel_and_serial_times_over_P.png'
set ylabel 'Seconds'
set xlabel 'P'
plot "l_n_speedup.data" using 1:2 title 'serial' with linespoints, \
	 "l_n_speedup.data"  using 1:3 title 'parallel' with linespoints
