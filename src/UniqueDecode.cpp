//
// Created by Bowen on 10/3/18.
//

#include "UniqueDecode.h"
#include "AdjMtx.h"

UniqueDecode::UniqueDecode(const size_t shingle_len, const char stop_word){
    shingleLen = shingle_len;
    stopWord = stop_word;
}

UniqueDecode::~UniqueDecode(){};

void UniqueDecode::injectStr(string &str) {
    str = string(1, stopWord) + str; // stop word is char, always size 1
    origStr = str; // TODO: Delete this if not necessary
    MergIndex.clear(); // make sure nothing is in it
    UDonline(str,MergIndex);

    cout<<"number of merge shingles: "<< MergIndex.size()<<endl;
    // we can test if it works here
    vector<string> shingle_set;
    std::map<string,bool> unique;
    for (int i = 0; i < str.size(); ++i) {
        string tmpstr = str.substr(i,shingleLen);
        if(unique[tmpstr]) {
            shingle_set.push_back(tmpstr);
            unique[tmpstr] = true;
        }
    }
    auto reconStr = reconstructDFS(shingle_set,MergIndex);
    cout<< reconStr <<endl;
    if (str ==reconStr)cout<<"we are done"<<endl;
}

void UniqueDecode::UDonline(const string &str, std::map<string,std::set<size_t>>& merg_idx) {
    // We use "Online algorithm for testing unique decodability"
    // The modification is that we merge everytime we hit false and redo assessment for that index
    // We create shingles one by one

    // For easy implementation, we follow UD paper to create O(n^2) whereas it can be promoted to O(n)

    // Sanity check
    if (str.empty()) throw invalid_argument("The input string is empty - UDonline");
    if (!merg_idx.empty()) throw invalid_argument("The input Merge Index Vector is not empty - UDonline");
    if (str[0] != stopWord) throw invalid_argument("No stop word found at the begining of the string - UDonline");

    std::map<string, pair<bool, bool>> isCycVis; //Init visited and cycle .first = isCycle .second = isVisited
    std::map<string, size_t> order_reference; // reference for location
    AdjMtx adjMatrix;
    vector<string> shingle_history;
    int p_fix_len = shingleLen-1;

    // nlog(n), n = string size
    adjMatrix.addNewVex(str.substr(0, p_fix_len));
    for (size_t i = p_fix_len; i < str.size(); ++i) { // go throw the string
            order_reference[str.substr(i-p_fix_len, shingleLen)] = 0; // put all possible shingle and arrange them later
        if (adjMatrix.addNewVex(str.substr(i,1))) { // create vertices
            isCycVis[str.substr(i,1)] = {false, false}; // init isCycle and isVisited
        }
    }

    size_t tmp_idx = 0; // arrange shingles lexicographic order
    for (auto it = order_reference.begin(); it != order_reference.end(); ++it) it->second = tmp_idx++;

    string head = str.substr(0, p_fix_len);
    isCycVis[head].second = true;
    shingle_history.push_back(head); // visit the head

    for (size_t j = p_fix_len; j < str.size()-1; ++j) {
        string cur = str.substr(j, 1);

        auto merg_it = MergIndex.find(str.substr(j-p_fix_len,shingleLen));
        while(merg_it != MergIndex.end() and
                merg_it->second.find(order_reference[str.substr(j-p_fix_len, shingleLen)]) != merg_it->second.end()) {
            j--;
            cur += str.substr(j, 1);
        }
        auto cur_it = isCycVis.find(cur);

        if (!cur_it->second.second)
            cur_it->second.second = true;
        else {
            if (adjMatrix.getWeight(shingle_history.back(), cur) ==
                0) {  // does edge w[i-1] -> w[i] not already exist in G?
                if (cur_it->second.first) {  //does w[i] already belongs to a cycle
                    // It is no longer a Uniquely decodable string, we hoping the last two shingles and assess this again
                    // merge with previous shingle
                    // revisit previous shingle
                    mergeNredo(str.substr(j - shingleLen, shingleLen+1), order_reference, shingle_history, j,
                               adjMatrix, isCycVis);
                    continue;
                } else { // creating a new cycle
                    isCycVis[cur].first = true;
                    for (auto hist_it = shingle_history.rbegin(); hist_it != shingle_history.rend(); ++hist_it) {
                        isCycVis[*hist_it].first = true; // label w[i] and all the nodes visited since the previous tp a cycle
                        if (*hist_it == cur)
                            break;
                    }
                }
            } else {
                // stepping along an existing cycle
                if (isCycVis[shingle_history[j]].first) j++;
            }
        }
auto a = adjMatrix.getInDegree(cur);
        if (adjMatrix.getInDegree(cur) > 1) {
            mergeNredo(str.substr(j-shingleLen, shingleLen+1), order_reference, shingle_history, j, adjMatrix,isCycVis);
            continue;
        }

        if (!adjMatrix.addWeigth(shingle_history.back(), cur, 1)) {
            throw invalid_argument("setWeight error, a vex does not exist");
        }
        shingle_history.push_back(cur);
        adjMatrix.printGraph();
    }
}

void UniqueDecode::mergeNredo(const string cur_mg,map<string, size_t>& order_reference,vector<string>& shingle_history, size_t& j, AdjMtx& adjMatrix, map<string, pair<bool, bool>>& isCycVis){
    MergIndex[cur_mg.substr(shingleLen-1)].insert(order_reference[cur_mg.substr(0,shingleLen)]);
    auto newVex = shingle_history.back()+cur_mg.substr(shingleLen);
    if(adjMatrix.addNewVex(newVex))
        isCycVis[newVex] = {false, false};
    j -= shingle_history.back().size();
    if(!shingle_history.empty())shingle_history.pop_back();
}

string UniqueDecode::reconstructDFS(vector<string> &shingle_set, map<string,std::set<size_t>>& merg_idx) {
    // Sanity Checks
    if (shingle_set.empty()) return "";
    // if merg_idx is empty then we have no merges

    // Shingle set has all unique values
    // sort shingle_set
    // merge shingles
    string str;

    return str;
}

bool UniqueDecode::isAllVisited(vector<pair<string,bool>> isVisited_pair){
    for (auto it = isVisited_pair.begin(); it!=isVisited_pair.end();++it){
        if (!it->second){
            return false;
        }
    }
    return true;
}

vector<ZZ> UniqueDecode::getShingleSet(const string str){
    vector<ZZ> res;
    for (int i = 0; i < str.size(); ++i) {
        ZZ shingle = StrtoZZ(str.substr(i, shingleLen));
        auto it = find(res.begin(),res.end(),shingle);
        if (it == res.end()) { // not exist in the res set
            res.push_back(shingle);
        }
    }
    return res;
}
