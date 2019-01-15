//
// Created by Bowen Song on 12/9/18.
//

#include "SetsOfContent.h"
SetsOfContent::SetsOfContent(size_t terminal_str_size, size_t levels, size_t partition, GenSync::SyncProtocol base_set_proto, size_t shingle_size)
: TermStrSize(terminal_str_size), Levels(levels), Partition(partition), baseSyncProtocol(base_set_proto), HashShingleSize(shingle_size){
    SyncID = SYNC_TYPE::SetsOfContent;
    if (levels > USHRT_MAX) throw invalid_argument("Num of Level specified is not feasible");
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
    if (win_size < 1) {
        cout
                << "Content Partition window size is less than 1 and adjusted to 1. Consider adjusting number of partition levels"
                << endl;
        win_size = 1;
    }
    vector<size_t> mins;
    map<size_t, size_t> hash_occurr;
    for (size_t j = 0; j < 2 * win_size; ++j) {
        if (hash_occurr[hash_val[j]])
            hash_occurr[hash_val[j]]++;
        else
            hash_occurr[hash_val[j]] = 1;
    }

    for (size_t i = win_size+1; i < hash_val.size() - win_size+1; ++i) {
        if (hash_val[i-1] <= hash_occurr.begin()->first)
            mins.push_back(i-1);
        if (hash_occurr[hash_val[i - win_size - 1]])
            hash_occurr[hash_val[i - win_size - 1]]--;
        if (!hash_occurr[hash_val[i - win_size - 1]])
            hash_occurr.erase(hash_val[i - win_size - 1]);
        if (hash_occurr[hash_val[i+win_size]])
            hash_occurr[hash_val[i+win_size]]++;
        else
            hash_occurr[hash_val[i+win_size]] = 1;
    }
    return mins;
}

size_t SetsOfContent::add_to_dictionary(string str) {
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
            const_cast<cycle&> (shingle.compose) = cycle{.cyc = 0, .head = it->second.front(), .tail = it->second.back()};
        }
        space = floor((space / Partition) / 2);
        shingle_size = floor(shingle_size / 2);
    }
//    for(auto lvl : myTree) for (auto shingle : lvl) cout<< shingle<<endl; // TODO: delete this print tree function
//    cout<<myTree[0].size()<<":"<<myTree[1].size()<<":"<<myTree[2].size()<<endl;
//    cout << "Rest of the Lvls time with BackTracking: " << (double) (clock() - all_lvls_t) / CLOCKS_PER_SEC << endl;
}



// what i am missing, and what they would be sending to me
void SetsOfContent::prepare_querys(vector<shingle_hash> shingle_hash_theirs, vector<shingle_hash> shingle_hash_mine) {
// get the tree ready
// know what to expect by getting term_query and cyc_query ready
//    map<size_t, vector<shingle_hash>> mine_rid;
    theirTree.resize(myTree.size());// resize their tree
    map<shingle_hash,bool> rid_shingles;

    term_query.clear();// should be empty anyway
    cyc_query.clear();

    // string recon needs to get rid of what is not on their tree. its sync not recon
    for(shingle_hash shingle : shingle_hash_mine){
        rid_shingles[shingle] = true;
    }

    // fill up cyc_quey and term_query , concner and query should be identical. We assume map auto sorts the items (hash values)
    for (shingle_hash shingle:shingle_hash_theirs) { // put what i learnt from set recon in their tree
        theirTree[shingle.lvl].insert(shingle);

        if (Dictionary.find(shingle.second) == Dictionary.end()) {
            if (shingle.lvl < theirTree.size() - 1)
                cyc_query[shingle.second] = 0;
            else
                term_query[shingle.second] = "";
        }
    }
//    for(auto lvl : theirTree) for (auto shingle : lvl) cout<< shingle<<endl; // TODO: delete this print tree function


    // move the rest of the tree.
    for (auto tree_lvl : myTree) { // if it is the same ID, put the shingle in their tree
        for (shingle_hash shingle: tree_lvl) {
            if (!rid_shingles[shingle]) {
                theirTree[shingle.lvl].insert(shingle);
            }
        }
    }

}

// what they are missing, and i will prepare to send them
void SetsOfContent::prepare_concerns(vector<shingle_hash> shingle_hash_theirs, vector<shingle_hash> shingle_hash_mine) {
    // figure out what they are missing, on cycles and dictionary
    // track trough to find cycle number
    // prepare
    // later send the information over

    // get all unique hashes that is konw to their host

    term_concern.clear();
    cyc_concern.clear();

    map<size_t,bool> hashes_they_know; // seconds of (myTree - my unique + their hashes)

    map<size_t,bool> my_unique;

    for(auto shingle:shingle_hash_mine){
        my_unique[shingle.second] = true;
    }

    for (auto lvl : myTree) {
        for(auto shingle : lvl){
            if(!my_unique[shingle.second]) hashes_they_know[shingle.second] = true;
        }
    }

    for (auto shingle : shingle_hash_theirs) hashes_they_know[shingle.second] = true;



    for (shingle_hash shingle : shingle_hash_mine) {

        if(hashes_they_know[shingle.second]) continue;

        if (myTree.size() - 1 == shingle.lvl) {
            term_concern[shingle.second] = Dictionary[shingle.second];
        }else {
            cycle tmp = shingle.compose;
            if (!shingle2hash_train(tmp,myTree[shingle.lvl+1],Cyc_dict[shingle.second])) throw invalid_argument("We failed to get a cycle number to send to an other party at lvl: "+to_string(shingle.lvl));
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
                        .compose = cycle{.cyc=0, .head = 0, .tail = 0},
                        .lvl = level});
    }

}


//vector<string> SetsOfContent::getTerminalDiffStr(vector<shingle_hash> diff_shingle) {
//    vector<string> terminal_str;
//    vector<shingle_hash> tmpset;
//    if (myTree.empty()) throw invalid_argument("getTerminalDiffStr needs myTree with content");
//    size_t lvl = myTree.size() - 1;
//    for(auto shingle : diff_shingle){
//        if(shingle.lvl == lvl) tmpset.push_back(shingle);
//    }
//    for (size_t hash_item: unique_substr_hash(tmpset)) {
//        terminal_str.push_back(Dictionary[hash_item]);
//    }
//
//    return terminal_str;
//}


// functions for backtracking
bool SetsOfContent::shingle2hash_train(cycle& cyc_info, set<shingle_hash> shingle_set, vector<size_t>& final_str) {

//    // edge case if there is only one shinlge in the set
//    if (shingle_set.empty()) throw invalid_argument("Nothing is passed into shingle2hash_train");
//
//    if (shingle_set.size() == 1) {// edge case of just one shingle
//        if (cyc_info.cyc == 0) {// we find cycle number
//            cyc_info.cyc = 1;
//            cout<<"shingle2hash_train, i should never happend"<<endl; // delete if proven tobe useless
//            return true;
//        } else { // we find string
//            return true;
//        }
//    }
    auto changed_shingle_set = tree2shingle_dict(shingle_set); // get a shingle dict from a level of a tree for fast next edge lookup

    if (changed_shingle_set.empty()) throw invalid_argument("the shingle_vec provided is empty for shingle2hash_train");

    vector<map<size_t, vector<shingle_hash>>> stateStack;
    vector<vector<shingle_hash>> nxtEdgeStack;
    stateStack.push_back(changed_shingle_set);// Init Original state
    size_t strCollect_size = 0, curEdge =0;
    vector<size_t> str; // temprary string hash train to last be compared/placed in final_str


    for (auto head_shingles : changed_shingle_set[(size_t)0]) {
        if (cyc_info.head == head_shingles.second) {
            if (curEdge!=0)
                throw invalid_argument(
                        "multiple heads? look into it"); // TODO: delete and add break if never triggered.
            str.push_back(head_shingles.second);
            curEdge = head_shingles.second;
        }
    }


    if (cyc_info.cyc == 0) { // find head from "final_str" (we are finding cycle number)
        //if we only have one, then we are done with cycle number one and head ==tail
        if (final_str.size() == 1 && cyc_info.head == cyc_info.tail) {
            cyc_info.cyc = 1;
            return true;
        }
    } else if (cyc_info.cyc > 0) {// find head from "final_str" (we are retrieving the string from cycle number)
        if (cyc_info.cyc == 1 && cyc_info.head == cyc_info.tail) {
            return true;
        }
    }
    //    Resources initRes;
//    initResources(initRes); // initiate Recourses tracking


    shingle_hash last_edge;

    while (!stateStack.empty() and stateStack.size() == nxtEdgeStack.size() + 1) { // while state stack is not empty
        vector<shingle_hash> nxtEdges = stateStack.back()[curEdge];

        if (!nxtEdges.empty()) { // If we can go further with this route
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
            //(!stateStack.empty()) ? stateStack.push_back(stateStack.back()) : stateStack.push_back(origiState);
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
        stateStack.push_back(stateStack.back());
        for (auto &tmp_shingle: stateStack.back()[last_edge.first]) {
            if (tmp_shingle == last_edge) {
                tmp_shingle.occurr--;
                break;
            }
        }


//
//        if (!resourceMonitor( initRes, MAX_TIME, MAX_VM_SIZE))
//            return false;

        // if we reached a stop point
        if (curEdge == cyc_info.tail) {
            strCollect_size++;
            if (str == final_str || (strCollect_size == cyc_info.cyc and cyc_info.cyc != 0)) {
                cyc_info.cyc = strCollect_size;
                final_str = str;
            }
        }

        if (strCollect_size == cyc_info.cyc && cyc_info.cyc != 0) {
            return true;
        }
    }
    return false;
}

//bool SetsOfContent::empty_state(vector<shingle_hash> state) {
//    for (shingle_hash item : state) {
//        if (item.occurr > 0) return false;
//    }
//    return true;
//}

std::map<size_t, vector<shingle_hash>> SetsOfContent::tree2shingle_dict(std::set<shingle_hash> tree_lvl) {
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

//string SetsOfContent::get_str_from(vector<shingle_hash> shingle_set) {
//    vector<size_t> final_str;
//    string recovered_str;
//    for (shingle_hash& shingle : shingle_set){
//        if (shingle.cycleVal>0) { // this means it is the head
//
//            shingle.occurr--;
//            final_str.push_back(shingle.first);
//            final_str.push_back(shingle.second);
//            shingle2hash_train(shingle_set,shingle.cycleVal,final_str);
//            final_str.insert(final_str.begin(),shingle.second);
//            final_str.insert(final_str.begin(),shingle.first);
//            break; // there should not be any other shingles that has it
//        }
//    }
//    for (size_t substr_hash : final_str){
//        if (substr_hash!=0 and dictionary[substr_hash].empty())cout<<"what is this on other tree: "<<substr_hash<<endl;
//        recovered_str+=dictionary[substr_hash];
//    }
//    return recovered_str;
//}
//
//vector<string> SetsOfContent::get_all_strs_from(vector<shingle_hash> level_shingle_set) {
//    // assume strings are from the same level
//    vector<string> res;
//    map<size_t,vector<shingle_hash>> shingle_group;
//    for(shingle_hash shingle:level_shingle_set){
//        // report none-unique group ID, there should no be any, since only the unique strings get expanded, If none unique, it is the group ID
//        shingle_group[shingle.groupID].push_back(shingle);
//    }
//    // update dictionary
//    for(auto group:shingle_group){
//        res.push_back(get_str_from(group.second));
//        add_to_dictionary(res.back());
//    }
//    return res;
//}

string SetsOfContent::retriveString() {
    // retrace cycles bottom-up and delete from query after tracing
    string substring;
//    for (auto lvl : theirTree) for (auto item:lvl) cout << item << endl; // TODO: detele this print tree function
    for (int i = theirTree.size() - 1; i >= 0; --i) {
        for (shingle_hash shingle : theirTree[i]) {
            auto it = cyc_query.find(shingle.second);
            if (it != cyc_query.end()) {
                vector<size_t> tmp;
                substring = "";
                cycle tmp_cyc = cycle{.head = shingle.compose.head, .tail=shingle.compose.tail, .cyc = it->second};
                shingle2hash_train(tmp_cyc, theirTree[i + 1], tmp);
                for (size_t hash:tmp) {
                    if (Dictionary.find(hash) == Dictionary.end())
                        cout << "Recover may have failed - Dictionary lookup failed" << endl;
                    substring += Dictionary[hash];
                }
                add_to_dictionary(substring);
            }
        }
    }

    return substring;


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


    for (DataObject *dop : setPointers) delete dop;
    for (ZZ item : getShingles_ZZ()) {
        setPointers.push_back(new DataObject(item));
    }
    datum = setPointers;


    return true;
}
void SetsOfContent::SendSyncParam(const shared_ptr<Communicant> &commSync, bool oneWay) {
    Logger::gLog(Logger::METHOD,"Entering SendSyncParam::SendSyncParam");
    // take care of parent sync method
    SyncMethod::SendSyncParam(commSync);
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
    SyncMethod::RecvSyncParam(commSync);

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

bool SetsOfContent::SyncServer(const shared_ptr<Communicant> &commSync, shared_ptr<SyncMethod> &setHost) {
    bool success = false;
    Logger::gLog(Logger::METHOD, "Entering SetsOfContent::SyncServer");

    commSync->commListen();
    long mbar;
    if(GenSync::SyncProtocol::IBLTSyncSetDiff == baseSyncProtocol){
        StrataEst est = StrataEst(sizeof(shingle_hash));

        for (auto item : setPointers) {
            est.insert(item);
        }

        // since Kshingling are the same, Strata Est parameters would also be the same.
        auto theirStarata = commSync->commRecv_Strata();
        mbar = (est -= theirStarata).estimate();
        commSync->commSend(mbar); // Dangerous cast
    }


    RecvSyncParam(commSync);
    SyncMethod::SyncServer(commSync, setHost);

    configure(setHost, mbar);

    for (DataObject *dop : setPointers) {
        setHost->addElem(dop); // Add to GenSync
    }
    list<DataObject *> mine, others;
    vector<shingle_hash> theirs_hash, mine_hash;

    setHost->SyncServer(commSync, mine, others);
    for (auto shingle : others) theirs_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));
    for (auto shingle : mine) mine_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));

    prepare_concerns(theirs_hash,mine_hash);

    cout<< "Server - cyc concern size : "<< cyc_concern.size()<<endl;
    cout<< "Server - Term concern size : "<< term_concern.size()<<endl;


    for (auto groupcyc : cyc_concern){
        commSync->commSend((long)groupcyc.second);
//        cout<<groupcyc.first<<":"<<groupcyc.second<<endl;
    }

    for (auto dic : term_concern) {
        if (!Dictionary[dic.first].empty())
            commSync->commSend(Dictionary[dic.first]);
        else
            commSync->commSend("$");
    }

//    commSync->commSend(SYNC_SUCCESS);
    cout<<"Server Close"<<endl;
    Logger::gLog(Logger::METHOD, "SetOfCeontent Done");
    commSync->commClose();
    return success;
}

bool SetsOfContent::SyncClient(const shared_ptr<Communicant> &commSync, shared_ptr<SyncMethod> &setHost) {
    //TODO: needs a flag, but  this will do for now
    bool success = false;
    Logger::gLog(Logger::METHOD, "Entering SetsOfContent::SyncClient");

    commSync->commConnect();
    long mbar;

    if (GenSync::SyncProtocol::IBLTSyncSetDiff == baseSyncProtocol){
        StrataEst est = StrataEst(sizeof(shingle_hash));

        for (auto item :setPointers) {
            est.insert(item); // Add to estimator
        }

        // since Kshingling are the same, Strata Est parameters would also be the same.
        commSync->commSend(est.getStrata(), false);

        mbar = commSync->commRecv_long(); // cast long to long long

    }

    SendSyncParam(commSync);
    SyncMethod::SyncClient(commSync, setHost);

    configure(setHost, mbar);

    for (DataObject *dop : setPointers) {
        setHost->addElem(dop); // Add to GenSync
    }
    list<DataObject *> mine, others;
    vector<shingle_hash> theirs_hash, mine_hash;

    setHost->SyncClient(commSync, mine, others);
    for (auto shingle : others)
        theirs_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));
    for (auto shingle : mine) mine_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));



    prepare_querys(theirs_hash,mine_hash);

    cout<< "cyc query size : "<< cyc_query.size()<<endl;
    cout<< "Term query size : "<< term_query.size()<<endl;


    for(auto& cyc:cyc_query){
        cyc.second = commSync->commRecv_long();
    }

    for (int i = 0; i < term_query.size(); ++i) {
        auto tmp = commSync->commRecv_string();
        if (tmp != "$")
            add_to_dictionary(tmp);
    }

    success = true;
    cout<<"Client Close"<<endl;
    Logger::gLog(Logger::METHOD, "Set Of Content Done");
    commSync->commClose();
    return success;
}


void SetsOfContent::configure(shared_ptr<SyncMethod>& setHost, long mbar) {
    if (GenSync::SyncProtocol::IBLTSyncSetDiff == baseSyncProtocol)
        setHost = make_shared<IBLTSync_SetDiff>(mbar, sizeof(shingle_hash), true);
    else if (GenSync::SyncProtocol::InteractiveCPISync == baseSyncProtocol)
        setHost = make_shared<InterCPISync>(5, sizeof(shingle_hash) * 8, 64, 7, true);
}

bool SetsOfContent::reconstructString(DataObject *&recovered_string, const list<DataObject *> &theirsMinusMine,
                                      const list<DataObject *> &mineMinusTheirs) {
    myString = retriveString();
    recovered_string = new DataObject(myString);
    return true;
}
