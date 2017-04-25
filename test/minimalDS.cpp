#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <assert.h>
#include <ctime>

#include <immintrin.h>
#include <emmintrin.h>

#include "Graph.h"

using namespace std;


typedef unsigned int uint;

typedef std::vector<int> svect;

typedef unsigned char STATE;
typedef std::vector<STATE> STATES;

#define White 0
#define Black 1
#define Green 2

// get the number of white neighbors of vert v
inline const unsigned int numWhiteNeighbors( const Graph& g, const STATES& states, const int v){
	const std::vector<int> neigh = g.neighbors_cache(v);
	
	unsigned int numWhite = 1;

	for (unsigned int i = 0; i < neigh.size(); i++)
	{
		states[neigh[i]]==White && numWhite++;
	}
	return numWhite;
}

// find the vert with the most white neighbors
const inline unsigned int findGreatestSpan( const Graph& g, const STATES& states){
	int v ,highSpan, newSpan;
	v = highSpan = newSpan = - 1;
	
	for (unsigned int i = 0; i < states.size(); i++)
	{
		
		newSpan = numWhiteNeighbors(g, states,i);
		newSpan > highSpan && ( (highSpan = newSpan) && (v = i) );
		
	}
	return v;
}

// decide if the set is dominated yet
inline const bool isDominated(const STATES& states){
	for (unsigned int i = 0; i < states.size(); i++)
	{
		if (states[i]==White)
		{
			return false;
		}
	}
	return true;
}

// a loud version of ^^
int verboseIsDominated( STATES& states){
	for (unsigned int i = 0; i < states.size(); i++)
	{
		if (states[i] & White)
		{
			cout << i << " not dominated" << endl;
			return i;
		}
	}
	return -1;
}


// kick off the domination process
void setDominated(const Graph& g, STATES& states, const int v){
	
	std::vector<int> v_neigh = g.neighbors_cache(v);
	unsigned int i = 0;
	states[v] = Black;
	
	for ( i = 0 ; i < v_neigh.size() ; ++i)
	{
		states[ v_neigh[i] ] == White && ( states [ v_neigh[i] ] = Green );
	}
}

// print the DS
void printDS(Graph& graph, STATES& states){
	int sizeDS=0;
	for (unsigned int i = 0; i < states.size(); i++)
	{
		if (states[i]==Black)
		{
			sizeDS++;
			// cout << i << " " << graph.degree(i) << endl;
		}
	}
	cout<<"\nDominating Set size: "<<sizeDS<<"\n";
	return;
}

// size of the DS
const inline int ds_size (const STATES &states)
{
	unsigned int i = 0;
	unsigned int size = 0;
	for ( i = 0 ; i < states.size() ; ++i ) 
	{
		states[i] == Black && ++size;
	}
	return size;
}

// dump the states
const inline void print_set( const STATES &states ) 
{
	unsigned int i = 0;
	for ( i = 0 ; i < states.size() ; ++i )
	{
		if ( states[i] == Black )
		{
			cout << i << " ";
		}
	}
	cout << endl;
}

// read in the graph
void fire_read(Graph *  graph, const char * name) 
{
	ifstream infile(name);
	graph -> readGraph(infile);
}


// add all verts of deg 0 to the graph
void preprocess(Graph graph, STATES &states)
{
	for ( int i = 0 ; i < graph.numVertices() ; ++i ) 
	{
		graph.degree(i) == 0 && (states[i] = Black);
	}
}

/*
We clock 3 diff metrics:
	1. Read time
	2. time to create neighbors list
	3. time to solve for DS
*/

int main(int argc, char const *argv[])
{
	if(argc<2)
	{
		cerr<< "Please provide a graph file!"<<endl;
		exit(1);
	}

	
	Graph graph;
	clock_t start = clock();
	fire_read(&graph, argv[1]);

	cout << "Read done after " << (clock() - start ) / (double) CLOCKS_PER_SEC << "s" << endl;
	
	start = clock();
	
	

	STATES states(graph.numVertices(),White);

	cout << "initialized state vector:  " << (clock() - start ) / (double) CLOCKS_PER_SEC << "s" << endl;	
	
	start = clock();

	graph.fill_neighbor_list();

	cout << "filled neighbor list:  " << (clock() - start ) / (double) CLOCKS_PER_SEC << "s" << endl;
	
	start = clock();

	cout << "kicking off the search\n";
	
	
	preprocess(graph, states);
	
	int dom;
	long long iters = 0;
	while(!isDominated(states)){
		
		dom=findGreatestSpan(graph, states);
		setDominated(graph,states,dom);
		++iters;
		// if ( ++iters % 5000 == 0 ) {
		// 	cout << "On iter: " << iters << endl;
			
		// 	int have_left = verboseIsDominated(states);	
		// 	if (iters >= 500000) {
		// 		std::vector<int> n = graph.neighbors_cache(have_left);
				
		// 		cout << "V: " << have_left << " has " << n.size() << "neighbors\n";
				
		// 		for ( unsigned int i = 0 ; i < n.size() ; ++i ) {
		// 			cout << n[i] << " " << states[n[i]] << endl;
		// 		}
		// 		break;
		// 	}
		// }
	}
	
	cout << endl;
	cout << "Summary:" << endl;
	cout << "  Iterations: " << iters << endl;
	cout << "  Set Size: " << ds_size(states) << endl;
	cout << "  The Set: ";
	print_set( states );
	cout << "  Time: " << (clock() - start ) / (double) CLOCKS_PER_SEC << "s" << endl << endl;
	return 0;
}
