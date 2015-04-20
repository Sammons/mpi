set terminal png nocrop enhanced
set output 'mpi.local_speedup_over_p.png'
set ylabel 'Ratio'
set xlabel 'P (N is fixed at 1500)'
plot "l_p_speedup.data"  using 1:2 title 'serial' with linespoints lw 2
