# set this to what I want
# https://software.intel.com/en-us/blogs/2012/09/26/gcc-x86-performance-hints
# https://gcc.gnu.org/onlinedocs/gcc-4.7.1/gcc/Optimize-Options.html
# procfile flags
# -mfpu -mvme -mde -mpse -mtsc -mmsr -mpae -mmce -mcx8 -mapic -msep -mmtrr -mpge -mmca -mcmov -mpat -mpse36 -mclflush -mdts
# -mmmx -mfxsr -msse -msse2 -mss -mht -msyscall -mnx -mlm -mconstant_tsc -march_perfmon -mpebs -mbts -mtsc_reliable -m
# -mnonstop_tsc -maperfmperf -munfair_spinlock -mpni -mssse3 -mcx16 -msse4_1 -mhypervisor -mlahf_lm -mdts

BENS_FLAGS=\
	-std=c++11 \
	-march=native \
	-O3 \
	-m64 \
	-mveclibabi=svml \
	-mfpmath=sse \
	-msse4.2 \
	-flto \
	-ftracer \
	-ffast-math \
	-fmerge-all-constants \
	-fno-signaling-nans \
	-fivopts \
	-fipa-matrix-reorg \
	-fvect-cost-model \
	-floop-flatten \
	-funroll-loops \
	-floop-strip-mine \
	-floop-parallelize-all \
	-floop-block \
	-ftree-vectorize \
	-ftree-loop-optimize \
	-ftree-loop-if-convert \
	-ftree-loop-if-convert-stores \
	-ftree-loop-distribution \
	-ftree-loop-distribute-patterns \
	-faggressive-loop-optimizations \
	-funsafe-loop-optimizations \
	-fvariable-expansion-in-unroller  \
	-fbranch-target-load-optimize \
	--param omega-eliminate-redundant-constraints=1 \
	--param omega-max-vars=2048 \
	--param omega-max-geqs=4096 \
	--param omega-max-eqs=2048 \
	--param omega-max-wild-cards=72 \
	--param max-gcse-memory=1073741824 \
	--param omega-hash-table-size=10017 \
	--param omega-max-keys=10000

THESE_FLAGS = $(OTHER_FLAGS) $(BENS_FLAGS) 
# Standard all target
all: hw1

# HW 1
hw1: hw1.o MatrixMultiply.o
	g++ -o hw1 hw1.o MatrixMultiply.o 

hw1.o:	hw1.cpp 
	g++ ${THESE_FLAGS} -Wall -c hw1.cpp 

MatrixMultiply.o : MatrixMultiply.hpp MatrixMultiply.cpp
	g++ ${THESE_FLAGS} -Wall -c MatrixMultiply.cpp

clean:
	rm -f *.o hw1


