#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <fstream>

#ifndef USE_CACHE 
#define USE_CACHE
#endif

class Graph
{

	unsigned int nVertices;
	unsigned int nEdges;
	unsigned int numActiveVertices;
	std::vector<std::vector<bool> > adjMat;
	std::vector<std::vector<double> > weightedMat;
	std::vector<int> degrees;
	std::vector<bool> active;
	std::vector<std::vector<int> > neighbor_list;

public:
	Graph();
	void readGraph(std::ifstream& );
	// void readWeighted(std::ifstream&);
	// void readGraph1Index(std::ifstream&);
	double density(void);
	int maxDegree(void);
	int minDegree(void);
	
	std::vector<int> neighbors(int) const;
	
	
	const std::vector<int> neighbors_cache(const int key) const {
		#ifdef USE_CACHE
		// std:: cout << "using cache\n";
		if ( neighbor_list.size() > 0 ) {
			return neighbor_list[key];
		} else {
			return (const std::vector<int>) neighbors(key);
		}
		#endif
		#ifndef USE_CACHE  
		return neighbors(key);
		#endif
	}

	
	// std::vector<int> closedNeighbors(int);
	// std::vector<int> wNeighbors(int);
	void addEdge(int,int);
	void deleteEdge(int,int);
	void deleteVertex(int);
	void deleteLowerDegreeVertices(int);
	void printGraph(void);
	void printWeighted(void);
	void fill_neighbor_list(void);
	~Graph();

	inline int numVertices(void){
		return nVertices;
	}

	inline int numEdges(void){
		return nEdges;
	}

	inline const bool edgeExists(int v1,int v2) const {
		if ( v1 < v2 ) 
		{
			return adjMat[v1][v2];
		}
		return adjMat[v2][v1];
	}

	inline bool isActive(int i) {
		return active[i];
	}

	inline void inactive(int i){
		active[i]=false;
	}

	inline bool hasWeight(int v1, int v2) {
		if ( v1 < v2 )
		{
			if(edgeWeight(v1,v2)!=0)
				return true;
			else
				return false;
		} else 
		{
			if(edgeWeight(v2,v1)!=0)
				return true;
			else
				return false;
		}
		
	}

	inline double edgeWeight(int v1, int v2){
		if ( v1 < v2 )
		{
			return weightedMat[v1][v2];
		}
		return weightedMat[v2][v1];
	}

	inline bool vertexExists(int v){
		return active[v];
	}

	inline int degree(int v){
		return degrees[v];
	}
	
};

#endif