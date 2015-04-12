MY_FLAGS = \
-march=native \
-O3 \
-m64 \
-DNDEBUG \
-mveclibabi=svml \
-mfpmath=sse \
-msse4.2 \
-flto \
-ftracer \
-ffast-math \
-fmerge-all-constants \
-fno-signaling-nans \
-fno-unsafe-math-optimizations \
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
--param omega-eliminate-redundant-constraints=1

AM_CPPFLAGS = \
	$(BOOST_CPPFLAGS) $(MY_FLAGS)

AM_LDFLAGS = \
	$(BOOST_LDFLAGS)

bin_PROGRAMS = mpi
mpi_SOURCES = main.cpp 

mpi_LDADD = \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_SYSTEM_LIB)
