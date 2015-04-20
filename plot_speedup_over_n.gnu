set terminal png nocrop enhanced
set output 'mpi.local_speedup_over_n.png'
set ylabel 'Seconds'
set xlabel 'P'
plot "l_n_speedup.data" using 1:2 title 'parallel speedup over N increase' with linespoints lw 2
