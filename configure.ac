AC_INIT([hw0], [0.0], [bds8c7@mail.missouri.edu])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.hpp])
AX_MPI
CXX="$MPICXX"
AX_BOOST_BASE
AX_BOOST_SERIALIZATION
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_PROGRAM_OPTIONS
AX_CXX_COMPILE_STDCXX_11
CXXFLAGS="-std=c++11"
CXX="mpic++"
AX_BOOST_MPI
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
