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


#define TAG_INITIAL_TASK 0
#define TAG_COMPLETE 1


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
std::vector<image_vector<size>> read_file ( const std::string filename )
{
    std::string line;
    std::ifstream stream( filename );
    std::vector<image_vector<size>> output_set;
    if ( !stream.is_open () )
    {
        std::cout << "unable to read file " << filename << std::endl;
        return output_set;
    }
    while ( stream.good () )
    {
        getline ( stream, line );
        
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

int main ( int argc, char* argv[] )
{

    auto vectors = read_file<128>("/cluster/content/hpc/dev_data/data_aa");
    std::cout << vectors[0].image_id << " image id" << std::endl;
    for (int i = 0; i < 128; i++)
        std::cout << "\t" << vectors[0].data[i] << std::endl;

    //MPI_Init ( &argc, &argv );

    ///* fun information about who "I" am here*/
    //char hostname[ 255 ]; int len;
    //int id, procs;
    //MPI_Comm_rank ( MPI_COMM_WORLD, &id );
    //MPI_Comm_size ( MPI_COMM_WORLD, &procs );
    //MPI_Get_processor_name ( hostname, &len );

    ///* dispatch to everyone, including self*/
    //if ( id == 0 )
    //{
    //    int buffer[] = { 1, 2, 3, 4 };
    //    std::cout << "master issuing broadcast" << std::endl;
    //    for ( int i = 0; i < procs; ++i )
    //    {
    //        MPI_Send ( buffer, 4, MPI_INT32_T, i, TAG_INITIAL_TASK, MPI_COMM_WORLD );
    //        std::cout << "master sent initial task to " << i << std::endl;
    //    }
    //}

    //{
    //    /* child receives */
    //    int * local_receive_buffer = ( int* )malloc ( 4 * sizeof ( int ) );
    //    MPI_Recv ( local_receive_buffer, 4, MPI_INT32_T, 0, TAG_INITIAL_TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    //    std::cout << "child " << id << " receiving task" << std::endl;

    //    std::this_thread::sleep_for ( std::chrono::milliseconds ( id * 1000 ) );
    //    /* perform work */
    //    /* for now, just testing, does nothing */
    //    MPI_Send ( local_receive_buffer, 4, MPI_INT32_T, 0, TAG_COMPLETE, MPI_COMM_WORLD );
    //    std::cout << "child " << id << " sending back result" << std::endl;
    //    /* child cleanup */
    //    free ( local_receive_buffer );
    //}

    ///* wait for everyone to turn in their work */
    //MPI_Barrier ( MPI_COMM_WORLD );

    ///* gather */
    //if ( id == 0 )
    //{
    //    int * rec_buffer = ( int* )malloc ( 4 * procs * sizeof ( int ) );
    //    for ( int i = 0; i < procs; ++i )
    //    {
    //        MPI_Recv ( rec_buffer + ( 4 * i ), 4, MPI_INT32_T, i, TAG_COMPLETE, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    //    }

    //    /* print report */
    //    std::cout << "printing results:" << std::endl;
    //    for ( int i = 0; i < procs * 4; ++i )
    //        std::cout << "\t" << rec_buffer[ i ] << std::endl;
    //    free ( rec_buffer );
    //    std::cout << "complete" << std::endl;

    //}

    //MPI_Finalize ();
    return 0;
}