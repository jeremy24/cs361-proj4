
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <map>
#include <omp.h>

#include "Graph.h"

using namespace std;




Graph::Graph(void){
	//cout<<"Creating graph object"<<endl;
	return;
}





// void Graph::readWeighted(ifstream& infile){
// 	int v1,v2;
// 	double weight;
// 	string line,temp;
// 	stringstream ss;
// 	int ncols=0;

// 	getline(infile,line);

// 	getline(infile,line);
// 	ss.clear();
// 	ss<<line;
// 	while(ss>>temp){
// 		ncols++;
// 	}

// 	infile.clear();
// 	infile.seekg(0,ios::beg);


// 	infile>>nVertices>>nEdges;
// 	numActiveVertices=nVertices;
// 	std::vector<double> bvector(nVertices,0);
// 	for (unsigned int i = 0; i < nVertices; i++)
// 	{
// 		weightedMat.push_back(bvector);
// 		degrees.push_back(0);
// 		active.push_back(true);
// 	}
	

// 	if(ncols<3)
// 	{
// 		while(infile>>v1>>v2){
// 		weightedMat[v1][v2]=1;
// 		weightedMat[v2][v1]=1;
// 		degrees[v1]++;
// 		degrees[v2]++;
// 	}
// 	}
// 	else{
// 		while(infile>>v1>>v2>>weight){
// 		weightedMat[v1][v2]=weight;
// 		weightedMat[v2][v1]=weight;
// 		degrees[v1]++;
// 		degrees[v2]++;
// 		}
// 	}

// 	return;
// }

void Graph::readGraph(ifstream& infile){
	int v1,v2;
	infile>>nVertices>>nEdges;
	numActiveVertices=nVertices;
	std::vector<bool> bvector(nVertices,false);
	
	
	
	for (unsigned int i = 0; i < nVertices; i++)
	{
		adjMat.push_back(bvector);
		adjMat[i].resize(i+1);
		degrees.push_back(0);
		active.push_back(true);
	}

	while(infile>>v1>>v2){
		if ( v1 < v2 )
		{
			adjMat[v1][v2]=true;
		} else
		{
			adjMat[v2][v1]=true;
		
		}
		adjMat[v2][v1]=true;
		degrees[v1]++;
		degrees[v2]++;
	}

	
	return;
}



// void Graph::readGraph1Index(ifstream& infile){
// 	int v1,v2;
// 	infile>>nVertices>>nEdges;
// 	numActiveVertices=nVertices;
// 	std::vector<bool> bvector(nVertices,false);
// 	for (unsigned int i = 0; i < nVertices; i++)
// 	{
// 		adjMat.push_back(bvector);
// 		degrees.push_back(0);
// 		active.push_back(true);
// 	}

// 	while(infile>>v1>>v2){
// 		v1--;v2--;
// 		adjMat[v1][v2]=true;
// 		adjMat[v2][v1]=true;
// 		degrees[v1]++;
// 		degrees[v2]++;
// 	}

	
// 	return;
// }



double Graph::density(void){
	double V=nVertices;
	double E=nEdges;
	return (E/((V*(V-1))/2.0));
}

int Graph::maxDegree(void){
	int maxDegree=0;
	for (unsigned int i = 0; i < nVertices; i++)
	{
		if(degrees[i]>maxDegree)
			maxDegree=degrees[i];
	}
	return maxDegree;
}

int Graph::minDegree(void){
	int minDegree;
	minDegree=nVertices-1;
	for (unsigned int i = 0; i < nVertices; i++)
	{
		
		if(degrees[i]<minDegree)
			minDegree=degrees[i];
	}
	return minDegree;
}







std::vector<int> Graph::neighbors(int v) const{
	std::vector<int> array;
	for (unsigned int i = 0; i < nVertices; i++)
	{
		if(edgeExists(v,i))
			array.push_back(i);
	}
	return array;
}

// std::vector<int> Graph::closedNeighbors(int v){
// 	std::vector<int> array;
// 	array.push_back(v);
// 	for (unsigned int i = 0; i < nVertices; i++)
// 	{
// 		if(edgeExists(v,i))
// 			if (active[i])
// 			array.push_back(i);
// 	}
// 	return array;
// }

// std::vector<int> Graph::wNeighbors(int v){
// 	std::vector<int> array;
	
// 	for (unsigned int i = 0; i < nVertices; i++)
// 	{
// 		if(hasWeight(v,i))
// 			if (active[i])
// 			array.push_back(i);
// 	}
// 	return array;
// }

void Graph::addEdge(int v1,int v2){
	
	if ( v1 < v2 )
	{
		adjMat[v1][v2] = true;
	} else 
	{
		adjMat[v2][v1] = true;
	}
	// adjMat[v2][v1]=true;
	++degrees[v1];
	++degrees[v2];
	++nEdges;
	return;
}

void Graph::deleteEdge(int v1,int v2){
	
	if ( v1 < v2 )
	{
		adjMat[v1][v2]=false;
	} else 
	{
		adjMat[v2][v1]=false;
	}
	// adjMat[v2][v1]=true;
	--degrees[v1];
	--degrees[v2];
	--nEdges;
	return;
}

void Graph::deleteVertex(int v){
	
	std::vector<int> array;
	array=neighbors(v);
	active[v]=false;
	for (unsigned int i = 0; i < array.size(); i++)
	{
		deleteEdge(v,array[i]);
	}

	return;
}

void Graph::deleteLowerDegreeVertices(int k){
	for (unsigned int i = 0; i < nVertices; i++)
	{
		if(vertexExists(i))
			if (degree(i)<k)
				deleteVertex(i);
	}
}

void Graph::printGraph(void){
	for (unsigned int i = 0; i < nVertices; i++)
	{
		for (unsigned int j = 0; j < nVertices; j++)
		{
			if ( i < j )
			{
				cout << adjMat[i][j] << " ";
			} else
			{
				cout << "  ";
			}
			// cout << "(" << i << ", " << j << ", " << adjMat[i].size() << ", " << adjMat[i][j] << ") ";
		}
		cout<< endl;
	}
	return;
}


void Graph::fill_neighbor_list(void) {
	neighbor_list.resize(nVertices);
	
	
	// for ( unsigned int i = 0 ; i < nVertices ; ++i )
	// {
	// 	for ( unsigned int j = 0 ; j < nVertices ; ++j )
	// 	{
	// 		if ( edgeExists(i,j) )
	// 		{
	// 			neighbor_list[i].push_back(j);
	// 		}
	// 	}
	// }
	
	for ( unsigned int i = 0 ; i < nVertices ; ++i ) 
	{
		neighbor_list[i] = neighbors(i);
		// cout << i << " " << neighbor_list[i].size() << " " << adjMat[i].size() << endl;
	}
}

void Graph::printWeighted(void){
	for (unsigned int i = 0; i < nVertices; i++)
	{
		for (unsigned int j = 0; j < nVertices; j++)
		{
			cout<< weightedMat[i][j]<<" ";
		}
		cout<< endl;
	}
	return;
}

Graph::~Graph(void){
	//cout<<"deleting graph"<<endl;
	return;
}
