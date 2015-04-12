#include <mpi.h>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <thread>
#include <array>
#include <vector>
#include <set>
#include <map>
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
template<int size>
image_vector<size> get_search_image_vector ( int which )
{
    srand ( which );
    image_vector<size> v { 0 };
    v.image_id = 0;
    for ( int i = 0; i < size; ++i )
        v.data[ i ] = rand() % 255;
    return v;
}

struct rank
{
    float distance;
    int image_id;

    /* for sorting */
    inline bool operator<( const rank& b ) const { return distance < b.distance; }

};

inline std::string generate_output_file_name ( const std::string& seed, int tries = 0 )
{
    const std::string output = boost::filesystem::basename(seed) + "_output_" + std::to_string(tries);
    if ( boost::filesystem::exists ( output ) )
        return generate_output_file_name ( seed, tries + 1 );
    return output;
}
template<int vectLen>
inline float manhattanDissimilarity ( const std::array<int, vectLen>& a, const std::array<int, vectLen>& b )
{
    int dis = 0;
    for ( int i = 0; i < vectLen; ++i )
    {
        // std::cout << dis << "m\n";
        dis += abs ( a[ i ] - b[ i ] );
    }
    return ( float )dis / ( float )vectLen;
}
template<int size, int c>
struct ranker
{
    inline static int sum_distances ( const image_vector<size>& a, const image_vector<size>& b )
    {
        return abs( a.data[ c-1 ] - b.data[ c-1 ] ) + ranker<size, c-1>::sum_distances ( a, b );
    }
    inline static float rank_vectors ( const image_vector<size>& a, const image_vector<size>& b )
    {
        return manhattanDissimilarity<size> ( a.data, b.data );
    }
};

template<int size>
struct ranker<size, 0>
{
    inline static int sum_distances ( const image_vector<size>& a, const image_vector<size>& b ){}
};

/* stores results in a file, only returns new filename */
template<int size>
std::string rank_file ( const std::string& filename, const image_vector<size>& search_vector, const int& nearest_neighbors )
{
    /* determine what the path we will write to is */
    const std::string output_file_path = generate_output_file_name ( filename );

    /* create the file, in case we do not have permissions - we want to fail fast*/
    FILE * handle = fopen ( output_file_path.c_str (), "wb+" );
    if ( handle == NULL )
    {
        std::cout << "incapable of opening file in this location " + output_file_path << std::endl;
        return "";
    }

    /* SERIOUS assumption here!!! that we are delegating properly, e.g. the file can fit into memory 
    but this really isn't that serious, considering that we can always just split the files up into
    smaller pieces */
    const std::vector<image_vector<size>> vectors_to_rank = read_file<size> ( filename );
    const int num_rankings = vectors_to_rank.size ();
    std::vector<rank> rankings(num_rankings);
    std::map<int, float> u_set;
    for (int i = 0; i < num_rankings; ++i )
    {
        /* calculate distances, and insert into map*/
        const float dist = ranker<size, size>::rank_vectors ( vectors_to_rank[ i ], search_vector );
        const int id = vectors_to_rank[ i ].image_id;
        if ( dist == 0 )
        {
            std::cout << "WHAT THE CRAY Z STUFFS" << std::endl;
        }
        const auto existing = u_set.find ( id );
        if ( existing != u_set.end () )
        {
            if ( u_set[ id ] > dist ) u_set[ id ] = dist;
        }
        else
        {
            u_set[ id ] = dist;
        }
    }

    auto iter = u_set.begin ();
    for (register int i = 0; i < num_rankings; ++i )
    {
        rankings[ i ].image_id = iter->first;
        rankings[ i ].distance = iter->second;
        if ( rankings[ i ].distance == 0 )
        {
            std::cout << "WHAT THE CRAY Z STUFFS 2" << std::endl;
        }
        ++iter;
    }

    int output_size = rankings.size ();

    /* breadcrumb for deserialization, how many there are */
    fwrite ( &output_size, sizeof ( int ), 1, handle );

    /* sort */
    std::sort ( rankings.begin (), rankings.end () );

    /* serialize results */
    fwrite ( &rankings[ 0 ], sizeof ( rank ), output_size, handle );

    fclose ( handle );
    return output_file_path;
}

inline void deserialize_and_merge_file ( const std::string& path, std::map<int, float>& distances, int nearest_neighbors )
{
    /* open that file thing*/
    FILE * handle = fopen ( path.c_str(), "rb" );
    if ( handle == NULL )
    {
        std::cout << "failed to open partial result " << path << std::endl;
        return;
    }
    /* read how many ranks are contained */
    int how_many_rankings = 0;
    fread ( &how_many_rankings, sizeof ( int ), 1, handle );

    /* allocate a vector to read these suckers into*/
    std::vector<rank> rankings ( how_many_rankings );
    fread ( &rankings[ 0 ], sizeof ( rank ), how_many_rankings, handle );

    /* merge them into the distances*/
    for ( int i = 0; i < how_many_rankings; ++i )
    {
        if ( distances.find ( rankings[ i ].image_id ) != distances.end () )
        {
            if ( rankings[ i ].distance < distances[ rankings[ i ].image_id ] )
                distances[ rankings[ i ].image_id ] = rankings[ i ].distance;
        }
        else
        {
            distances[ rankings[ i ].image_id ] = rankings[ i ].distance;
        }
    }

    fclose ( handle );
    /* clean up after ourselves */
    boost::filesystem::remove ( path );
}

int main ( int argc, char* argv[] )
{
    MPI_Init ( &argc, &argv );

    if ( argc < 4 )
    {
        std::cout << "not enough args, usage mpirun <data directory> <search vector steps> <nearest neighbors>" << std::endl;
        MPI_Finalize ();
        return 0;
    }
    constexpr int vector_sizes = 128;
    const auto search_vector = get_search_image_vector<vector_sizes> ( atoi ( argv[ 2 ] ) );
    const int nearest_neighbors = atoi ( argv[ 3 ] );
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
        MPI_Recv ( &receive_buffer[0], package_size, MPI_CHAR, 0, TAG_FILENAME, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        std::cout << "child " << id << " receiving task" << std::endl;
        std::cout << "received:\n" << receive_buffer << std::endl;

        /* split apart file names*/
        std::vector<std::string> file_paths;
        boost::split ( file_paths, receive_buffer, boost::is_any_of ( "\n" ) );

        /*files we are going to send back to the master*/
        std::vector<std::string> output_file_paths;

        /* process each file independently */
        for ( auto path : file_paths )
        {
            if ( path == "" ) continue;
            output_file_paths.push_back ( rank_file ( path, search_vector, nearest_neighbors) );
        }

        /* packaging things to send out */
        std::string output_buffer = boost::join ( output_file_paths, "\n" );
        package_size = output_buffer.size ();

        /* send back number of file names included */
        MPI_Send ( &package_size, 1, MPI_INT32_T, 0, TAG_STRINGSIZE, MPI_COMM_WORLD );

        /* send back output file names */
        MPI_Send ( &output_buffer[0], package_size, MPI_CHAR, 0, TAG_COMPLETE, MPI_COMM_WORLD );
        
        std::cout << "child " << id << " sending back result" << std::endl;
    }

    /* wait for everyone to turn in their work */
    MPI_Barrier ( MPI_COMM_WORLD );

    /* gather */
    if ( id == 0 )
    {
        std::vector<std::string> partial_result_files;
        for ( int i = 0; i < procs; ++i )
        {
            int rec = 0;
            std::vector<std::string> some_partial_result_files;
            
            /* master receive */
            MPI_Recv ( &rec, 1, MPI_INT32_T, i, TAG_STRINGSIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            std::string incoming_buffer ( rec, '\0' );
            MPI_Recv ( &incoming_buffer[ 0 ], rec, MPI_CHAR, i, TAG_COMPLETE, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

            /* get the file names */
            boost::split ( some_partial_result_files, incoming_buffer, boost::is_any_of ( "\n" ) );
            
            /* append them to the master list */
            partial_result_files.insert ( partial_result_files.end (), some_partial_result_files.begin (), some_partial_result_files.end () );
            
            std::cout << "received " << rec << " more parts" <<std::endl;
        }


        /* dedup + extract data */
        std::map<int, float> distances;
        for ( auto presult_path : partial_result_files )
        {
            /* merge the file into distances */
            deserialize_and_merge_file ( presult_path, distances, nearest_neighbors );
        }

        std::vector<rank> result;

        /* convert our map back into rankings, now we have no duplicates and just need to sort*/
        for ( auto each = distances.begin ();
              each != distances.end (); ++each )
        {
            rank a;
            a.image_id = each->first;
            a.distance = each->second;
            result.push_back ( a );
        }

        /* sort result */
        std::sort ( result.begin (), result.end () );

        /* truncate result to nearest neighbors */
        result.resize ( nearest_neighbors );

        /* print report */
        std::cout << "NEAREST NEIGHBORS" << std::endl;
        for ( int i = 0; i < result.size (); ++i )
        {
            std::cout << i << "\t" << result[ i ].image_id << ": " << result[ i ].distance << std::endl;
        }
        std::cout << "complete" << std::endl;
    }

    MPI_Finalize ();
    return 0;
}
