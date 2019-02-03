//
// Created by Bowen on 10/3/18.
//

#include "UniqueDecode.h"
#include "AdjMtx.h"

UniqueDecode::UniqueDecode(const size_t shingle_len, const char stop_word){
    shingleLen = shingle_len;
    stopWord = stop_word;
}

UniqueDecode::~UniqueDecode(){

};

void UniqueDecode::injectStr(string &str) {
    str = string(1, stopWord) + str; // stop word is char, always size 1
    origStr = str; // TODO: Delete this if not necessary
    MergIndex.clear(); // make sure nothing is in it
    UDonline(str,MergIndex);

    cout<< MergIndex.size()<<endl;
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

void UniqueDecode::UDonline(const string &str, std::map<string,vector<size_t>>& merg_idx) {
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
    for (size_t i = 0; i < str.size() - shingleLen+1; ++i) { // go throw the string
        string tmpstr = str.substr(i, shingleLen-1);
        if (adjMatrix.addNewVex(tmpstr)) { // create vertices
            order_reference[tmpstr] = 0; // put all possible shingle and arrange them later
            isCycVis[tmpstr] = {false, false}; // initi isCycle and isVisited
        }
    }

    size_t tmp_idx = 0; // shingles lexicographic order
    for (auto it = order_reference.begin(); it != order_reference.end(); ++it) it->second = tmp_idx++;

    string head = str.substr(0, shingleLen-1);
    isCycVis[head].second = true;
    shingle_history.push_back(head); // visit the head

    for (size_t j = 1; j < str.size()- shingleLen+1; ++j) {
        string cur = str.substr(j, shingleLen-1);
        auto cur_it = isCycVis.find(cur);

        if (!cur_it->second.second)
            cur_it->second.second = true;
        else {
            if (adjMatrix.getWeight(shingle_history.back(), cur) == 0) {  // does edge w[i-1] -> w[i] not already exist in G?
                if (cur_it->second.first) {  //does w[i] already belongs to a cycle
                    // It is no longer a Uniquely decodable string, we koping the last two shingles and assess this again
                    // merge with previous shingle
                    // revisit previous shingle
                    mergeNredo(cur, order_reference, shingle_history, j, adjMatrix);
                    continue;
                } else { // creating a new cycle
                    isCycVis[cur].first = true;
                    for (auto hist_it = shingle_history.rbegin(); hist_it != shingle_history.rend(); ++hist_it) {
                        string a = *hist_it;

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

        // this is a N^2 time or high communicaiton cost
        // unless the number of vertices is small, aka a lot more eularian cycles, then bad communicaiton cost
        // on the other hand, N as big as string size then we have N^2
        bool distinct = true; // init distinct
        for (auto tmp_shingle : order_reference) {
            if (adjMatrix.getWeight(tmp_shingle.first, cur) == 1 and distinct) {
                distinct = false;
            } else if (adjMatrix.getWeight(tmp_shingle.first, cur) == 1 and not distinct) {
                mergeNredo(cur, order_reference, shingle_history, j, adjMatrix);
                continue;
            }
        }


        if (!adjMatrix.setWeight(shingle_history.back(), cur, 1)) {
            throw invalid_argument("setWeight error, a vex does not exist");
        }
        shingle_history.push_back(cur);
    }
}

void UniqueDecode::mergeNredo(const string cur,std::map<string, size_t>& order_reference,vector<string>& shingle_history, size_t& j, AdjMtx& adjMatrix){
    MergIndex[cur].push_back(order_reference[shingle_history.back()]);
    shingle_history.back() += cur.substr(shingleLen-1);
    adjMatrix.addNewVex(shingle_history.back());
    j -= shingle_history.back().size() - shingleLen + 1;
}



string UniqueDecode::reconstructDFS(vector<string> &shingle_set, std::map<string,vector<size_t>>& merg_idx) {
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
