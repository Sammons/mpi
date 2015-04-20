set terminal png nocrop enhanced
set output 'mpi.distributed_times_over_n.png'
set ylabel 'Seconds'
set xlabel 'N (P is fixed at 8)'
plot "d_n_basic.data"  using 1:2 title 'serial' with linespoints lw 2, \
	 "d_n_basic.data"  using 1:3 title 'parallel' with linespoints lw 2
