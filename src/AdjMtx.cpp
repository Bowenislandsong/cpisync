//
// Created by Bowen on 10/3/18.
//

#include "AdjMtx.h"

AdjMtx::AdjMtx() = default;
AdjMtx::~AdjMtx() = default;

void AdjMtx::create(const vector<ZZ>& _ver) {
    for (auto vex : _ver) {
        addNewVex(vex);
    }
}

void AdjMtx::create(const vector<string>& _ver) {
    for (auto vex : _ver) {
        addNewVex(StrtoZZ(vex));
    }
}

bool AdjMtx::addNewVex(ZZ shingle) {
    if(vertex_map.find(shingle) != vertex_map.end())
        return false;

    vertex.push_back(shingle);
    vertex_map[shingle] = true;
    return true;
}

bool AdjMtx::addNewVex(string shingle) {
//    auto v = new DataObject(shingle);
//    return addNewVex(v->to_ZZ());
    return addNewVex(StrtoZZ(shingle));
}

bool AdjMtx::addWeigth(ZZ vfrom, ZZ vto, int add_weight){
    if (contains(vto) and contains(vfrom)) {
        pair<ZZ, ZZ> vexpair{vfrom, vto};
        auto temp = graph.find(vexpair);
        if (temp != graph.end()) {
            temp->second += add_weight;
        }else{
            graph.insert(make_pair(vexpair,add_weight));
        }
        return true;
    }
    return false;
}

void AdjMtx::sortVex(){
    sort(vertex.begin(),vertex.end());
}

bool AdjMtx::setWeight(ZZ vfrom, ZZ vto, int set_weight){
    if (contains(vto) and contains(vfrom)) {
        pair<ZZ, ZZ> vexpair{vfrom, vto};
        auto temp = graph.find(vexpair);
        if (temp != graph.end()) {
            temp->second = set_weight;
        } else{
            graph.insert(make_pair(vexpair,set_weight));
        }
        return true;
    }
    return false;
}

bool AdjMtx::delWeigth(ZZ vfrom, ZZ vto, int del_weight){
    if (contains(vto) and contains(vfrom)) {
        pair<ZZ, ZZ> vexpair{vfrom, vto};
        auto temp = graph.find(vexpair);
        if (temp != graph.end()) {
            temp->second -= del_weight;
        } else{
            graph.insert(make_pair(vexpair,-del_weight));
        }
        return true;
    }
    return false;
}


void AdjMtx::printGraph(vector<ZZ> print_vertex){
    cout << "   ";
    for (auto vex: print_vertex) {
        cout << ZZtoStr(vex)<< "|";
    }
    cout << "\n";
    for (auto vexi: print_vertex){
        cout << ZZtoStr(vexi)<< "|";
        for (auto vexj: print_vertex){
            cout << getWeight(vexi,vexj)<< "|";
        }
        cout << "\n";
    }
}

void AdjMtx::printGraph(){
    printGraph(vertex);
}

int AdjMtx::getWeight(ZZ vfrom, ZZ vto) {
    pair<ZZ,ZZ> vexpair{vfrom,vto};
    auto temp = graph.find(vexpair);
    if (temp != graph.end()){
        return temp->second;
    }
    return 0;
}