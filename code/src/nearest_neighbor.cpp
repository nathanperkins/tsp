#include "nearest_neighbor.hpp"

#include <stdio.h>
#include <ctime>
#include <limits>

#if defined(_OPENMP)
#include <omp.h>
#else
#include <time.h>
#endif

// TODO: replace clock with suggestion here: https://stackoverflow.com/questions/2962785/c-using-clock-to-measure-time-in-multi-threaded-programs

// adapted from https://en.wikipedia.org/wiki/Nearest_neighbour_algorithm
// asssuming that edges in the problem are already sorted!!!
struct tour tsp_nearest_neighbor( struct tsp_problem &problem ) {

    #if defined(_OPENMP)
    float start_time = omp_get_wtime();
    #else
    float start_time = clock();
    #endif

    struct tour best_tour(problem.cities.size());
    best_tour.distance = std::numeric_limits<int>::max();

    // we will test greedy tours at every starting point
    #if defined(_OPENMP)
    #pragma omp parallel for
    #endif
    for( unsigned starting_id = 0; starting_id < problem.cities.size(); ++starting_id ) {

        // only continue if clock is under 60s
        #if defined(_OPENMP)
        float elapsed = omp_get_wtime() - start_time;
        #else
        float elapsed = (clock() - start_time) / CLOCKS_PER_SEC;
        #endif

        if ( elapsed < 60 ) {
            // create a new tour and add the first city
            struct tour* current_tour = new struct tour(problem.cities.size());
            struct city* start = &(problem.cities[starting_id]);
            current_tour->cities.push_back(start);
            current_tour->visited[start->id] = true;

            // while the current_tour is not complete
            while( current_tour->cities.size() < problem.cities.size() ) {

                // starting at the current end of the tour
                struct city* current_city = current_tour->cities.back();

                int min = std::numeric_limits<int>::max();
                int min_id = -1;

                // look through the edges for the current city
                for ( unsigned i = 0; i < problem.cities.size(); ++i ) {
                    int adj_index = matrix_index(current_city->id, i, problem.cities.size());

                    // if this new city is not visited and is the closest so far, set the new minimum
                    if ( !current_tour->visited[i] && problem.adjacency[adj_index] < min) {
                        min = problem.adjacency[adj_index];
                        min_id = i;
                    }
                }

                // add the closest city found
                add_city_to_tour(*current_tour, &(problem.cities[min_id]), min);
            }

            // return to the starting city
            complete_tour(*current_tour, problem.adjacency);
            
            // only one thread can run this at a time
            #if defined(_OPENMP)
            elapsed = omp_get_wtime() - start_time;
            #pragma omp critical
            #else
            elapsed = (clock() - start_time) / CLOCKS_PER_SEC;
            #endif
            {
                // change best_tour if this current tour is better.
                if( current_tour->distance < best_tour.distance ) {
                    printf("city: %8d, distance: %8d, elapsed: %.4fs\n", starting_id, current_tour->distance, elapsed);
                    best_tour = *current_tour;
                } else {
                    delete current_tour;
                }
            }
        }
    }

    return best_tour;
}
