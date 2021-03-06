
# build
echo "building"
# autoreconf -ivf
# ./configure
# make

module load openmpi-x86_64
echo "done building"

echo "------------------------------------------"
echo "     starting babbage-local analysis      "
echo ""
echo "These are just variations of running      "
echo "mpirun -n # src/mpi ........"
echo ""
echo "------------------------------------------"

echo "1) running 3 times per P, with a different vector each time "
echo "   where P is the number of threads, and ranges from "
echo "   1 to 16. we search for the 1500 nearest neighbors."
N=1500;
for i in `seq 1 19`; # P (processes)
do
	echo $i
	# perform run
	for j in `seq 1 10`; # vector seed
	do
		echo $j &&
		mpirun -n $i src/mpi /cluster/content/hpc/distributed_data/ $j $N >& run_$i.txt &&
		cat run_$i.txt | grep -Eo ":[0-9].*$" | sed s/seconds// | grep -Eo "[0-9\.\e\-\+]*\b" | awk '{s+=$1} END {print s}' >> serial_search_$i.txt &&
		cat bds8c7_results.txt | grep search-time | sed s/search-time:// | sed s/seconds// >> parallel_search_$i.txt &&
		rm run_$i.txt &&
		rm bds8c7_results.txt
	done;
done;

# accumulate data and produce graphs
# with N = 1500, 10 samples
rm l_p_*.txt;
SAMPLE_COUNT=10;
for i in `seq 1 19`; # P (processes)
do
	echo $i >> l_p_count.txt;
	cat serial_search_$i.txt | awk '{s+=$1} END {print s}' | awk '{print $1/"'"$SAMPLE_COUNT"'"}' >> l_p_average_serial.txt;
	cat parallel_search_$i.txt | awk '{s+=$1} END {print s}' | awk '{print $1/"'"$SAMPLE_COUNT"'"}' >> l_p_average_parallel.txt;
done;
paste l_p_average_serial.txt l_p_average_parallel.txt > l_p_averages_together.txt;
cat l_p_averages_together.txt | awk '{ print $1/$2 }' > l_p_average_parallel_speedup.txt;
paste l_p_count.txt l_p_averages_together.txt > l_p_basic.data;
paste l_p_count.txt l_p_average_parallel_speedup.txt > l_p_speedup.data;
gnuplot < local_plot_parallel_and_serial_times_over_p.gnu;
gnuplot < local_plot_speedup_over_p.gnu;
echo "----------DONE-----------";
# wall-clock & serial : vs P

# ratio vs P

echo "--------------------------------";
echo "    P=8, N=100,500,1000,1500";
echo "--------------------------------";


# with P fixed at 8, 10 samples
rm parallel_search*.txt;
rm serial_search*.txt;
rm l_n*.data
P=8;
# 500 1000 1500;
for i in 100 500 1000 1500;
do
	echo $i
	# perform run
	for j in `seq 1 20`; # vector seed
	do
		echo $j &&
		mpirun -n $P src/mpi /cluster/content/hpc/distributed_data/ $j $i >& run_$i.txt &&
		cat run_$i.txt | grep -Eo ":[0-9].*$" | sed s/seconds// | grep -Eo "[0-9\.\e\-\+]*\b" | awk '{s+=$1} END {print s}' >> serial_search_$i.txt &&
		cat bds8c7_results.txt | grep search-time | sed s/search-time:// | sed s/seconds// >> parallel_search_$i.txt
		# rm run_$i.txt &&
		# rm bds8c7_results.txt;
	done;
done;

SAMPLE_COUNT=10;
rm l_n_*.txt;
for i in 100 500 1000 1500;
do
	echo $i >> l_n_count.txt;
	cat serial_search_$i.txt | awk '{s+=$1} END {print s}' | awk '{print $1/"'"$SAMPLE_COUNT"'"}' >> l_n_average_serial.txt;
	cat parallel_search_$i.txt | awk '{s+=$1} END {print s}' | awk '{print $1/"'"$SAMPLE_COUNT"'"}' >> l_n_average_parallel.txt;
done;
paste l_n_average_serial.txt l_n_average_parallel.txt > l_n_averages_together.txt;
paste l_n_count.txt l_n_averages_together.txt > l_n_basic.data;
cat l_n_averages_together.txt | awk '{ print $1/$2 }' > l_n_average_parallel_speedup.txt;
paste l_n_count.txt l_n_average_parallel_speedup.txt > l_n_speedup.data;
gnuplot < local_plot_parallel_and_serial_times_over_n.gnu;
gnuplot < local_plot_speedup_over_n.gnu;
echo "--------DONE---------"
# wall-clock & serial : vs N
# ratio vs N
