set terminal png nocrop enhanced
set output 'mpi.parallel_and_serial_times_over_N.png'
set ylabel 'Seconds'
set xlabel 'P'
plot "l_n_basic.data" using 1:2 title 'serial' with linespoints lw 2, \
	 "l_n_basic.data"  using 1:3 title 'parallel' with linespoints lw 2
