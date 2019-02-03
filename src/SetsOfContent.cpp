//
// Created by Bowen Song on 12/9/18.
//

#include "SetsOfContent.h"
SetsOfContent::SetsOfContent(size_t terminal_str_size, size_t levels, size_t partition, GenSync::SyncProtocol base_set_proto, size_t shingle_size)
        : TermStrSize(terminal_str_size), Levels(levels), Partition(partition), baseSyncProtocol(base_set_proto), HashShingleSize(shingle_size) {
    SyncID = SYNC_TYPE::SetsOfContent;
    if (levels > USHRT_MAX or levels < 2)
        throw invalid_argument("Num of Level specified should be between 2 and " + to_string(USHRT_MAX));

//    initResources(initRes);
}

SetsOfContent::~SetsOfContent() {
    for (DataObject* dop : setPointers) delete dop;
}

vector<size_t> SetsOfContent::create_HashSet(string str,size_t win_size, size_t space, size_t shingle_size) {
    // space and shingle_size has to match between reconciling strings
    // Time Complexity 2n, where n is the string size.
    vector<size_t> hash_val, hash_set;

    // if the substring is smaller than the terminal string size, we do not partition it anymore.
    if (str.size() <= TermStrSize) {
        hash_set = {str_to_hash(str)};
    } else { // else we partitions it


        for (size_t i = 0; i < str.size() - shingle_size + 1; ++i) {
            std::hash<std::string> shash;
            hash_val.push_back(shash(str.substr(i, shingle_size)) % space);
        }
        size_t prev = 0;

        for (size_t min:local_mins(hash_val, win_size)) {
            hash_set.push_back(add_to_dictionary(str.substr(prev, min - prev)));
            prev = min;
        }


        hash_set.push_back(add_to_dictionary(str.substr(prev)));
    }
    // write it to cyc-dict
    auto cyc_it = Cyc_dict.find(str_to_hash(str));
    if (cyc_it == Cyc_dict.end()) { // check if cyc exists
        Cyc_dict[str_to_hash(str)] = hash_set; // update Cyc_dict
    }
    else if(cyc_it->second != hash_set) // check if it is getting overwritten
        throw invalid_argument("More than one answer is possible for Cyc_dict");

    return hash_set;
}

vector<size_t> SetsOfContent::local_mins(vector<size_t> hash_val, size_t win_size) {
    // relying on hashMap sorting

    // minimum partition distance
    if (win_size < 1) {
        cout
                << "Content Partition window size is less than 1 and adjusted to 1. Consider adjusting number of partition levels"
                << endl;
        win_size = 1;
    }
    vector<size_t> mins;
    map<size_t, size_t> hash_occurr;
    for (size_t j = 0; j < 2 * win_size; ++j) {
        auto it = hash_occurr.find(hash_val[j]);
        if (it != hash_occurr.end())
            it->second++;
        else
            hash_occurr[hash_val[j]] = 1;
    }

    for (size_t i = win_size+1; i < hash_val.size() - win_size+1; ++i) {
        if (hash_val[i-1] <= hash_occurr.begin()->first and i - ((!mins.empty())?mins.back():0) > win_size)
            mins.push_back(i-1);
        auto it_prev = hash_occurr.find(hash_val[i - win_size - 1]);
        if (it_prev != hash_occurr.end())
            it_prev->second--;

        auto it_pos = hash_occurr.find(hash_val[i+win_size]);
        if (it_pos != hash_occurr.end())
            it_pos->second++;
        else
            hash_occurr[hash_val[i+win_size]] = 1;
    }
    return mins;
}

size_t SetsOfContent::add_to_dictionary(const string& str) {
    (Dictionary.find(str_to_hash(str)) == Dictionary.end() or Dictionary[str_to_hash(str)] == str) ? Dictionary[str_to_hash(str)] = str : throw invalid_argument(
            "Dictionary duplicated suggest using new/multiple hash functions");
    return str_to_hash(str);
}

void SetsOfContent::go_through_tree() {
    myTree.clear(); // should be redundant

    auto String_Size = pow(Partition, Levels) *
                       TermStrSize; // calculate a supposed string size, a string size that make sense with the parameters

    if (String_Size < 1)
        throw invalid_argument(
                "fxn go_through_tree - parameters do not make sense - num of par: " + to_string(Partition) + ", num of lvls: " +
                to_string(Levels) + ", Terminal String Size: " + to_string(TermStrSize) + ", Actual String Size: " +
                to_string(myString.size()));

    size_t shingle_size = floor(log2(String_Size));
    if (shingle_size < 2)
        throw invalid_argument("Consider larger the parameters for auto shingle size to be more than 2");
    size_t space = TermStrSize * 126; //126 for ascii content
    vector<size_t> cur_level;
    // fill up the tree
    myTree.resize(Levels);

//    clock_t lvl1_t = clock();
    // put up the first level
    update_tree_shingles({add_to_dictionary(myString)}, 0);


//    cout << "Lvl 1 time with BackTracking: " << (double) (clock() - lvl1_t) / CLOCKS_PER_SEC << endl;

//    clock_t all_lvls_t = clock();
    for (int l = 1; l < Levels; ++l) {


        for (auto substr_hash:unique_substr_hash(myTree[l - 1])) {
            string substring = Dictionary[substr_hash];
            if (substring.empty()) continue; // this ditches the empty strings
            cur_level = create_HashSet(substring, floor((substring.size() / Partition) / 2), space, shingle_size);
            update_tree_shingles(cur_level, l);
        }

        // update lost level's cycle since it is done with cyc dict
        for (const shingle_hash& shingle : myTree[l - 1]){
            auto it = Cyc_dict.find(shingle.second);
            if (it == Cyc_dict.end()) throw invalid_argument("Cycle not found, should have been inserted");
            const_cast<cycle&> (shingle.compose) = cycle{.cyc = 0, .head = it->second.front(), .len = it->second.size()};
        }
        space = floor((space / Partition) / 2);
        shingle_size = floor(shingle_size / 2);
    }
//    for(auto lvl : myTree) for (auto shingle : lvl) cout<< shingle<<endl; // TODO: delete this print tree function
//    cout<<myTree[0].size()<<":"<<myTree[1].size()<<":"<<myTree[2].size()<<endl;
//    cout << "Rest of the Lvls time with BackTracking: " << (double) (clock() - all_lvls_t) / CLOCKS_PER_SEC << endl;
}



// what i am missing, and what they would be sending to me
void SetsOfContent::prepare_querys(const vector<shingle_hash> & shingle_hash_theirs,const vector<shingle_hash>& shingle_hash_mine) {
// get the tree ready
// know what to expect by getting term_query and cyc_query ready
//    map<size_t, vector<shingle_hash>> mine_rid;
//    theirTree.resize(myTree.size());// resize their tree
//    map<shingle_hash,bool> rid_shingles;

    term_query.clear();// should be empty anyway
    cyc_query.clear();

    // string recon needs to get rid of what is not on their tree. its sync not recon
//    for(shingle_hash shingle : shingle_hash_mine){
//        rid_shingles[shingle] = true;
//    }

    // fill up cyc_quey and term_query , concner and query should be identical. We assume map auto sorts the items (hash values)
    for (shingle_hash shingle:shingle_hash_theirs) { // put what i learnt from set recon in their tree

//        theirTree[shingle.lvl].insert(shingle);

        if (Dictionary.find(shingle.second) == Dictionary.end()) {
            if (shingle.lvl < Levels - 1)
                cyc_query[shingle.second] = 0;
            else
                term_query[shingle.second] = "";
        }
    }
//    for(auto lvl : theirTree) for (auto shingle : lvl) cout<< shingle<<endl; // TODO: delete this print tree function


    // move the rest of the tree.
//    for (auto tree_lvl : myTree) { // if it is the same ID, put the shingle in their tree
//        for (shingle_hash shingle: tree_lvl) {
//            if (!rid_shingles[shingle]) {
//                theirTree[shingle.lvl].insert(shingle);
//            }
//        }
//    }

}

// what they are missing, and i will prepare to send them
//void SetsOfContent::prepare_concerns(const vector<shingle_hash> &shingle_hash_theirs, const vector<shingle_hash> &shingle_hash_mine) {
//    // figure out what they are missing, on cycles and dictionary
//    // track trough to find cycle number
//    // prepare
//    // later send the information over
//
//    // get all unique hashes that is konw to their host
//
//    term_concern.clear();
//    cyc_concern.clear();
//
//    map<size_t,bool> hashes_they_know; // seconds of (myTree - my unique + their hashes)
//
//    map<size_t,bool> my_unique;
//
//    for(auto shingle:shingle_hash_mine){
//        my_unique[shingle.second] = true;
//    }
//
//    for (auto lvl : myTree) {
//        for(auto shingle : lvl){
//            if(!my_unique[shingle.second]) hashes_they_know[shingle.second] = true;
//        }
//    }
//
//    for (auto shingle : shingle_hash_theirs) hashes_they_know[shingle.second] = true;
//
//
//
//    for (shingle_hash shingle : shingle_hash_mine) {
//        if(hashes_they_know[shingle.second]) continue;
//
//        if (myTree.size() - 1 == shingle.lvl) {
//            term_concern[shingle.second] = Dictionary[shingle.second];
////        }else {
////            cycle tmp = shingle.compose;
////            if (!shingle2hash_train(tmp,myTree[shingle.lvl+1],Cyc_dict[shingle.second])) throw invalid_argument("We failed to get a cycle number to send to an other party at lvl: "+to_string(shingle.lvl));
////            cyc_concern[shingle.second] = tmp.cyc;
//        }
//    }
//}

void SetsOfContent::answer_queries(const map<size_t,bool> &queries, const vector<shingle_hash> &shingle_hash_mine) {
    cyc_concern.clear();

    for (shingle_hash shingle : shingle_hash_mine) {
        auto it = queries.find(shingle.second);
        if (it == queries.end() or !it->second) continue;

        if (myTree.size() - 1 == shingle.lvl) {
            term_concern[shingle.second] = Dictionary[shingle.second];
        } else {
            cycle tmp = shingle.compose;

            if (!shingle2hash_train(tmp, myTree[shingle.lvl + 1], Cyc_dict[shingle.second]))
                cout << "We failed to get a cycle number to send to an other party at lvl: " << shingle.lvl << endl;

            cyc_concern[shingle.second] = tmp.cyc;
        }
    }
}

void SetsOfContent::update_tree_shingles(vector<size_t> hash_vector, sm_i level) {
    if (myTree.size() <= level) throw invalid_argument("We have exceeded the levels of the tree");
    if (hash_vector.size() > 100)
        cout << "It is advised to not exceed 100 partitions for fast backtracking at Level: " + to_string(level) +
                " Current set size: " + to_string(hash_vector.size()) << endl;
    if (hash_vector.empty())
        throw invalid_argument("hash_vector is zero at level:" + to_string(level) +
                               ". All terminal strings to be passed down to the bottom level");

    map<pair<size_t, size_t>, size_t> tmp;

    // make shingles including a number of start shingles size of shingle size - 1
    for (int i = 0; i < hash_vector.size(); ++i) {
        size_t shingle;
        (i > 0) ? shingle = hash_vector[i - 1] : shingle = 0;

        if (tmp.find({shingle, hash_vector[i]}) != tmp.end())
            tmp[{shingle, hash_vector[i]}]++;
        else
            tmp[{shingle, hash_vector[i]}] = 1;
    }
    if (tmp.empty())
        throw invalid_argument(
                "update_tree shingle is empty");// TODO: Delete it after making sure thi is never triggered.

    for (auto item = tmp.begin(); item != tmp.end(); ++item) {
        myTree[level].insert(
                shingle_hash{.first = item->first.first, .second = item->first.second, .occurr = item->second,
                        .compose = cycle{.cyc=0, .head = 0, .len = 0},
                        .lvl = level});
    }

}

// Backtracking using front ab dn back and cycle number
//// functions for backtracking
//bool SetsOfContent::shingle2hash_train(cycle& cyc_info, set<shingle_hash>& shingle_set, vector<size_t>& final_str) {
//
////    // edge case if there is only one shinlge in the set
////    if (shingle_set.empty()) throw invalid_argument("Nothing is passed into shingle2hash_train");
////
////    if (shingle_set.size() == 1) {// edge case of just one shingle
////        if (cyc_info.cyc == 0) {// we find cycle number
////            cyc_info.cyc = 1;
////            cout<<"shingle2hash_train, i should never happend"<<endl; // delete if proven tobe useless
////            return true;
////        } else { // we find string
////            return true;
////        }
////    }
//    auto changed_shingle_set = tree2shingle_dict(shingle_set); // get a shingle dict from a level of a tree for fast next edge lookup
//
//    if (changed_shingle_set.empty()) throw invalid_argument("the shingle_vec provided is empty for shingle2hash_train");
//
//    vector<map<size_t, vector<shingle_hash>>> stateStack;
//    vector<vector<shingle_hash>> nxtEdgeStack;
//    stateStack.push_back(changed_shingle_set);// Init Original state
//    size_t strCollect_size = 0, curEdge =0;
//    vector<size_t> str; // temprary string hash train to last be compared/placed in final_str
//
//
//    for (auto head_shingles : changed_shingle_set[(size_t)0]) {
//        if (cyc_info.head == head_shingles.second) {
//            str.push_back(head_shingles.second);
//            curEdge = head_shingles.second;
//            break;
//        }
//    }
//
//
//    if (cyc_info.cyc == 0) { // find head from "final_str" (we are finding cycle number)
//        //if we only have one, then we are done with cycle number one and head ==tail
//        if (final_str.size() == 1 && cyc_info.head == cyc_info.tail) {
//            cyc_info.cyc = 1;
//            final_str = str;
//            return true;
//        }
//    } else if (cyc_info.cyc > 0) {// find head from "final_str" (we are retrieving the string from cycle number)
//        if (cyc_info.cyc == 1 && cyc_info.head == cyc_info.tail) {
//            final_str = str;
//            return true;
//        }
//    }
//    //    Resources initRes;
////    initResources(initRes); // initiate Recourses tracking
//
//
//    shingle_hash last_edge;
//
//    while (!stateStack.empty() and stateStack.size() == nxtEdgeStack.size() + 1) { // while state stack is not empty
//        vector<shingle_hash> nxtEdges = stateStack.back()[curEdge];
//
//        if (!nxtEdges.empty()) { // If we can go further with this route
//            last_edge = get_nxt_edge(curEdge, nxtEdges.back());
//            nxtEdges.pop_back();
//            nxtEdgeStack.push_back(nxtEdges);
//        } else if (!nxtEdgeStack.empty() and stateStack.size() == nxtEdgeStack.size() + 1 and
//                   !nxtEdgeStack.back().empty()) { //if this route is dead, we should look for other options
//            if (!str.empty()) str.pop_back();
//
//            //look for other edge options
//            last_edge = get_nxt_edge(curEdge, nxtEdgeStack.back().back());
//            nxtEdgeStack.back().pop_back();
//
//            stateStack.pop_back();
//            //(!stateStack.empty()) ? stateStack.push_back(stateStack.back()) : stateStack.push_back(origiState);
//        } else if (!stateStack.empty() and stateStack.size() == nxtEdgeStack.size() + 1 and !nxtEdgeStack.empty() and
//                   nxtEdgeStack.back().empty()) {// if this state is dead and we should look back a state
//            if (!str.empty()) str.pop_back();
//            // look back a state or multiple if we have empty nxt choice (unique nxt edge)
//            while (!nxtEdgeStack.empty() and nxtEdgeStack.back().empty()) {
//                nxtEdgeStack.pop_back();
//                stateStack.pop_back();
//                if (!str.empty()) str.pop_back();
//            }
//            if (nxtEdgeStack.empty()) {
//                return false;
//            } else if (!nxtEdgeStack.back().empty()) {
//                last_edge = get_nxt_edge(curEdge, nxtEdgeStack.back().back());
//                nxtEdgeStack.back().pop_back();
//                stateStack.pop_back();
//            }
//        } else if (stateStack.size() != nxtEdgeStack.size() + 1) {
//            throw invalid_argument("state stack and nxtEdge Stack size miss match" + to_string(stateStack.size())
//                                   + ":" + to_string(nxtEdgeStack.size()));
//        }
//
//        str.push_back(curEdge);
//
//        // Change and register our state for shingle occurrence and nxt edges
//        stateStack.push_back(stateStack.back());
//        for (auto &tmp_shingle: stateStack.back()[last_edge.first]) {
//            if (tmp_shingle == last_edge) {
//                tmp_shingle.occurr--;
//                break;
//            }
//        }
//
//
////
////        if (!resourceMonitor( initRes, MAX_TIME, MAX_VM_SIZE))
////            return false;
//
//        // if we reached a stop point
//        if (curEdge == cyc_info.tail) {
//            strCollect_size++;
//            if (str == final_str || (strCollect_size == cyc_info.cyc and cyc_info.cyc != 0)) {
//                cyc_info.cyc = strCollect_size;
//                final_str = str;
//            }
//        }
//
//        if (strCollect_size == cyc_info.cyc && cyc_info.cyc != 0) {
//            return true;
//        }
//    }
//    return false;
//}

//bool SetsOfContent::empty_state(vector<shingle_hash> state) {
//    for (shingle_hash item : state) {
//        if (item.occurr > 0) return false;
//    }
//    return true;
//}

vector<shingle_hash> SetsOfContent::get_nxt_shingle_vec(const size_t cur_edge,
                                                        const map<size_t, vector<shingle_hash>> &last_state_stack,
                                                        const map<size_t, vector<shingle_hash>> &original_state_stack) {
    vector<shingle_hash> res_vec;

    auto from_stateStack = last_state_stack.find(cur_edge);

    if (from_stateStack != last_state_stack.end()) {
        for (auto tmp_shingle: from_stateStack->second) {
            if (tmp_shingle.occurr > 0) res_vec.push_back(tmp_shingle);
        }
    } else {
        auto cur_it = original_state_stack.find(cur_edge);
        if (cur_it == original_state_stack.end())return res_vec; // there is no possible edge(no edge after this)
        for (auto tmp_shingle: original_state_stack.find(cur_edge)->second) {
            if (tmp_shingle.occurr > 0) res_vec.push_back(tmp_shingle);
        }
    }

    return res_vec;
}

// functions for backtracking using front, length, and cycle number
bool SetsOfContent::shingle2hash_train(cycle& cyc_info, set<shingle_hash>& shingle_set, vector<size_t>& final_str) {

    map<size_t, vector<shingle_hash>> original_state_stack = tree2shingle_dict(
            shingle_set); // get a shingle dict from a level of a tree for fast next edge lookup

    if (shingle_set.empty())
        throw invalid_argument(
                "the shingle_set provided is empty for shingle2hash_train, we accessed a tree level with no shingle.");

    vector<map<size_t, vector<shingle_hash>>> stateStack;
    vector<vector<shingle_hash>> nxtEdgeStack;
    stateStack.push_back(
            map<size_t, vector<shingle_hash>>());// Init Original state - nothing changed, state stack only records what is changed
    size_t strCollect_size = 0, curEdge = 0;
    vector<size_t> str; // temprary string hash train to last be compared/placed in final_str


    for (auto head_shingles : original_state_stack[(size_t) 0]) {
        if (cyc_info.head == head_shingles.second) {
            str.push_back(head_shingles.second);
            curEdge = head_shingles.second;
            break;
        }
    }


    if (cyc_info.cyc == 0) { // find head from "final_str" (we are finding cycle number)
        //if we only have one, then we are done
        if (final_str.size() == 1 && cyc_info.len == 1) {
            cyc_info.cyc = 1;
            final_str = str;
            return true;
        }
    } else if (cyc_info.cyc > 0) {// find head from "final_str" (we are retrieving the string from cycle number)
        if (cyc_info.cyc == 1 && cyc_info.len == 1) {
            final_str = str;
            return true;
        }
    }
    //    Resources initRes;
//    initResources(initRes); // initiate Recourses tracking


    shingle_hash last_edge;

    while (!stateStack.empty() and stateStack.size() == nxtEdgeStack.size() + 1) { // while state stack is not empty
        vector<shingle_hash> nxtEdges = get_nxt_shingle_vec(curEdge,stateStack.back(),original_state_stack);

        if (!nxtEdges.empty() and str.size() < cyc_info.len) { // If we can go further with this route
            last_edge = get_nxt_edge(curEdge, nxtEdges.back());
            nxtEdges.pop_back();
            nxtEdgeStack.push_back(nxtEdges);
        } else if (!nxtEdgeStack.empty() and stateStack.size() == nxtEdgeStack.size() + 1 and
                   !nxtEdgeStack.back().empty()) { //if this route is dead, we should look for other options
            if (!str.empty()) str.pop_back();

            //look for other edge options
            last_edge = get_nxt_edge(curEdge, nxtEdgeStack.back().back());
            nxtEdgeStack.back().pop_back();

            stateStack.pop_back();
        } else if (!stateStack.empty() and stateStack.size() == nxtEdgeStack.size() + 1 and !nxtEdgeStack.empty() and
                   nxtEdgeStack.back().empty()) {// if this state is dead and we should look back a state
            if (!str.empty()) str.pop_back();
            // look back a state or multiple if we have empty nxt choice (unique nxt edge)
            while (!nxtEdgeStack.empty() and nxtEdgeStack.back().empty()) {
                nxtEdgeStack.pop_back();
                stateStack.pop_back();
                if (!str.empty()) str.pop_back();
            }
            if (nxtEdgeStack.empty()) {
                return false;
            } else if (!nxtEdgeStack.back().empty()) {
                last_edge = get_nxt_edge(curEdge, nxtEdgeStack.back().back());
                nxtEdgeStack.back().pop_back();
                stateStack.pop_back();
            }
        } else if (stateStack.size() != nxtEdgeStack.size() + 1) {
            throw invalid_argument("state stack and nxtEdge Stack size miss match" + to_string(stateStack.size())
                                   + ":" + to_string(nxtEdgeStack.size()));
        }

        str.push_back(curEdge);

        // Change and register our state for shingle occurrence and nxt edges
        map<size_t, vector<shingle_hash>> tmp_stack = stateStack.back();
        bool found = false;
        auto from_stateStack = tmp_stack.find(last_edge.first);
        if (from_stateStack != tmp_stack.end()) {// it is in the state_stack(previously touched shingle)
            for (auto &tmp_shingle: from_stateStack->second) {
                if (tmp_shingle == last_edge) {
                    tmp_shingle.occurr--;
                    found = true;
                    break;
                }
            }
        }

        if (!found) { // it is never touched,. fetch from original state
            for (auto tmp_shingle: original_state_stack[last_edge.first]) {
                if (tmp_shingle == last_edge) {
                    tmp_shingle.occurr--;
                    tmp_stack[tmp_shingle.first] = {tmp_shingle};
                    break;
                }
            }
        }
        stateStack.push_back(tmp_stack);

//
//        if (!resourceMonitor( initRes, MAX_TIME, MAX_VM_SIZE))
//            return false;

        // if we reached a stop point
        if (str.size() == cyc_info.len) {
            strCollect_size++;
            if (str == final_str || (strCollect_size == cyc_info.cyc and cyc_info.cyc != 0)) {
                cyc_info.cyc = strCollect_size;
                final_str = str;
                auto old_mem = initRes.VmemUsed;
                resourceMonitor(initRes, 300, SIZE_T_MAX);
                (old_mem < initRes.VmemUsed)? highwater = initRes.VmemUsed : highwater = old_mem;
            }
        }

        if (strCollect_size == cyc_info.cyc && cyc_info.cyc != 0) {
            HeapProfilerStop();
            return true;
        }
    }
    return false;
}

std::map<size_t, vector<shingle_hash>> SetsOfContent::tree2shingle_dict(std::set<shingle_hash> &tree_lvl) {
    // prepare shingle_set in a map, and microsorted(sorted for shingles with same head)
    std::map<size_t, vector<shingle_hash>> res;
    for (shingle_hash shingle : tree_lvl){
        res[shingle.first].push_back(shingle);
    }


    for(auto& shingle : res){
        std::sort(shingle.second.begin(),shingle.second.end());
    }
    return res;
}

shingle_hash SetsOfContent::get_nxt_edge(size_t& current_edge, shingle_hash _shingle) {
    current_edge = _shingle.second;
    return _shingle;
}

string SetsOfContent::retriveString() {
    // retrace cycles bottom-up and delete from query after tracing
    string substring;
//    for (auto lvl : theirTree) for (auto item:lvl) cout << item << endl; // TODO: detele this print tree function
    for (int i = myTree.size() - 2; i >= 0; --i) {
        for (shingle_hash shingle : myTree[i]) {
            auto it = cyc_query.find(shingle.second);
            if (it != cyc_query.end()) {
                vector<size_t> tmp;
                substring = "";
                cycle tmp_cyc = cycle{.head = shingle.compose.head, .len=shingle.compose.len, .cyc = it->second};
                shingle2hash_train(tmp_cyc, myTree[i + 1], tmp);
                for (size_t hash:tmp) {
                    if (Dictionary.find(hash) == Dictionary.end())
                        cout << "Recover may have failed - Dictionary lookup failed for "<< hash << " at level " << endl;
                    substring += Dictionary[hash];
                }
                add_to_dictionary(substring);
            }
        }
    }

    return (substring.empty())?myString:substring;
}


// functions for  SyncMethods
bool SetsOfContent::addStr(DataObject *str_p, vector<DataObject *> &datum, bool sync) {
    Logger::gLog(Logger::METHOD,
                 "Entering SetsOfContent::addStr. Parameters - num of par: " + to_string(Partition) + ", num of lvls: "
                 + to_string(Levels) + ", Terminal String Size: " + to_string(TermStrSize) + ", Actual String Size: " +
                 to_string(myString.size()));

    myString = str_p->to_string();
    SyncMethod::addStr(str_p, datum, sync); // calls super class for book keeping

    if (myString.empty()) return false;

    if (myString.size() / pow(Partition, Levels) < 1)
        throw invalid_argument(
                "Terminal String size would end up less than 1, please consider lessen the levels or number of partitions");

    if (Levels == NOT_SET) throw invalid_argument("Consider set a Level value bigger than 0");


    go_through_tree();

//    //show the info of the tree
//    for(auto lvl:myTree) {
//        vector<size_t> lvl_vec;
//        for(auto item : lvl) (item.lvl<myTree.size()-1)?lvl_vec.push_back(Cyc_dict[item.second].size()) : lvl_vec.push_back(Dictionary[item.second].size());
//        sort(lvl_vec.begin(),lvl_vec.end());
//        cout<<"max: "<<lvl_vec.back()<<", min: "<<lvl_vec.front()<<", median: "<<getMedian(lvl_vec)<<", lvl size: "<<lvl_vec.size()<<endl;
//    }//TODO: delete this

    for (DataObject *dop : setPointers) delete dop;
    for (ZZ item : getShingles_ZZ()) {
        setPointers.push_back(new DataObject(item));
    }
    datum = setPointers;
    return true;
}
void SetsOfContent::SendSyncParam(const shared_ptr<Communicant> &commSync, bool oneWay) {
    Logger::gLog(Logger::METHOD,"Entering SendSyncParam::SendSyncParam");
    // take care of parent sync method for sync mode
//    SyncMethod::SendSyncParam(commSync);
    commSync->commSend(enumToByte(SyncID));
    commSync->commSend((size_t)TermStrSize);
    commSync->commSend((size_t)Levels);
    commSync->commSend((size_t)Partition);
    if (commSync->commRecv_byte() == SYNC_FAIL_FLAG)
        throw SyncFailureException("Sync parameters do not match.");
    Logger::gLog(Logger::COMM, "Sync parameters match");
}

void SetsOfContent::RecvSyncParam(const shared_ptr<Communicant> &commSync, bool oneWay) {
    Logger::gLog(Logger::METHOD,"Entering SetsOfContent::RecvSyncParam");
    // take care of parent sync method
//    SyncMethod::RecvSyncParam(commSync);

    byte theSyncID = commSync->commRecv_byte();
    size_t TermStrSize_C = (size_t)commSync->commRecv_size_t();
    size_t Levels_C = (size_t)commSync->commRecv_size_t();
    size_t Partition_C = (size_t)commSync->commRecv_size_t();

    if (theSyncID != enumToByte(SyncID) ||
        TermStrSize_C != TermStrSize ||
        Levels_C != Levels ||
        Partition_C != Partition) {
        // report a failure to establish sync parameters
        commSync->commSend(SYNC_FAIL_FLAG);
        Logger::gLog(Logger::COMM, "Sync parameters differ from client to server: Client has (" +
                                   to_string(TermStrSize_C) + "," + to_string(Levels_C) + "," + toStr(Partition_C) +
                                   ").  Server has (" + to_string(TermStrSize) + "," + to_string(Levels) + "," + to_string(Partition) +").");
        throw SyncFailureException("Sync parameters do not match.");
    }
    commSync->commSend(SYNC_OK_FLAG);
    Logger::gLog(Logger::COMM, "Sync parameters match");
}

bool SetsOfContent::SyncServer(const shared_ptr<Communicant> &commSync, list<DataObject*> &selfMinusOther, list<DataObject*> &otherMinusSelf) {
    bool success = false;
    Logger::gLog(Logger::METHOD, "Entering SetsOfContent::SyncServer");

    commSync->commListen();
    long mbar;
    if (GenSync::SyncProtocol::IBLTSyncSetDiff == baseSyncProtocol) {
        StrataEst est = StrataEst(sizeof(shingle_hash));

        for (auto item : setPointers) {
            est.insert(item);
        }

        // since Kshingling are the same, Strata Est parameters would also be the same.
        auto theirStarata = commSync->commRecv_Strata();
        mbar = (est -= theirStarata).estimate();
        commSync->commSend(mbar); // Dangerous cast
    } else if (GenSync::SyncProtocol::CPISync == baseSyncProtocol) {
        mbar = 1e4;// elaborate Mbar

    }

    RecvSyncParam(commSync);
    shared_ptr<SyncMethod> setHost;
    SyncMethod::SyncServer(commSync,selfMinusOther,otherMinusSelf);
    configure(setHost, mbar);
    for (DataObject *dop : setPointers) {
        setHost->addElem(dop); // Add to GenSync
    }

    vector<shingle_hash> theirs_hash, mine_hash;

    size_t top_str_size = SIZE_T_MAX;
    while (!success and mbar < top_str_size) { // if set recon failed, This can be caused by error rate and small mbar
        success = setHost->SyncServer(commSync, selfMinusOther, otherMinusSelf);
        success = ((SYNC_SUCCESS == commSync->commRecv_int()) and success);
        success ? commSync->commSend(SYNC_SUCCESS) : commSync->commSend(SYNC_FAILURE);
        if (success) break;

        top_str_size = myString.size();
        commSync->commSend(myString.size());
        Logger::gLog(Logger::METHOD,
                     "SetsOfContent::SyncServer - mbar doubled from " + to_string(mbar) + " to " +
                     to_string(2 * (mbar + 1)));
        mbar = 2 * (mbar + 1);
        configure(setHost, mbar);

        for (DataObject *dop : setPointers) {
            setHost->addElem(dop); // Add to GenSync
        }
        selfMinusOther.clear();
        otherMinusSelf.clear();
    }



//    for (auto shingle : others) theirs_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));
    for (auto shingle : selfMinusOther) mine_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));

//    prepare_concerns(theirs_hash,mine_hash);

//    cout<< "Server - Term concern size : "<< term_concern.size()<<endl;

    size_t query_size = commSync->commRecv_size_t();
    map<size_t, bool> queries;
    for (size_t i = 0; i < query_size; ++i) {// get cycles queries
        queries[commSync->commRecv_size_t()] = true;
    }
    answer_queries(queries, mine_hash);
    for (auto groupcyc : cyc_concern) {
        commSync->commSend(groupcyc.second);
    }
    for (auto dic : term_concern) {
        string tmp_str = Dictionary[dic.first];
        if (!tmp_str.empty())
            commSync->commSend(tmp_str);
        else
            commSync->commSend("$");
    }

//    commSync->commSend(SYNC_SUCCESS);
//    cout<<"Server Close"<<endl;
    Logger::gLog(Logger::METHOD, "SetOfCeontent Done");
    commSync->commClose();
    return success;
}

bool SetsOfContent::SyncClient(const shared_ptr<Communicant> &commSync, list<DataObject*> &selfMinusOther, list<DataObject*> &otherMinusSelf) {
    //TODO: needs a flag, but  this will do for now
    bool success = false;
    Logger::gLog(Logger::METHOD, "Entering SetsOfContent::SyncClient");

    commSync->commConnect();
    long mbar;
    if (GenSync::SyncProtocol::IBLTSyncSetDiff == baseSyncProtocol) {
        StrataEst est = StrataEst(sizeof(shingle_hash));

        for (auto item :setPointers) {
            est.insert(item); // Add to estimator
        }

        // since Kshingling are the same, Strata Est parameters would also be the same.
        commSync->commSend(est.getStrata(), false);

        mbar = commSync->commRecv_long(); // cast long to long long

    } else if (GenSync::SyncProtocol::CPISync == baseSyncProtocol) {
        mbar = 1e4;// elaborate Mbar
    }


    SendSyncParam(commSync);
    shared_ptr<SyncMethod> setHost;
    SyncMethod::SyncClient(commSync,selfMinusOther,otherMinusSelf);
    configure(setHost, mbar);

    for (DataObject *dop : setPointers) {
        setHost->addElem(dop); // Add to GenSync
    }

    vector<shingle_hash> theirs_hash, mine_hash;
    size_t top_str_size = SIZE_T_MAX;
    while (!success and mbar < top_str_size) { // if set recon failed, This can be caused by error rate and small mbar
        success = setHost->SyncClient(commSync, selfMinusOther, otherMinusSelf);
        success ? commSync->commSend(SYNC_SUCCESS) : commSync->commSend(SYNC_FAILURE);
        success = (commSync->commRecv_int() == SYNC_SUCCESS) and success;
        if (success) break;

        top_str_size = commSync->commRecv_size_t();
        Logger::gLog(Logger::METHOD,
                     "SetsOfContent::SyncClient - mbar doubled from " + to_string(mbar) + " to " +
                     to_string(2 * (mbar + 1)));
        cout << "SetsOfContent::SyncClient - mbar doubled from " + to_string(mbar) + " to " + to_string(2 * (mbar + 1))
             << endl;
        mbar = 2 * (mbar + 1);
        configure(setHost, mbar);

        for (DataObject *dop : setPointers) {
            setHost->addElem(dop); // Add to GenSync
        }
        selfMinusOther.clear();
        otherMinusSelf.clear();
    }

    for (auto shingle : otherMinusSelf) theirs_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));
    for (auto shingle : selfMinusOther) mine_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));

    prepare_querys(theirs_hash, mine_hash);
//    cout<< "cyc query size : "<< cyc_query.size()<<endl;
//    cout<< "Term query size : "<< term_query.size()<<endl;

// ask questions
    commSync->commSend(cyc_query.size() + term_query.size());
    for (auto cyc:cyc_query) {// ask about cycles
        commSync->commSend(cyc.first);
    }

    for (auto term:term_query) {// ask about cycles
        commSync->commSend(term.first);
    }
// get answers from server
    for (auto &cyc:cyc_query) {
        cyc.second = commSync->commRecv_size_t();
    }

    for (int i = 0; i < term_query.size(); ++i) {
        auto tmp = commSync->commRecv_string();
        if (tmp != "$")
            add_to_dictionary(tmp);
    }

//    cout<<"Client Close"<<endl;
    Logger::gLog(Logger::METHOD, "Set Of Content Done");
    commSync->commClose();
    return success;
}


void SetsOfContent::configure(shared_ptr<SyncMethod>& setHost, long mbar) {
    if (GenSync::SyncProtocol::IBLTSyncSetDiff == baseSyncProtocol)
        setHost = make_shared<IBLTSync_SetDiff>(mbar, sizeof(shingle_hash), true);
    else if (GenSync::SyncProtocol::InteractiveCPISync == baseSyncProtocol)
        setHost = make_shared<InterCPISync>(5, sizeof(shingle_hash) * 8, 64, 7, true);
    else if (GenSync::SyncProtocol::CPISync == baseSyncProtocol)
        setHost = make_shared<ProbCPISync>(mbar,sizeof(shingle_hash) * 8,64,true);
}

bool SetsOfContent::reconstructString(DataObject *&recovered_string, const list<DataObject *>& mySetData) {
    myTree.clear();
    myTree.resize(Levels);
    for(auto s_zz : mySetData){
        auto shingle = ZZtoShingleHash(s_zz->to_ZZ());
        myTree[shingle.lvl].insert(shingle);
    }

    myString = retriveString();
    recovered_string = new DataObject(myString);
    return true;
}
