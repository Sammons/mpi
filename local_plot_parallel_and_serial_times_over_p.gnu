set terminal png nocrop enhanced
set output 'mpi.local_times_over_p.png'
set ylabel 'Seconds'
set xlabel 'P (N is fixed at 1500)'
plot "l_p_basic.data"  using 1:2 title 'serial' with linespoints lw 2, \
	 "l_p_basic.data"  using 1:3 title 'parallel' with linespoints lw 2
