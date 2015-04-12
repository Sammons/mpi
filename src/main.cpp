#include <mpi.h>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/filesystem.hpp>


#define TAG_FILENAME 0
#define TAG_COMPLETE 1
#define TAG_STRINGSIZE 2
#define MAX_FILE_NAME_CHARS 4096

template <int size>
struct image_vector
{
    /* frankly, we don't need to track anything else in this lab */
    int image_id;
    std::array<int, size> data;
};



template<class t, int size, int start, int cur>
struct converter {
    static inline void map_tokens_to_image_vector ( const std::vector<std::string>& tokens, image_vector<size>& v )
    {
        v.data[ cur - 1 ] = static_cast< t >( atoi ( tokens[ start + ( cur - 1 ) ].c_str () ) );
        converter<t, size, start, cur - 1>::map_tokens_to_image_vector ( tokens, v );
    }
};

template<class t, int size, int start>
struct converter<t, size, start, 0>
{
    static inline void map_tokens_to_image_vector ( const std::vector<std::string>& tokens, image_vector<size>& v )
    {}
};

template <int size>
inline std::vector<image_vector<size>> read_file ( const std::string filename )
{
    std::string line;
    std::ifstream stream( filename );
    std::vector<image_vector<size>> output_set;
    if ( !stream.is_open () )
    {
        std::cout << "unable to read file " << filename << std::endl;
        return output_set;
    }
    while ( std::getline ( stream, line ) )
    {
        /* shatter the line */
        std::vector<std::string> tokens ( size + 7 /* meta data */ + 4 /* empty spaces */, "" );
        boost::split ( tokens, line, boost::is_any_of ( ",\"{}" ) );

        /* populate vector */
        image_vector<size> next_vector;

        //index_of_first_element_in_128_element_vector = 9;
        converter<int, size, 9, size>::map_tokens_to_image_vector ( tokens, next_vector );
        next_vector.image_id = atoi( tokens[ 0 ].c_str() );

        /* save the vector */
        output_set.push_back ( next_vector );
     }
    stream.close ();
    return output_set;
}

inline std::vector<std::string> get_file_names_in_dir ( std::string dirname )
{
    boost::filesystem::recursive_directory_iterator dir(dirname), end;
    std::vector<std::string> file_names;
    while ( dir != end )
    {
        if ( boost::filesystem::is_regular_file ( *dir ) )
        {
            std::cout << boost::filesystem::complete ( dir->path () ).string () << std::endl;
            file_names.push_back ( boost::filesystem::complete( dir->path() ).string() );
        }
        dir++;
    }
    return file_names;
}

inline int get_start_for_me ( const int& me, const int& us, const int& size )
{
    const int divides_to = size / us;
    if ( divides_to == 0 ) return 0;

    return me * divides_to;
}

inline int get_end_for_me ( const int& me, const int& us, const int& size )
{
    const int divides_to = size / us;
    if ( divides_to == 0 && me != 0) return 0;
    else return size;

    if ( me == us - 1 ) return size;
    else return ( me + 1 )* divides_to;
}


int main ( int argc, char* argv[] )
{
    MPI_Init ( &argc, &argv );

    if ( argc < 2 )
    {
        std::cout << "not enough args, usage mpirun <data directory> <search vector steps>" << std::endl;
        MPI_Finalize ();
        return 0;
    }

    /* fun information about who "I" am here*/
    char hostname[ 255 ]; int len;
    int id, procs;
    MPI_Comm_rank ( MPI_COMM_WORLD, &id );
    MPI_Comm_size ( MPI_COMM_WORLD, &procs );
    MPI_Get_processor_name ( hostname, &len );

    /* dispatch to everyone, including self*/
    if ( id == 0 )
    {
        /* get files to be delegated */
        auto files = get_file_names_in_dir ( argv[ 1 ] );

        int buffer[] = { 1, 2, 3, 4 };
        std::cout << "master issuing broadcast" << std::endl;
        for ( int i = 0; i < procs; ++i )
        {
            /* determine how many files to delegate to child */
            std::string my_joined_paths;
            for (int j = i; j < files.size (); j += procs ) 
                my_joined_paths += files[ j ] + "\n";

            std::cout << "files for:" << i << ":\n" << my_joined_paths;
            
            int package_size = my_joined_paths.size ();
            MPI_Send ( &package_size, 1, MPI_INT32_T, i, TAG_STRINGSIZE, MPI_COMM_WORLD );
            MPI_Send ( &my_joined_paths[0], package_size, MPI_CHAR, i, TAG_FILENAME, MPI_COMM_WORLD );

            std::cout << "master sent initial task to " << i << std::endl;
        }
    }

    {
        /* child receives */
        int package_size = 0;
        MPI_Recv ( &package_size, 1, MPI_INT32_T, 0, TAG_STRINGSIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        std::string receive_buffer ( package_size, '\0' );
        MPI_Recv ( receive_buffer.data, package_size, MPI_CHAR, 0, TAG_FILENAME, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        std::cout << "child " << id << " receiving task" << std::endl;
        std::cout << "received:\n" << receive_buffer << std::endl;
        std::this_thread::sleep_for ( std::chrono::milliseconds ( id * 1000 ) );
        /* perform work */
        /* for now, just testing, does nothing */
        MPI_Send ( &package_size, 1, MPI_INT32_T, 0, TAG_COMPLETE, MPI_COMM_WORLD );
        std::cout << "child " << id << " sending back result" << std::endl;
    }

    /* wait for everyone to turn in their work */
    MPI_Barrier ( MPI_COMM_WORLD );

    /* gather */
    if ( id == 0 )
    {
        int rec = 0;
        for ( int i = 0; i < procs; ++i )
        {
            MPI_Recv ( &rec, 1, MPI_INT32_T, i, TAG_COMPLETE, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            std::cout << "received " << rec << std::endl;
        }

        /* print report */
        std::cout << "complete" << std::endl;
    }

    MPI_Finalize ();
    return 0;
}
