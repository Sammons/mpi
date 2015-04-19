#!bin/bash

GCC_48=$(g++ --version | sed s/4.8.2/CORRECT_COMPILER/ | grep CORRECT_COMPILER | wc --lines)

if [ $GCC_48 == "1" ]; then

	echo "purging stray files"
# purge
	git clean -xdf

# reset (murders your changes)
	echo "stashing changes"
	git stash
	echo "checking out master"
	git checkout master
	git reset origin --hard

# get latest
	echo "getting latest changes from master"
	git pull https://github.com/Sammons/mpi master

# load binaries
	echo "loading in mpi"
	module load openmpi-x86_64

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
			echo $j
			mpirun -n $i src/mpi /cluster/content/hpc/distributed_data/ $j $N >& run_$i.txt;
			cat run_$i.txt | grep -Eo ":[0-9].*$" | sed s/seconds// | grep -Eo "[0-9\.\e\-\+]*\b" | awk '{s+=$1} END {print s}' >> serial_search_$i.txt;
			cat bds8c7_results.txt | grep search-time | sed s/search-time:// | sed s/seconds// >> parallel_search_$i.txt;
			rm run_$i.txt;
			rm bds8c7_results.txt;
		done;
	done;
else

	echo "please run using hpc bash, or at least using gcc 4.8"

fi
