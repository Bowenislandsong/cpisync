//
// Created by Bowen on 10/3/18.
// This is for adjacency matrix
// Allow negative weights for directed graph
//
//

#ifndef CPISYNCLIB_ADJMTX_H
#define CPISYNCLIB_ADJMTX_H


#include <NTL/ZZ.h>
#include <vector>
#include <utility>
#include <string>
#include "Auxiliary.h"
#include "DataObject.h"
#include <map>

using std::vector;
using std::string;
using std::stringstream;
using std::pair;
using namespace NTL;

class AdjMtx {
public:
    // CONTRUCTOR AND DISTRUCTOR
    AdjMtx();

    ~AdjMtx();

    /**
     * append to vertex list
     * @param shingle <ZZ>
     * @return
     */
    bool addNewVex(string shingle);

    /**
     * Add a list of vertex at once
     * @param _ver list of shingles
     */
    void create(const vector<string> &_ver);

    /**
     * list of shingles
     * @return list of vertices
     */
    vector<string> getGraphVex() {
        vector<string> res;
        for (auto vex : vertex) res.push_back(ZZtoStr(vex));
        return res;
    }

    /**
     * number pf verticies in the graph
     * @return
     */
    size_t getNumVex() {
        return vertex.size();
    }

    void sortVex();

    /**
     * check if vex is one of the Vertices
     * @param vex
     * @return
     */
    bool contains(string vex) {
        return vertex_map.find(StrtoZZ(vex)) != vertex_map.end();
    }

    /**
     * print the graph of concerned vertex
     * @param print_vertex ONLY applicable to string input
     */

    void printGraph();

    // wrapper functions
    bool addWeigth(string vfrom, string vto, int add_weight = 1) {
        return addWeigth(StrtoZZ(vfrom), StrtoZZ(vto), add_weight);
    };

    bool setWeight(string vfrom, string vto, int set_weight){
        return setWeight(vfrom, vto,set_weight);
    };

    bool delWeigth(string vfrom, string vto, int del_weight = 1){
        return delWeigth(vfrom, vto, del_weight);
    };

    int getWeight(string vfrom, string vto){
        return getWeight(vfrom, vto);
    };

protected:
    bool addNewVex(ZZ shingle);

    void create(const vector<ZZ> &_ver);

    void printGraph(vector<ZZ> print_vertex);

    bool contains(ZZ vex) {
        return vertex_map.find(vex) != vertex_map.end();
    }

    /**
     * change the weight of graph, allow negative
     * @param vfrom
     * @param vto
     * @param del_weight
     * @return
     */
    bool addWeigth(ZZ vfrom, ZZ vto, int add_weight = 1);

    bool setWeight(ZZ vfrom, ZZ vto, int set_weight);

    bool delWeigth(ZZ vfrom, ZZ vto, int del_weight = 1);

    int getWeight(ZZ vfrom, ZZ vto);

private:
    map<pair<ZZ, ZZ>, int> graph;  // if two vex tex is not connected, it would not be in the graph.
    vector<ZZ> vertex;
    map<ZZ, bool> vertex_map;
};


#endif //CPISYNCLIB_ADJMTX_H
