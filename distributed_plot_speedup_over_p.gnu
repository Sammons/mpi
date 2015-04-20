set terminal png nocrop enhanced
set output 'mpi.distributed_speedup_over_p.png'
set ylabel 'Ratio'
set xlabel 'P (N is fixed at 1500)'
plot "d_p_speedup.data"  using 1:2 title 'serial' with linespoints lw 2
