//
// Created by Bowen Song on 12/9/18.
//

#include "SetsOfContent.h"
SetsOfContent::SetsOfContent(size_t terminal_str_size, size_t levels, int partition, GenSync::SyncProtocol base_set_proto)
: TermStrSize(terminal_str_size), Levels(levels), Partition(partition), baseSyncProtocol(base_set_proto){
    SyncID = SYNC_TYPE::SetsOfContent;
}

SetsOfContent::~SetsOfContent() {
    for (DataObject* dop : setPointers) delete dop;
}

vector<size_t> SetsOfContent::create_HashSet(string str,size_t win_size, size_t space, size_t shingle_size) {
    // space and shingle_size has to match between reconciling strings

    // if the substring is smaller than the terminla stringh size, we do not partition it anymore.

    if (str.size() <= TermStrSize) return vector<size_t>({str_to_hash(str)});
    vector<size_t> hash_val, hash_set;

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
    (dictionary[str_to_hash(str)].empty() or dictionary[str_to_hash(str)] == str) ? dictionary[str_to_hash(str)] = str
                                                                                  : throw invalid_argument(
            "Dictionary duplicated suggest using new/multiple hashfunctions");
    return str_to_hash(str);
}

void SetsOfContent::go_through_tree(bool get_string_cycles) {
    myTree.clear();
    auto String_Size = pow(Partition,Levels)*TermStrSize; // calculate a supposed string size, a string size that make sense with the parameters

    size_t shingle_size = floor(log2(String_Size));

    if (shingle_size < 2) throw invalid_argument("Consider larger the paramters for auto shingle size to be more than 2");


    size_t space = TermStrSize * 126; //126 for ascii content

    // fill up the tree
    myTree.resize(Levels);

    clock_t lvl1_t = clock();

    auto first_level = create_HashSet(myString, floor((String_Size / Partition) / 2), space, shingle_size);
    update_tree(first_level, 0, get_string_cycles);

//    cout << "Lvl 1 time with BackTracking: " << (double) (clock() - lvl1_t) / CLOCKS_PER_SEC << endl;

//    clock_t all_lvls_t = clock();
    for (int l = 1; l < Levels; ++l) {
        space = floor((space / Partition) / 2);
        shingle_size = floor(shingle_size / 2);
        for (auto substr_hash:unique_substr_hash(myTree[l - 1])) {
            string substring = dictionary[substr_hash];
            auto tmp_level = create_HashSet(substring, floor((substring.size() / Partition) / 2),
                                            space, shingle_size);
            update_tree(tmp_level, l, get_string_cycles);
        }

    }
//    cout<<myTree[0].size()<<":"<<myTree[1].size()<<":"<<myTree[2].size()<<endl;
//    cout << "Rest of the Lvls time with BackTracking: " << (double) (clock() - all_lvls_t) / CLOCKS_PER_SEC << endl;
}

void SetsOfContent::redo_tree_with_cyc() {
//
//    map<size_t, size_t> groupPair;
//
//    if (!Rev_Group.empty()) {
//        for(auto item : Rev_Group){
//            Req_Group.push_back(item.first);
//        }
//    }

//
//    for (size_t cyc : Req_Group) groupPair[cyc] = 0; //TODO: this is only the  one way case
//    for (int i = 0; i < get_Cyc.size(); ++i) {
//        groupPair[Req_Group[i]] = get_Cyc[i];
//    }
//

//    if(!Rev_Group.empty()) {
//        for (auto cyc : Rev_Group) groupPair[cyc.first] = 0;
//        for (int i = 0; i < get_Cyc.size(); ++i) {
//            groupPair[Rev_Group.at(i).first] = get_Cyc[i];
//        }
//    }

    for (auto &lvl:theirTree) { // fill their tree with cycle val
        for (auto &item:lvl) {
            if (my_group_of_query[item.groupID] and item.groupID == item.first) {
                item.cycleVal = my_group_of_query[item.groupID];
            }
        }
    }
}
// what i am missing, and what they would be sending to me
void SetsOfContent::single_out_querys(vector<shingle_hash> shingle_hash_theirs, vector<shingle_hash> shingle_hash_mine) {
// get the tree ready
// know what to expect by getting  my_group_of_query ready

//
//    map<size_t, vector<shingle_hash>> mine_rid;
    theirTree.resize(myTree.size());// resize their tree
    map<shingle_hash,bool> isExist, rid_shingles;
    if (my_group_of_query.size()>0) throw invalid_argument("we are not supposed to chance this before single_out_querys");

    for (shingle_hash shingle:shingle_hash_theirs) { // put what i learnt from set recon in their tree
        theirTree[shingle.lvl].push_back(shingle);
        my_group_of_query[shingle.groupID] = 1; // first time editing
        isExist[shingle] = true;
    }


    for(shingle_hash shingle : shingle_hash_mine){
        rid_shingles[shingle] = true;
    }

    for (auto tree_lvl : myTree) { // if it is the same ID, put the shingle in their tree
        for (shingle_hash shingle: tree_lvl) {
            if (!isExist[shingle] and !rid_shingles[shingle] and my_group_of_query[shingle.groupID] > 0) {
                theirTree[shingle.lvl].push_back(shingle);
            }
            else if (my_group_of_query[shingle.groupID] == 0){
                my_group_of_query.erase(shingle.groupID);
            }

        }
    }

    if (myTree.front().size()!=theirTree.front().size()) cout<<"Our tree top are not the same, Set sync might have failed"<<endl;
//
//    for (shingle_hash shingle : shingle_hash_mine) { // put my shingles no in their tree in a map for letter to get rid off
//        mine_rid[shingle.first + shingle.second * 2].push_back(shingle);
//    }
//
//    map<size_t, bool> conceredPool;
//    for (shingle_hash shingle : shingle_hash_theirs) {
//        conceredPool[shingle.groupID] = true;
//    }
//    for (auto id :  conceredPool) {
//        groupIDs.push_back(id.first);
//    }
//
//
//    for (auto tree_lvl : myTree) { // transfer tree
//        for (shingle_hash shingle: tree_lvl) {
//            if (mine_rid[shingle.first + shingle.second * 2].empty() and
//                conceredPool[shingle.groupID]) { // if it is not in the rid list and it is part of concerned gropup: add it in
//                theirTree[shingle.lvl].push_back(shingle);
//            } else { // else, if we are sure,  don't add it
//                for (shingle_hash ridshingle: mine_rid[shingle.first + shingle.second * 2]) {
//                    if (shingle == ridshingle or !conceredPool[shingle.groupID]) break;
//                    theirTree[shingle.lvl].push_back(shingle);
//                }
//            }
//
//        }
//    }        //Their Tree is now complete
//
//    for (shingle_hash shingle : shingle_hash_theirs) { // put their shingles in their shingle tree at my locol host
//        theirTree[shingle.lvl].push_back(shingle);
//    }
//
//    return unique_substr_hash(theirTree.back());

}

// what they are missing, and i will prepare to send them
void SetsOfContent::single_out_concerns(vector<shingle_hash> shingle_hash_theirs, vector<shingle_hash> shingle_hash_mine) {

    // figure out what they are missing, on cycles and dictionary
    // later send the information over
//map<shingle_hash,bool> mine_unique_MAP;
    for (shingle_hash shingle : shingle_hash_mine) {
        my_group_of_concern[shingle.groupID] = true; // record all group ids from mine - theirs pile
        if (shingle.lvl==myTree.size()-1) { // get all the shingles first , then delete from commons
            my_dic_of_concern[shingle.first] = true;
            my_dic_of_concern[shingle.second] = true;
        }
//        mine_unique_MAP[shingle] = true;
    }
    //TODO: need to prove that it does no harm by first make this work and enable it
//    cout<<"my_dic_of_concern: "<<my_dic_of_concern.size()<<endl;
//    for (auto tree_lvl : myTree) { // get rid of dictionaries that is known by the other side
//        for (shingle_hash shingle: tree_lvl) {
//            if (!mine_unique_MAP[shingle]) {
//                 my_dic_of_concern.erase(shingle.first);
//                my_dic_of_concern.erase(shingle.second);
//            }
//        }
//    }
//    cout<<"my_dic_of_concern: "<<my_dic_of_concern.size()<<endl;

}

void SetsOfContent::update_tree(vector<size_t> hash_vector, int level, bool isComputeCyc) {
    if (myTree.size() <= level) throw invalid_argument("We have exceeded the levels of the tree");
    if (hash_vector.size() > 100)
        cout
                << "It is advised to not excceed 100 partitions for fast backtracking at Level: " + to_string(level) +
                   " Current set size: " + to_string(hash_vector.size()) << endl;
    if (hash_vector.empty())
        throw invalid_argument("hash_vector is zero at level:" + to_string(level) +
                               ". All termianl strgins to be passed down to the bottom level");

    map<pair<size_t, size_t>, int> tmp;
    for (int i = 0; i < hash_vector.size() - 1; ++i) {
        if (tmp[{hash_vector[i], hash_vector[i + 1]}])
            tmp[{hash_vector[i], hash_vector[i + 1]}]++;
        else
            tmp[{hash_vector[i], hash_vector[i + 1]}] = 1;
    }

    size_t group_id = get_group_signature(hash_vector);

    // incase a string was not partitioned based on the given window and space size, (either, string is small enough, or it would be partitioned again in a subsequent level)
    if (tmp.empty()) {
        // passs down termianl string, if not partitioned
        if (hash_vector.empty()) throw invalid_argument("inherited empty hash vector at level: " + to_string(level));
        myTree[level].emplace_back(
                shingle_hash{.first = hash_vector.back(), .second = 0, .occurr = 1, .groupID = group_id, .cycleVal = 1, .lvl = level});

        if (isComputeCyc and my_group_of_concern[group_id] ) {
            my_group_of_concern_tosend[group_id] = 1; // set the cycle number at the head shingle
        }

        return;
    }

    vector<shingle_hash> lst;

    for (auto item = tmp.begin(); item != tmp.end(); ++item) {
        lst.emplace_back(
                shingle_hash{.first = item->first.first, .second = item->first.second, .groupID = group_id, .occurr = item->second, .cycleVal = 0, .lvl = level});
    }

    if (isComputeCyc and my_group_of_concern[group_id] ) {
        my_group_of_concern_tosend[group_id] = set_cyc_val(lst, hash_vector); // set the cycle number at the head shingle
    }


//    sort(lst.begin(), lst.end()); // sorting might be unnecessary here due to the use of hashmap above
    //order in lexicographical order for backtracking (sort once at insert, sort another time at reconstruction)

    myTree[level].insert(myTree[level].end(), lst.begin(), lst.end()); // put it in the tree
}

size_t SetsOfContent::get_group_signature(vector<size_t> strordered_hashset) {
    // cover one element situation as well
    // make sure unique
    // make usre it can just be 0 by chance which is not going to be unique
    return strordered_hashset.front();
}

vector<string> SetsOfContent::getTerminalDiffStr(vector<shingle_hash> diff_shingle) {
    vector<string> terminal_str;
    vector<shingle_hash> tmpset;
    if (myTree.empty()) throw invalid_argument("getTerminalDiffStr needs myTree with content");
    size_t lvl = myTree.size() - 1;
    for(auto shingle : diff_shingle){
        if(shingle.lvl == lvl) tmpset.push_back(shingle);
    }
    for (size_t hash_item: unique_substr_hash(tmpset)) {
        terminal_str.push_back(dictionary[hash_item]);
    }

    return terminal_str;
}


// functions for backtracking
bool SetsOfContent::shingle2hash_train(vector<shingle_hash> shingle_set, int &str_order,
                                                                    vector<size_t>& final_str) {

    // edge case if there is only one shinlge in the set
    if (shingle_set.empty()) throw invalid_argument("Nothing is passed into shingle2hash_train");

    if (shingle_set.size() == 1) {// edge case of just one shingle
        if (str_order == 0) {// we find cycle number
            str_order = 1;
            return true;
        } else { // we find string
            return true;
        }
    }



    // nxtEdgeStack: [[idx of nxt edges];[];...] Register the state of nxt possible edges
    // stateStack: [[occr of shingles ];[];...] Register the state of shigle set occurrences
    vector<vector<size_t>> nxtEdgeStack;
    vector<vector<shingle_hash>> stateStack;
    size_t curEdge, strCollect_size = 0;
    vector<size_t> str; // trmprary string hash train to last be compared/placed in final_str

    if (str_order == 0) { // find head from "final_str" (we are finding cycle number)
        for (shingle_hash &shingle : shingle_set) {
            if (shingle.first == final_str[0] and shingle.second == final_str[1]) {
                shingle.occurr--;
                str.push_back(shingle.first);
                str.push_back(shingle.second);
                curEdge = shingle.second;
                break;
            }
        }
    } else {// else find head is already inserted (we are retrieving string)
        curEdge = final_str[1];
    }
    Resources initRes;
    initResources(initRes); // initiate Recourses tracking


    // Init Original state
    stateStack.push_back(shingle_set);
    size_t nxt_idx = 0;
    while (!stateStack.empty() and stateStack.size() == nxtEdgeStack.size() + 1) { // while state stack is not empty
        vector<size_t> nxtEdges = get_nxt_edge_idx(curEdge, stateStack.back());

        if (!nxtEdges.empty()) { // If we can go further with this route
            nxt_idx = nxtEdges.back();
            nxtEdges.pop_back();
            nxtEdgeStack.push_back(nxtEdges);
        } else if (!nxtEdgeStack.empty() and stateStack.size() == nxtEdgeStack.size() + 1 and
                   !nxtEdgeStack.back().empty()) { //if this route is dead, we should look for other options
            if (!str.empty()) str.pop_back();

            //look for other edge options
            nxt_idx = nxtEdgeStack.back().back();
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
                nxt_idx = nxtEdgeStack.back().back();
                nxtEdgeStack.back().pop_back();
                stateStack.pop_back();
            }
        } else if (stateStack.size() != nxtEdgeStack.size() + 1) {
            throw invalid_argument("state stack and nxtEdge Stack size miss match" + to_string(stateStack.size())
                                   + ":" + to_string(nxtEdgeStack.size()));
        }

        str.push_back(shingle_set[nxt_idx].second);

        // Change and register our state for shingle occurrence and nxt edges
        stateStack.push_back(stateStack.back());
        stateStack.back()[nxt_idx].occurr--;

        curEdge = shingle_set[nxt_idx].second;

        if (!resourceMonitor( initRes, MAX_TIME, MAX_VM_SIZE))
            return false;

        // if we reached a stop point
        if (empty_state(stateStack.back())) {
            strCollect_size++;
            if (str == final_str || (strCollect_size == str_order and str_order != 0)) {
                str_order = strCollect_size;
                final_str = str;
            }
        }

        if (strCollect_size == str_order && str_order != 0) {
            return true;
        }
    }
    return false;
}

bool SetsOfContent::empty_state(vector<shingle_hash> state) {
    for (shingle_hash item : state) {
        if (item.occurr > 0) return false;
    }
    return true;
}

vector<size_t> SetsOfContent::get_nxt_edge_idx(size_t current_edge, vector<shingle_hash> changed_shingleOccur) {
    vector<size_t> tmplst;
    for (size_t i = 0; i < changed_shingleOccur.size(); ++i){
        if (changed_shingleOccur[i].first == current_edge && changed_shingleOccur[i].occurr > 0){
            tmplst.push_back(i);
        }
    }
    return tmplst;
}

size_t SetsOfContent::set_cyc_val(vector<shingle_hash> &shingle_set,
                                          vector<size_t> strordered_substr_hash) {
    int cycnum = 0;

    shingle2hash_train(shingle_set,cycnum,strordered_substr_hash); // get the cycnum
    if (cycnum == 264)
        cout<<"My 256 size: "<<strordered_substr_hash.size()<<endl;

    for(shingle_hash& shingle : shingle_set){
        if(shingle.first==strordered_substr_hash[0] and shingle.second==strordered_substr_hash[1]){
            shingle.cycleVal = cycnum; // put in the head
        }
    }
    return cycnum;
}

string SetsOfContent::get_str_from(vector<shingle_hash> shingle_set) {
    vector<size_t> final_str;
    string recovered_str;
    for (shingle_hash& shingle : shingle_set){
        if (shingle.cycleVal>0) { // this means it is the head

            if (shingle.cycleVal == 264)
                cout<<"My 256 size: "<<shingle_set.size()<<endl;
            shingle.occurr--;
            final_str.push_back(shingle.first);
            final_str.push_back(shingle.second);
            shingle2hash_train(shingle_set,shingle.cycleVal,final_str);
            final_str.insert(final_str.begin(),shingle.second);
            final_str.insert(final_str.begin(),shingle.first);
            if (shingle.cycleVal == 264) for (auto ss : final_str) cout<<"Did it again here"<<ss<<endl;
            break; // there should not be any other shingles that has it
        }
    }
    for (size_t substr_hash : final_str){
        if (substr_hash!=0 and dictionary[substr_hash].empty())cout<<"what is this on other tree: "<<substr_hash<<endl;
        recovered_str+=dictionary[substr_hash];
    }
    return recovered_str;
}

vector<string> SetsOfContent::get_all_strs_from(vector<shingle_hash> level_shingle_set) {
    // assume strings are from the same level
    vector<string> res;
    map<size_t,vector<shingle_hash>> shingle_group;
    for(shingle_hash shingle:level_shingle_set){
        // report none-unique group ID, there should no be any, since only the unique strings get expanded, If none unique, it is the group ID
        shingle_group[shingle.groupID].push_back(shingle);
    }
    // update dictionary
    for(auto group:shingle_group){
        res.push_back(get_str_from(group.second));
        add_to_dictionary(res.back());
    }
    return res;
}

string SetsOfContent::retriveString() {
    for (int i = theirTree.size()-1; i > 0; --i){
        get_all_strs_from(theirTree[i]);
    }
    return get_all_strs_from(theirTree[0]).back();
}


// functions for  SyncMethods
bool SetsOfContent::addStr(DataObject *str_p, vector<DataObject *> &datum, bool sync) {
    myString = str_p->to_string();
    SyncMethod::addStr(str_p, datum, sync);
    if (myString.empty()) return false;

    if (myString.size()/pow(Partition,Levels)< 1) throw invalid_argument("Terminal String size would end up less than 1, please consider lessen the levels");

    if (Levels == NOT_SET) throw invalid_argument("Consider set a Level value bigger than 0");


    go_through_tree(false);


    for (DataObject* dop : setPointers) delete dop;
    for (ZZ item : getShingles_ZZ()){
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

    cout<<"Server has: "<<mine.size()<<"other: "<<others.size()<<endl;

    // get request
//    size_t group_Size = commSync->commRecv_size_t();
//    size_t dict_Size = commSync->commRecv_size_t();
//    for (int i = 0; i < dict_Size; ++i) {
//        Rev_Dict.push_back(commSync->commRecv_size_t());
//    }
//    for (int i = 0; i < group_Size; ++i) {
//        Rev_Group[commSync->commRecv_size_t()] = true;
//    }

    single_out_concerns(theirs_hash,mine_hash);

    go_through_tree(true);

    for (auto groupcyc : my_group_of_concern_tosend){
        commSync->commSend(groupcyc.second);
//        cout<<groupcyc.first<<":"<<groupcyc.second<<endl;
    }
    int dic_size = my_dic_of_concern.size();
    commSync->commSend(dic_size);
    for (auto dic : my_dic_of_concern) {
        if (!dictionary[dic.first].empty())
            commSync->commSend(dictionary[dic.first]);
        else
            commSync->commSend("$");
    }
//
//    commSync->commSend((size_t) Dif_Group.size());
//    commSync->commSend((size_t) Dif_Dict.size());
//
//    for (long cyc : Dif_Group) {
//        commSync->commSend(cyc);
//    }
//    for (auto dic : Dif_Dict) {
//        string tmp = dictionary[dic];
//        if (!tmp.empty())commSync->commSend(tmp);
//        else commSync->commSend("$");
//
//    }


//

    success=(SYNC_SUCCESS==commSync->commRecv_int());
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
    for (auto shingle : others) theirs_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));
    for (auto shingle : mine) mine_hash.push_back(ZZtoShingleHash(shingle->to_ZZ()));

    cout<<"Client has: "<<mine.size()<<"other: "<<others.size()<<endl;

    single_out_querys(theirs_hash,mine_hash);



    for(auto& cyc:my_group_of_query){
        cyc.second = commSync->commRecv_int();
//        cout<<cyc.first<<":"<<cyc.second<<endl;
    }

    size_t dict_Size = commSync->commRecv_int();
//    for (int i = 0; i < Rev_Group.size(); ++i) {
//        get_Cyc.push_back(commSync->commRecv_long());
//    }
    for (int i = 0; i < dict_Size; ++i) {
        auto tmp = commSync->commRecv_string();
        if (tmp != "$")
            add_to_dictionary(tmp);
    }


    commSync->commSend(SYNC_SUCCESS);
    success = true;
//    success=(SYNC_SUCCESS==commSync->commRecv_int());
    cout<<"Client Close"<<endl;
    Logger::gLog(Logger::METHOD, "SetOfCeontent Done");
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

    // send group IDs and Concerned Strings as request
//    for(size_t item:group_ids) send
    redo_tree_with_cyc();
    myString = retriveString();
    recovered_string = new DataObject(myString);
    return true;
}
