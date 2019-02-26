//
// Created by Bowen Song on 12/9/18.
//

#include "SetsOfContent.h"

SetsOfContent::SetsOfContent(size_t terminal_str_size, size_t levels, size_t partition,
                             GenSync::SyncProtocol base_set_proto, size_t shingle_size, size_t space)
        : TermStrSize(terminal_str_size), Levels(levels), Partition(partition), baseSyncProtocol(base_set_proto),
          terShingleLen(shingle_size), terSpace(space) {
    SyncID = SYNC_TYPE::SetsOfContent;
    if (levels > USHRT_MAX or levels < 2)
        throw invalid_argument("Num of Level specified should be between 2 and " + to_string(USHRT_MAX));

//    initResources(initRes);
}

SetsOfContent::~SetsOfContent() {
    for (DataObject *dop : setPointers) delete dop;
    for (DataObject *dop : hashPointers) delete dop;
}

vector<size_t> SetsOfContent::create_HashSet(string str, size_t win_size, size_t space, size_t shingle_size) {
    // space and shingle_size has to match between reconciling strings
    // Time Complexity 2n, where n is the string size.
    vector<size_t> hash_val, hash_set;

    /* ---------original begin  */
    // if the substring is smaller than the terminal string size, we do not partition it anymore.
    if (str.size() <= TermStrSize) {
        hash_set = {add_to_dictionary(str)};
    } else { // else we partitions it

        if (space == 0) throw invalid_argument("Space for windowing is 0 at create_HashSet");
        if (shingle_size < 2) throw invalid_argument("Shingle size should not go under 2");
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
    /* ---------original end  */
    /* ---------fixed win size begin */
//    if (str.size() <= TermStrSize) {
//        hash_set = {str_to_hash(str)};
//    }else {
//        for (int i = 0; i < str.size(); i += win_size) {
//            hash_set.push_back(add_to_dictionary(str.substr(i, win_size)));
//        }
//    }
    /* ---------fixed win size end */

    /* ---------fixed hash value begin */
//    if (str.size() <= TermStrSize) {
//        hash_set = {add_to_dictionary(str)};
//    }else {size_t prev = 0;
//    auto hash_content = hashcontent_dict[str_to_hash(str)];
//                for (size_t min:local_mins(hash_content, win_size)) {
//
//                    min+=shingle_size;
//            hash_set.push_back(add_to_dictionary(str.substr(prev, min - prev)));
//                    hashcontent_dict[str_to_hash(str.substr(prev, min - prev))] = {hash_content.begin()+(prev-shingle_size),hash_content.begin()+(min-shingle_size)};
//
//                    prev = min;
//        }
//        hash_set.push_back(add_to_dictionary(str.substr(prev)));
//        hashcontent_dict[str_to_hash(str.substr(prev))] = {hash_content.begin()+(prev-shingle_size),hash_content.end()};
//
//    }
    /* ---------fixed hash value end */

    // write it to cyc-dict
    auto cyc_it = Cyc_dict.find(str_to_hash(str));
    if (cyc_it == Cyc_dict.end()) // check if cyc exists
        Cyc_dict[str_to_hash(str)] = hash_set; // update Cyc_dict
    else if (cyc_it->second != hash_set and cyc_it->second.size() == 1 and cyc_it->second.front() == cyc_it->first)
        Cyc_dict[str_to_hash(str)] = hash_set;// last stage no partition, update Cyc_dict
    else if (cyc_it->second != hash_set) // check if it is getting overwritten
        throw invalid_argument("More than one answer is possible for Cyc_dict");

    return hash_set;
}


size_t SetsOfContent::add_to_dictionary(const string &str) {
    (Dictionary.find(str_to_hash(str)) == Dictionary.end() or Dictionary[str_to_hash(str)] == str)
    ? Dictionary[str_to_hash(str)] = str : throw invalid_argument(
            "Dictionary duplicated suggest using new/multiple hash functions");
    return str_to_hash(str);
}

void SetsOfContent::go_through_tree() {
    myTree.clear(); // should be redundant

    auto String_Size = pow(Partition, Levels) *
                       TermStrSize; // calculate a supposed string size, a string size that make sense with the parameters

    if (String_Size < 1)
        throw invalid_argument(
                "fxn go_through_tree - parameters do not make sense - num of par: " + to_string(Partition) +
                ", num of lvls: " +
                to_string(Levels) + ", Terminal String Size: " + to_string(TermStrSize) + ", Actual String Size: " +
                to_string(myString.size()));

    size_t shingle_size = terShingleLen * pow(2,
                                              Levels); //(Parameter c, terminal rolling hash window size) //max(floor(log2(String_Size)),pow(terWin_s,Levels+1))
    if (shingle_size < 1)
        throw invalid_argument("Consider larger the parameters for auto shingle size to be more than 1");
    size_t space = terSpace * pow(2 * Partition, Levels); //126 for ascii content (Parameter terminal space)
    vector<size_t> cur_level;
    // fill up the tree
    myTree.resize(Levels);

    // put up the first level
    update_tree_shingles({add_to_dictionary(myString)}, 0);

/* ---------fixed hash value begin */
//vector<size_t> hash_val;
//            for (size_t i = 0; i < myString.size() - shingle_size + 1; ++i) {
//            std::hash<std::string> shash;
//            hash_val.push_back(shash(myString.substr(i, shingle_size)) % space);
//        }
//    hashcontent_dict[str_to_hash(myString)] = hash_val;
/* ---------fixed hash value end */

    for (int l = 1; l < Levels; ++l) {

        // Fill up Cycle Dictionary for non terminal strings
        for (auto substr_hash:unique_substr_hash(myTree[l - 1])) {
            string substring = Dictionary[substr_hash];
            if (substring.empty()) continue; // this ditches the empty strings

            cur_level = create_HashSet(substring, floor((substring.size() / Partition) / 2), space, shingle_size);
            update_tree_shingles(cur_level, l);

        }

        space = floor((space / Partition) / 2);
        shingle_size = floor(shingle_size / 2);
    }

}


// what i am missing, and what they would be sending to me
void SetsOfContent::prepare_querys(std::set<size_t> &myQueries) {

    term_query.clear();// should be empty anyway
    cyc_query.clear();

    for (auto rit = myTree.rbegin(); rit != myTree.rend(); ++rit) {
        for (auto shingle : *rit) {

            auto it = myQueries.find(shingle.second);
            if (it != myQueries.end()) {
                myQueries.erase(it);

                if (myTree.size() - 1 == shingle.lvl) {
                    if (!term_query[shingle.second].empty())
                        cout << "we already have this term for some reason, check it!" << endl;
//                    term_query[shingle.second] = "";
                } else {
                    cyc_query[shingle.second] = cycle{.head=0, .len=0, .cyc=0};
                }
            }
        }
    }
//    for(auto lvl : theirTree) for (auto shingle : lvl) cout<< shingle<<endl; // TODO: delete this print tree function

}


void SetsOfContent::answer_queries(std::set<size_t> &theirQueries) {
    cyc_concern.clear();
    term_concern.clear();

    for (auto rit = myTree.rbegin(); rit != myTree.rend(); ++rit) {
        for (auto shingle : *rit) {
            auto it = theirQueries.find(shingle.second);
            if (it != theirQueries.end()) {
                theirQueries.erase(it);
                if (myTree.size() - 1 == shingle.lvl)
                    term_concern[shingle.second] = Dictionary[shingle.second];
                else {
                    vector<size_t> tmp_vec = Cyc_dict[shingle.second];
                    cycle tmp = cycle{.head = tmp_vec.front(), .len = (unsigned int) tmp_vec.size(), .cyc=0};

                    if (!shingle2hash_train(tmp, myTree[shingle.lvl + 1], Cyc_dict[shingle.second])) {
                        continue;
                    }
                    cyc_concern[shingle.second] = tmp;
                }
            }
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
                "update_tree shingle is empty");

    for (auto item = tmp.begin(); item != tmp.end(); ++item) {
        if (item->second > USHRT_MAX)
            Logger::error_and_quit(
                    "Shingle occurrance is larger than USHRT_MAX, (backtracking could be infeasiable and our shingle_hash carrier is overflown)");
        myTree[level].insert(
                shingle_hash{.first = item->first.first, .second = item->first.second, .occurr = (sm_i) item->second,
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
bool SetsOfContent::shingle2hash_train(cycle &cyc_info, const std::set<shingle_hash> &shingle_set,
                                       vector<size_t> &final_str) {

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
        vector<shingle_hash> nxtEdges = get_nxt_shingle_vec(curEdge, stateStack.back(), original_state_stack);

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
                }
                tmp_stack[tmp_shingle.first].push_back(tmp_shingle);
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
//                resourceMonitor(initRes, 300, SIZE_T_MAX);
                (old_mem < initRes.VmemUsed) ? highwater = initRes.VmemUsed : highwater = old_mem;
            }
        }


        if (strCollect_size == cyc_info.cyc && cyc_info.cyc != 0) {
            HeapProfilerStop();
            return true;
        }
    }
    return false;
}

std::map<size_t, vector<shingle_hash>> SetsOfContent::tree2shingle_dict(const std::set<shingle_hash> &tree_lvl) {
    // prepare shingle_set in a map, and microsorted(sorted for shingles with same head)
    std::map<size_t, vector<shingle_hash>> res;
    for (shingle_hash shingle : tree_lvl) {
        res[shingle.first].push_back(shingle);
    }


    for (auto &shingle : res) {
        std::sort(shingle.second.begin(), shingle.second.end());
    }
    return res;
}

shingle_hash SetsOfContent::get_nxt_edge(size_t &current_edge, shingle_hash _shingle) {
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
                cycle tmp_cyc = it->second;
                if (!shingle2hash_train(tmp_cyc, myTree[i + 1], tmp))
                    substring = Dictionary[shingle.second];
                for (size_t hash:tmp) {
                    if (Dictionary.find(hash) == Dictionary.end())
                        cout << "Recover may have failed - Dictionary lookup failed for " << hash << " at level "
                             << shingle.lvl << endl;
                    substring += Dictionary[hash];
                }
                add_to_dictionary(substring);
            }
        }
    }

    return (substring.empty()) ? myString : substring;
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
        invalid_argument( "Terminal String size could end up less than 1, limited at" + to_string(TermStrSize) +
                ", please consider lessen the levels or number of partitions");

    if (Levels == NOT_SET) throw invalid_argument("Consider set a Level value bigger than 0");


    go_through_tree();

//    //TODO: delete below
//    //show the info of the tree
//    for(auto lvl:myTree) {
//        vector<size_t> lvl_vec;
//        for(auto item : lvl) (item.lvl<myTree.size()-1)?lvl_vec.push_back(Cyc_dict[item.second].size()) : lvl_vec.push_back(Dictionary[item.second].size());
//        sort(lvl_vec.begin(),lvl_vec.end());
//        cout<<"max: "<<lvl_vec.back()<<", min: "<<lvl_vec.front()<<", median: "<<getMedian(lvl_vec)<<", lvl size: "<<lvl_vec.size()<<endl;
//    }
//    std::set<size_t> unique_hashes;
//    for (auto term :myTree.back()){
//        cout<<term<<endl;
//        unique_hashes.insert(term.second);
//    }
//    cout<<"We have "<<unique_hashes.size()<<" unique terminal hashes over "<<myTree.back().size() <<" shingles"<<endl;
//    //TODO: delete above

    for (DataObject *dop : setPointers) delete dop;
    for (DataObject *dop : hashPointers) delete dop;
    for (auto item : getALLFuzzyShingleZZ()) {
        setPointers.push_back(new DataObject(item));
    }  //get individual fuzzy_shingle
    for (auto item : getALLHashZZ()) { hashPointers.push_back(new DataObject(item)); }

    datum = setPointers;
    return true;
}

void SetsOfContent::SendSyncParam(const shared_ptr<Communicant> &commSync, bool oneWay) {
    Logger::gLog(Logger::METHOD, "Entering SendSyncParam::SendSyncParam");
    // take care of parent sync method for sync mode
//    SyncMethod::SendSyncParam(commSync);
    commSync->commSend(enumToByte(SyncID));
    commSync->commSend((size_t) TermStrSize);
    commSync->commSend((size_t) Levels);
    commSync->commSend((size_t) Partition);
    if (commSync->commRecv_byte() == SYNC_FAIL_FLAG)
        throw SyncFailureException("Sync parameters do not match.");
    Logger::gLog(Logger::COMM, "Sync parameters match");
}

void SetsOfContent::RecvSyncParam(const shared_ptr<Communicant> &commSync, bool oneWay) {
    Logger::gLog(Logger::METHOD, "Entering SetsOfContent::RecvSyncParam");
    // take care of parent sync method
//    SyncMethod::RecvSyncParam(commSync);

    byte theSyncID = commSync->commRecv_byte();
    size_t TermStrSize_C = (size_t) commSync->commRecv_size_t();
    size_t Levels_C = (size_t) commSync->commRecv_size_t();
    size_t Partition_C = (size_t) commSync->commRecv_size_t();

    if (theSyncID != enumToByte(SyncID) ||
        TermStrSize_C != TermStrSize ||
        Levels_C != Levels ||
        Partition_C != Partition) {
        // report a failure to establish sync parameters
        commSync->commSend(SYNC_FAIL_FLAG);
        Logger::gLog(Logger::COMM, "Sync parameters differ from client to server: Client has (" +
                                   to_string(TermStrSize_C) + "," + to_string(Levels_C) + "," + toStr(Partition_C) +
                                   ").  Server has (" + to_string(TermStrSize) + "," + to_string(Levels) + "," +
                                   to_string(Partition) + ").");
        throw SyncFailureException("Sync parameters do not match.");
    }
    commSync->commSend(SYNC_OK_FLAG);
    Logger::gLog(Logger::COMM, "Sync parameters match");
}

bool SetsOfContent::SyncServer(const shared_ptr<Communicant> &commSync, list<DataObject *> &selfMinusOther,
                               list<DataObject *> &otherMinusSelf) {
    bool success = true;
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
    // sync shingle xor sum
    if (!setReconServer(commSync, mbar, sizeof(fuzzy_shingle), setPointers, selfMinusOther, otherMinusSelf))
        success = false;
    cleanup(setPointers, selfMinusOther, otherMinusSelf);

    // sync hash
    if (!setReconServer(commSync, mbar, sizeof(size_t), hashPointers, selfMinusOther, otherMinusSelf))
        success = false;

    std::set<size_t> theirQueries;
    for (DataObject *item : selfMinusOther)
        theirQueries.insert(ZZtoSize_t(item->to_ZZ()));

    cleanup(hashPointers, selfMinusOther, otherMinusSelf);

    vector<DataObject *> fuzzyPts;
    unsigned int counter = 0;
    if (fuzzy_lookup.size() > UINT_MAX)
        Logger::error_and_quit(
                "Server: Fuzzy Order (Number of tree nodes) exceed UINT_MAX, CHANGE CODE to long or size_t"); // in our design, the tree size should not be that big
    for (auto fuzzy : fuzzy_lookup)
        fuzzyPts.push_back(new DataObject(
                TtoZZ(fuzzyorder{.order = counter++, .mode =(sm_i)(fuzzy.first.duplicate==1 ? 2 : (fuzzy.second.first == std::min(fuzzy.second.first,fuzzy.second.second)?0:1))})));

    if (!setReconServer(commSync, mbar, sizeof(fuzzyorder), fuzzyPts, selfMinusOther, otherMinusSelf))
        success = false;


    answer_queries(theirQueries);

    cleanup(fuzzyPts, selfMinusOther, otherMinusSelf, true);

//    size_t top_str_size = SIZE_T_MAX;
//    while (!success and mbar < top_str_size) { // if set recon failed, This can be caused by error rate and small mbar
//        success = setHost->SyncServer(commSync, selfMinusOther, otherMinusSelf);
//        success = ((SYNC_SUCCESS == commSync->commRecv_int()) and success);
//        success ? commSync->commSend(SYNC_SUCCESS) : commSync->commSend(SYNC_FAILURE);
//        if (success) break;
//
//        top_str_size = myString.size();
//        commSync->commSend(myString.size());
//        Logger::gLog(Logger::METHOD,
//                     "SetsOfContent::SyncServer - mbar doubled from " + to_string(mbar) + " to " +
//                     to_string(2 * (mbar + 1)));
//        mbar = 2 * (mbar + 1);
//        configure(setHost, mbar);
//
//        for (DataObject *dop : setPointers) {
//            setHost->addElem(dop); // Add to GenSync
//        }
//        selfMinusOther.clear();
//        otherMinusSelf.clear();
//    }


//
////    for (auto shingle : others) theirs_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));
//    for (auto shingle : selfMinusOther) mine_hash.push_back(ZZtoFuzzyShingle(shingle->to_ZZ()));
//
////    cout<< "Server - Term concern size : "<< term_concern.size()<<endl;
//
//    size_t query_size = commSync->commRecv_size_t();
//    map<size_t, bool> queries;
//    for (size_t i = 0; i < query_size; ++i) {// get cycles queries
//        queries[commSync->commRecv_size_t()] = true;
//    }
//    answer_queries(queries, mine_hash);

//    cout << "we answered " << cyc_concern.size() << " cycles and " << term_concern.size() << " hashes" << endl;
    for (auto groupcyc : cyc_concern) {
        commSync->commSend(TtoZZ(groupcyc.second), sizeof(cycle));
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

bool SetsOfContent::SyncClient(const shared_ptr<Communicant> &commSync, list<DataObject *> &selfMinusOther,
                               list<DataObject *> &otherMinusSelf, map<string, double> &CustomResult) {
    bool success = true;
    Logger::gLog(Logger::METHOD, "Entering SetsOfContent::SyncClient");

    commSync->commConnect();
    long mbar;
    if (GenSync::SyncProtocol::IBLTSyncSetDiff == baseSyncProtocol) {
        StrataEst est = StrataEst(sizeof(shingle_hash));

        for (auto item :setPointers) {
            est.insert(item); // Add to estimator
        }
        commSync->commSend(est.getStrata(), false);

        mbar = commSync->commRecv_long(); // cast long to long long

    } else if (GenSync::SyncProtocol::CPISync == baseSyncProtocol) {
        mbar = 1e4;// elaborate Mbar
    }


    SendSyncParam(commSync);
    // ------------------------- Sync Fuzzy Shingle
    // sync shingle xor sum
    if (!setReconClient(commSync, mbar, sizeof(fuzzy_shingle), setPointers, selfMinusOther, otherMinusSelf))
        success = false;

//    cout << "We used comm bytes: " << commSync->getRecvBytesTot() + commSync->getXmitBytesTot() << endl;
    cleanup(setPointers, selfMinusOther, otherMinusSelf);

    // ------------------------- Sync Individual Hash
    auto myPartitions = hashPointers.size();
    // sync hash
    if (!setReconClient(commSync, mbar, sizeof(size_t), hashPointers, selfMinusOther, otherMinusSelf))
        success = false;

    CustomResult["Partition Sym Diff"] = (selfMinusOther.size() +
                                          otherMinusSelf.size()); // recorder # symmetrical partition difference
    CustomResult["Total Num Partitions"] = hashPointers.size() + myPartitions;

    std::set<size_t> myQueries;
    for (DataObject *item : otherMinusSelf)
        myQueries.insert(ZZtoSize_t(item->to_ZZ()));

    cleanup(hashPointers, selfMinusOther, otherMinusSelf);

    // Collect fuzzy shingles in use and their composite
    map<fuzzy_shingle, pair<vector<size_t>, sm_i>> fuzzy_map; // Place them in fuzzy shingle order, smaller component, bigger component, and placement (0: small first, 1: small second, 3: both exist)
    std::set<size_t> all_hashes{0}; // we need to include 0 as a hash value
    auto a = all_hashes.size();
    for (auto pts : hashPointers) all_hashes.insert(ZZtoSize_t(pts->to_ZZ()));
    for (auto pt :setPointers) {
        fuzzy_shingle fshingle = ZZtoFuzzyShingle(pt->to_ZZ());

        auto it = fuzzy_lookup.find(fshingle);
        if (it != fuzzy_lookup.end()) {// locally available from fuzzy lookup table
            fuzzy_map[fshingle] = FuzzyOrder({it->second.first, it->second.second}, 3, fshingle.duplicate ==1);
        } else {//else find its composition from all_hashes
            if(fshingle.duplicate ==2) {
                fuzzy_map[fshingle] = FuzzyOrder({fshingle.sum, fshingle.sum}, 3, false);
                continue;
            }
            for (size_t hash : all_hashes) {

                auto tmp_it = all_hashes.find(fshingle.sum ^ hash);
                if (tmp_it != all_hashes.end()) {
                    fuzzy_map[fshingle] = FuzzyOrder({hash, (*tmp_it)}, 3, fshingle.duplicate ==1);
//                    cout<<"We found "<<hash<< " and "<<(*tmp_it)<< " for "<<fshingle.sum<<endl;
                    break;
                }
            }
        }
    }

    // ----------------------------- Sync Fuzzy Order
    vector<DataObject *> fuzzyPts;
    unsigned int counter = 0;
    if (fuzzy_map.size() > UINT_MAX)
        Logger::error_and_quit(
                "Client: Fuzzy Order (Number of tree nodes) exceed UINT_MAX, CHANGE CODE to long or size_t"); // in our design, the tree size should not be that big

    // figure out the composition for every known shingles
    for (auto fuzzy : fuzzy_map)
        fuzzyPts.push_back(new DataObject(TtoZZ(fuzzyorder{.order = counter++, .mode = (sm_i) fuzzy.second.second})));
    // set recon
    if (!setReconClient(commSync, mbar, sizeof(fuzzyorder), fuzzyPts, selfMinusOther, otherMinusSelf))
        success = false;

    vector<sm_i> newOrder(fuzzyPts.size());
    for (auto pts:fuzzyPts) {
        fuzzyorder tmp = ZZtoFuzzyorder(pts->to_ZZ());
        newOrder[tmp.order] = tmp.mode;
    }

    // reform the tree
    myTree.clear();
    myTree.resize(Levels);
    size_t newOrder_i = 0;
    for (auto &Fuzzyshingle_pair : fuzzy_map) {
        Fuzzyshingle_pair.second.second = newOrder[newOrder_i++];
        for (shingle_hash shingle : FuzzyOrderAssign(Fuzzyshingle_pair))
            myTree[shingle.lvl].insert(shingle);
    }


    prepare_querys(myQueries);

    cleanup(fuzzyPts, selfMinusOther, otherMinusSelf, true);


//    vector<shingle_hash> theirs_hash, mine_hash;
//    size_t top_str_size = SIZE_T_MAX;
//    while (!success and mbar < top_str_size) { // if set recon failed, This can be caused by error rate and small mbar
//        success = setHost->SyncClient(commSync, selfMinusOther, otherMinusSelf);
//        success ? commSync->commSend(SYNC_SUCCESS) : commSync->commSend(SYNC_FAILURE);
//        success = (commSync->commRecv_int() == SYNC_SUCCESS) and success;
//        if (success) break;
//
//        top_str_size = commSync->commRecv_size_t();
//        Logger::gLog(Logger::METHOD,
//                     "SetsOfContent::SyncClient - mbar doubled from " + to_string(mbar) + " to " +
//                     to_string(2 * (mbar + 1)));
//        cout << "SetsOfContent::SyncClient - mbar doubled from " + to_string(mbar) + " to " + to_string(2 * (mbar + 1))
//             << endl;
//        mbar = 2 * (mbar + 1);
//        configure(setHost, mbar);
//
//        for (DataObject *dop : setPointers) {
//            setHost->addElem(dop); // Add to GenSync
//        }
//        selfMinusOther.clear();
//        otherMinusSelf.clear();
//    }

//    for (auto shingle : otherMinusSelf) theirs_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));
//    for (auto shingle : selfMinusOther) mine_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));
//
//    prepare_querys(theirs_hash, mine_hash);
////    cout<< "cyc query size : "<< cyc_query.size()<<endl;
////    cout<< "Term query size : "<< term_query.size()<<endl;
//
//// ask questions
//    commSync->commSend(cyc_query.size() + term_query.size());
//    for (auto cyc:cyc_query) {// ask about cycles
//        commSync->commSend(cyc.first);
//    }
//
//    for (auto term:term_query) {// ask about cycles
//        commSync->commSend(term.first);
//    }
//    CustomResult["hash vec comm"] = (double)(term_query.size()+cyc_query.size())*sizeof(size_t); // record bytes
//// get answers from server
//    cout << "We queried " << cyc_query.size() << " cycles and " << term_query.size() << " hashes" << endl;


    for (auto &cyc:cyc_query) {
        cyc.second = ZZtoCycle(commSync->commRecv_ZZ(sizeof(cycle)));
    }


    size_t term_counter = 0;
    for (int i = 0; i < term_query.size(); ++i) {
        auto tmp = commSync->commRecv_string();
        term_counter += tmp.size();
        if (tmp != "$")
            add_to_dictionary(tmp);
    }
    CustomResult["Literal comm"] = (double) term_counter;
//    cout<<"Client Close"<<endl;
    Logger::gLog(Logger::METHOD, "Set Of Content Done");

    commSync->commClose();
    return success;
}


void SetsOfContent::configure(shared_ptr<SyncMethod> &setHost, long mbar, size_t elem_size) {
    if (GenSync::SyncProtocol::IBLTSyncSetDiff == baseSyncProtocol)
        setHost = make_shared<IBLTSync_SetDiff>(mbar, elem_size, true);
    else if (GenSync::SyncProtocol::InteractiveCPISync == baseSyncProtocol)
        setHost = make_shared<InterCPISync>(5, elem_size * 8, 64, 3, true);
    else if (GenSync::SyncProtocol::CPISync == baseSyncProtocol)
        setHost = make_shared<ProbCPISync>(mbar, elem_size * 8, 64, true);
}

bool SetsOfContent::reconstructString(DataObject *&recovered_string, const list<DataObject *> &mySetData) {
//    myTree.clear();
//    myTree.resize(Levels);
//    for(auto s_zz : mySetData){
//        auto shingle = ZZtoShingleHash(s_zz->to_ZZ());
//        myTree[shingle.lvl].insert(shingle);
//    }
//
    myString = retriveString();
    recovered_string = new DataObject(myString);
    return true;
}

