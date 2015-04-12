#include <mpi.h>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>


#define TAG_INITIAL_TASK 0
#define TAG_COMPLETE 1



/* maps strings into a container of integer types*/
template<class o, int total_count, int start_index, int this_count>
struct convert
{
    inline static void loop_map_strings_to_ints ( std::array<std::string, total_count>& input, std::array<o, total_count>& output )
    {
        output[ this_count - 1 ] = atoi ( input[ start_index + (this_count - 1) ].c_str() );
        convert<o, total_count, start_index, this_count - 1>::loop_map_strings_to_ints ( input, output );
    }

};
/* zero specifier */
template<class o, int total_count, int start_index>
struct convert<o, total_count, start_index, 0> { inline static void loop_map_strings_to_ints ( std::array<std::string, total_count>& input, std::array<o, total_count>& output ) {} };


template <int size>
struct image_vector
{
    /* frankly, we don't need to track anything else in this lab */
    int image_id;
    std::array<short, size> data;
};

// template <int size>
// std::vector<image_vector<size>> read_file ( const std::string filename )
// {
//     std::string line;
//     std::ifstream stream = std::ifstream ( filename );
//     std::vector<image_vector<size>> output_set;
//     if ( !stream.is_open () )
//     {
//         std::cout << "unable to read file " << filename << std::endl;
//         return output_set;
//     }
//     while ( stream.good () )
//     {
//         getline ( stream, line );
        
//         /* shatter the line */
//         std::array<std::string, size + 7 /* meta data */ + 4 /* empty spaces */> tokens;
//         boost::split ( tokens, line, boost::is_any_of ( ",\"{}" ) );

//         /* populate vector */
//         image_vector<size> next_vector;
//         convert<int, size, 9, size> (tokens, next_vector.data);
//         next_vector.image_id = tokens[ 0 ];

//         /* save the vector */
//         output_set.push_back ( next_vector );
//      }
//     stream.close ();
//     return output_set;
// }

int main ( int argc, char* argv[] )
{
    MPI_Init ( &argc, &argv );

    /* fun information about who "I" am here*/
    char hostname[ 255 ]; int len;
    int id, procs;
    MPI_Comm_rank ( MPI_COMM_WORLD, &id );
    MPI_Comm_size ( MPI_COMM_WORLD, &procs );
    MPI_Get_processor_name ( hostname, &len );

    /* dispatch to everyone, including self*/
    if ( id == 0 )
    {
        int buffer[] = { 1, 2, 3, 4 };
        std::cout << "master issuing broadcast" << std::endl;
        for ( int i = 0; i < procs; ++i )
        {
            MPI_Send ( buffer, 4, MPI_INT32_T, i, TAG_INITIAL_TASK, MPI_COMM_WORLD );
            std::cout << "master sent initial task to " << i << std::endl;
        }
    }

    {
        /* child receives */
        int * local_receive_buffer = ( int* )malloc ( 4 * sizeof ( int ) );
        MPI_Recv ( local_receive_buffer, 4, MPI_INT32_T, 0, TAG_INITIAL_TASK, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        std::cout << "child " << id << " receiving task" << std::endl;

        std::this_thread::sleep_for ( std::chrono::milliseconds ( id * 1000 ) );
        /* perform work */
        /* for now, just testing, does nothing */
        MPI_Send ( local_receive_buffer, 4, MPI_INT32_T, 0, TAG_COMPLETE, MPI_COMM_WORLD );
        std::cout << "child " << id << " sending back result" << std::endl;
        /* child cleanup */
        free ( local_receive_buffer );
    }

    /* wait for everyone to turn in their work */
    MPI_Barrier ( MPI_COMM_WORLD );

    /* gather */
    if ( id == 0 )
    {
        int * rec_buffer = ( int* )malloc ( 4 * procs * sizeof ( int ) );
        for ( int i = 0; i < procs; ++i )
        {
            MPI_Recv ( rec_buffer + ( 4 * i ), 4, MPI_INT32_T, i, TAG_COMPLETE, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        }

        /* print report */
        std::cout << "printing results:" << std::endl;
        for ( int i = 0; i < procs * 4; ++i )
            std::cout << "\t" << rec_buffer[ i ] << std::endl;
        free ( rec_buffer );
        std::cout << "complete" << std::endl;

    }

    MPI_Finalize ();
    return 0;
}