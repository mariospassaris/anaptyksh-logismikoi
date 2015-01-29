#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "graph.h"
#include "graph_entry.h"
#include "graph_stats.h"
#include "database.h"
#include "girvan_newman.h"

struct edge_measuring {
    int from;
    int to;
    int measure;
};

int calculate_modularity( ptr_graph g, size_t size );
int detect_edge( const void *a, const void *b );
int compare_edges( const void *a, const void *b );
struct Communities *construct_communities( ptr_graph g );
struct community *create_community( ptr_graph g, list_ptr left, int *left_number );
ptr_graph copy_graph( ptr_graph g, char *edge_type );
double edge_betweenness( ptr_graph g, int from, int to );
void edge_shortest_paths_rec_( ptr_graph g, int start, int end, int from, int to, int found, int *shortest_in, int *shortest_out, int dist, list_ptr checked );

void communities_print( struct Communities *this );
void community_print( struct community *this );

struct Communities *girvan_newman_method( double limit, ptr_graph g )
{
    ptr_graph cg;
    list_ptr initial, edges;
    HT_iter_ptr node_it;
    LL_iter_ptr edge_it;
    ptr_entry node;
    ptr_edge edge;
    struct edge_measuring **assignment, *edge_info, temp;
    struct edge sample;
    int i;
    size_t size;
    double modularity;
    struct Communities *retval;

/* Construct the initial values */

    /* Get a copy of the graph */
    cg = copy_graph( g, "person_knows_person.csv" );

    node_it = HT_iter_create( Graph_nodes(cg) );
    initial = LL_create( detect_edge );

    /* For each node in the graph */
    do {
        node = HT_iter_data( node_it );
        edges = node->edges;
        edge_it = LL_iter_create( edges );

        /* For each edge of the node */
        do {
            edge = (ptr_edge) LL_iter_data( edge_it );
            temp.from = node->id;
            temp.to = edge->target_id;
            if ( LL_search( initial, &temp ) ) {
                /* Ignore duplicate edges ( assuming graph is not oriented ) */
                continue;
            }
            edge_info = malloc( sizeof(struct edge_measuring) );
            memcpy( edge_info, &temp, sizeof(struct edge_measuring) );
            LL_insert( initial, edge_info );
        } while ( LL_iter_next( edge_it ) );
        LL_iter_destroy( edge_it );
    } while ( HT_iter_next( node_it ) );
    HT_iter_destroy( node_it );

    /* Do an initial calculation of the betweenness for each edge */
    edge_it = LL_iter_create( initial );
    do {
        edge_info = LL_iter_data( edge_it );
        edge_info->measure = edge_betweenness( cg, edge_info->from, edge_info->to );
    } while ( LL_iter_next( edge_it ) );
    assignment = (struct edge_measuring**) LL_export( initial );
    size = LL_size( initial );
    LL_destroy( initial, NULL );

/* Start the loop */
    sample.target_type = PERSON;
    sample.weight = 0;
    strcpy( sample.edge_type, "person_knows_person.csv" );
    do {
        /* Remove the most central edge */
        qsort( assignment, size, sizeof(struct edge_measuring*), compare_edges );
        edge_info = *assignment;

        /* Edge 1 */
        sample.target_id = edge_info->to;
        node = lookupNode( cg, edge_info->from );
        edges = node->edges;
        LL_delete( edges, &sample );

        /* Edge 2 */
        sample.target_id = edge_info->from;
        node = lookupNode( cg, edge_info->to );
        edges = node->edges;
        LL_delete( edges, &sample );

        /* Calculate again the betweenness for each other edge */
        for ( i = 0; i < size; ++i ) {
            assignment[i]->measure = edge_betweenness( g, assignment[i]->from, assignment[i]->to );
        }

        /* Construct the new Communities */
        retval = construct_communities( cg );

        /* Calculate the new modularity */
        modularity = calculate_modularity( cg, size );
    } while ( modularity < limit );

    return retval;
}

int calculate_modularity( ptr_graph g, size_t size )
{
    ht_ptr checked = HT_create( 8, 8, hash );
    ptr_entry node, node2;
    HT_iter_ptr node_it, node_it2;
    int dist;
    double sum = 0.0;

    node_it = HT_iter_create( Graph_nodes(g) );
    node_it2 = HT_iter_create( Graph_nodes(g) );

    do {
        node = HT_iter_data( node_it );
        do {
            node2 = HT_iter_data( node_it2 );
            if ( node->id == node2->id || HT_search( checked, node2->id ) ) {
                continue;
            }
            if ( ( dist = reachNode1( g, node->id, node2->id ) ) > 0 ) {
                sum += ( ( dist == 1 ) - ( ( LL_size( node->edges ) * LL_size( node2->edges ) ) / (double) ( 2 * size ) ) );
            }
        } while ( HT_iter_next( node_it2 ) );
        HT_insert( checked, (void*) node->id, node->id );
        HT_iter_reset( node_it2 );
    } while ( HT_iter_next( node_it ) );

    HT_destroy( checked, NULL );
    HT_iter_destroy( node_it );
    HT_iter_destroy( node_it2 );

    return sum / ( 2 * size );
}

int detect_edge( const void *a, const void *b )
{
    return ( ( (struct edge_measuring*) a )->from != ( (struct edge_measuring*) b )->from
        && ( ( (struct edge_measuring*) a )->to != ( (struct edge_measuring*) b )->to ) );
}

int compare_edges( const void *a, const void *b )
{
    return ( (struct edge_measuring*) b )->measure - ( (struct edge_measuring*) a )->measure;
}

struct Communities *construct_communities( ptr_graph g )
{
    struct Communities *ret;
    struct community *init_assign[ Graph_size( g ) ];
    ptr_entry node;
    list_ptr left;
    HT_iter_ptr node_it = HT_iter_create( Graph_nodes( g ) );
    int count = 0, left_number = 0;

    /* Construct the initial list of node ids */
    left = LL_create( compare_ints );
    do {
        node = HT_iter_data( node_it );
        LL_insert( left, (void*) node->id );
        ++left_number;
    } while ( HT_iter_next( node_it ) );
    HT_iter_destroy( node_it );

    /* Create communities while there are still nodes left */
    do {
        init_assign[ count++ ] = create_community( g, left, &left_number );
    } while ( left_number );
    LL_destroy( left, NULL );

    ret = malloc( sizeof(struct Communities) );
    ret->size = count;
    memcpy( ret->array, init_assign, count * sizeof(struct community*) );

    return ret;
}

struct community *create_community( ptr_graph g, list_ptr left, int *left_number )
{
    list_ptr edges, gathered = LL_create( compare_ints );
    ptr_edge edge;
    ht_ptr checked = HT_create( 8, 8, hash );
    HT_iter_ptr node_it, node_it2;
    ptr_entry node, node2;
    int target;
    struct community *retval;
    int id;


    node_it = HT_iter_create( Graph_nodes( g ) );
    node_it2 = HT_iter_create( Graph_nodes( g ) );

    /* First get a random first node to begin with */
    node = HT_iter_data( node_it );
    LL_insert( gathered, (void*) node->id );
    HT_insert( checked, (void*) node->id, node->id );
    --*left_number;

    /* First graph traverse */
    while ( HT_iter_next( node_it ) ) {
        node = HT_iter_data( node_it );

        /* Second graph traverse */
        do {
            node2 = HT_iter_data( node_it2 );
            if ( HT_search( checked, node2->id ) ) {
                continue;
            }
            if ( reachNode1( g, node->id, node2->id ) >= 0 ) {
                LL_insert( gathered, (void*) node2->id );
                LL_delete( left, (void*) node2->id );
                --*left_number;
            }
        } while ( HT_iter_next( node_it2 ) );
        HT_insert( checked, (void*) node->id, node->id );
        HT_iter_reset( node_it2 );
    } while ( HT_iter_next( node_it ) );

    HT_iter_destroy( node_it );
    HT_iter_destroy( node_it2 );
    HT_destroy( checked, NULL );

    retval = malloc( sizeof(struct community) );
    retval->id = id + 1;
    retval->size = LL_size( gathered );
    retval->members = (int*) LL_export( gathered );
    LL_destroy( gathered, NULL );
    return retval;
}

ptr_graph copy_graph( ptr_graph g, char *edge_type )
{
    ptr_graph retval;
    HT_iter_ptr node_it;
    ptr_entry addition, node;

    retval = createGraph( PERSON, 8, 8 );
    node_it = HT_iter_create( Graph_nodes(g) );
    do {
        node = HT_iter_data( node_it );
        addition = create_entry( node->id, node->properties, NULL );
        addition->edges = LL_copy( type_list( node, edge_type ) );
        insertNode( retval, addition );
    } while ( HT_iter_next( node_it ) );

    return retval;
}

double edge_betweenness( ptr_graph g, int from, int to )
{
    int size = Graph_size(g);
    int id, dist, next, found, shortest_in, shortest_out;
    double ret = 0.0;
    ht_ptr checked_nodes = HT_create( 8, 8, hash );
    list_ptr checked_paths;
    HT_iter_ptr node_it = HT_iter_create( Graph_nodes(g) );
    ptr_entry node;
    ResultSet *set;

    /* For each node in the graph */
    do {
        node = HT_iter_data( node_it );
        set = reachNodeN( g, node->id );
        checked_paths = LL_create( compare_ints );

        /* For each possible pair of nodes */
        while ( ( next = ResultSet_next( set, &id, &dist ) ) || 1 ) {
            do {
                if ( node->id == id ) {
                    break;
                }
                if ( dist < 0 ) {
                    break;
                }
                if ( HT_search( checked_nodes, id ) != NULL ) {
                    break;
                }
                found = 0;
                shortest_in = 0;
                shortest_out = 0;
                edge_shortest_paths_rec_( g, node->id, id, from, to, found, &shortest_in, &shortest_out, dist, checked_paths );
                if ( shortest_in || shortest_out ) {
                    ret += ( (double) shortest_in ) / ( shortest_in + shortest_out );
                }
            } while(0);
            if ( next == 0 ) {
                break;
            }
        }
        HT_insert( checked_nodes, node, node->id );
        LL_destroy( checked_paths, NULL );
    } while ( HT_iter_next( node_it ) );
    HT_destroy( checked_nodes, NULL );
    HT_iter_destroy( node_it );

    return ret;
}

void edge_shortest_paths_rec_( ptr_graph g, int start, int end, int from, int to, int found, int *shortest_in, int *shortest_out, int dist, list_ptr checked )
{
    int temp, old_found, node_id;
    ptr_entry starting_node;
    ptr_edge edge;
    list_ptr edges;
    LL_iter_ptr edge_it;
    double sum;

    if ( dist == 0 ) {
        if ( start == end ) {
            if ( found ) {
                ++*shortest_in;
            } else {
                ++*shortest_out;
            }
        }
        return;
    }
    starting_node = lookupNode( g, start );
    edges = (list_ptr) type_list( starting_node, "person_knows_person.csv" );
    assert( edges != NULL );

    edge_it = LL_iter_create( edges );
    do {
        edge = LL_iter_data( edge_it );
        node_id = edge->target_id;
        if ( LL_search( checked, (void*) node_id ) ) {
            continue;
        }
        old_found = found;
        LL_insert( checked, (void*) node_id );
        if ( ( start == from && node_id == to ) || ( start == to && node_id == from ) ) {
            found = 1;
        }
        edge_shortest_paths_rec_( g, node_id, end, from, to, found, shortest_in, shortest_out, dist - 1, checked );
        LL_delete( checked, (void*) node_id );
        found = old_found;
    } while ( LL_iter_next( edge_it ) );
    LL_iter_destroy( edge_it );
}

void communities_print( struct Communities *this )
{
    int i, size = this->size;
    for ( i = 0; i < size; ++i ) {
        community_print( this->array[i] );
    }
}

void community_print( struct community *this )
{
    int i, size = this->size;

    puts( "{ " );
    for ( i = 0; i < size; ++i ) {
        printf( "%d ", this->members[i] );
    }
    puts( "}\n" );
}