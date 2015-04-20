set terminal png nocrop enhanced
set output 'mpi.distributed_speedup_over_n.png'
set ylabel 'Ratio'
set xlabel 'N (P is fixed at 8)'
plot "d_n_speedup.data"  using 1:2 title 'serial' with linespoints lw 2
