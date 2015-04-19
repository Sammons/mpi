
# build
echo "building"
autoreconf -ivf
./configure
make

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
for i in `seq 1 16`; 
do
	echo $i
	# perform run
	for j in `seq 1 3`;
	do
		echo $j &&
		mpirun -n $i src/mpi /cluster/content/hpc/distributed_data/ $j $N >& run_$i.txt &&
		cat run_$i.txt | grep -Eo ":[0-9].*$" | sed s/seconds// | grep -Eo "[0-9\.\e\-\+]*\b" | awk '{s+=$1} END {print s}' >> serial_search_$i.txt &&
		cat bds8c7_results.txt | grep search-time | sed s/search-time:// | sed s/seconds// >> parallel_search_$i.txt &&
		rm run_$i.txt &&
		rm bds8c7_results.txt
	done;
done;
