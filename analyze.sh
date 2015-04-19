#!bin/bash

GCC_48=$(g++ --version | sed s/4.8.2/CORRECT_COMPILER/ | grep CORRECT_COMPILER | wc --lines)

if [ $GCC_48 == "1" ]; then

	echo "purging stray files"
# purge
	git clean -xdf

# #reset (murders your changes)
	echo "stashing changes"
	git stash
	echo "checking out master"
	get checkout master
	git reset origin --hard

# #get latest
	echo "getting latest changes from master"
	git pull https://github.com/Sammons/mpi

# #load binaries
	echo "loading in mpi"
	module load openmpi-x86_64

else

	echo "please run using hpc bash, or at least using gcc 4.8"

fi
