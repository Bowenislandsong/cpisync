//
// Created by Bowen Song on 12/9/18.
//

#ifndef CPISYNCLIB_SETSOFCONTENT_H
#define CPISYNCLIB_SETSOFCONTENT_H


#include <vector>
#include <string>
#include <NTL/ZZ.h>
#include "kshingling.h"
#include "StrataEst.h"
#include "CPI.h"
#include "SyncMethod.h"
#include "DataObject.h"
#include <algorithm>
#include <NTL/ZZ_p.h>
#include "Exceptions.h"
#include "InterCPISync.h"
#include "IBLTSync_SetDiff.h"
#include "ProbCPISync.h"
#include "FullSync.h"

#include "ForkHandle.h" // tobe removed

using std::vector;
using std::hash;
using std::string;
using namespace NTL;
typedef unsigned short sm_i; // small index for lvl, MAX = 65,535 which is terminal string size * partition^lvl = file string size

// describes a cycling using first and last and cycle number. Used in backtracking. We pointou the first, then the last, then cycle value./
struct cycle {
    size_t head, tail, cyc;
};

static bool operator==(const cycle& a, const cycle& b){
    return a.head == b.head and a.tail == b.tail and a.cyc == b.cyc;
};

static bool operator!=(const cycle& a, const cycle& b) {
    return !(a == b);
};

static ostream& operator<<(ostream &os, const cycle cyc) {
    os << "head: " + to_string(cyc.head) + ", tail: " + to_string(cyc.tail) + ", cyc: " +
          to_string(cyc.cyc);
    return os;
};


static vector<size_t> recursion(vector<size_t> it){
    std::vector<unsigned char> udata;
    for (int i = 0; i < it.size(); ++i) {
        std::vector<unsigned char> foo(sizeof(size_t));
//        ::memcpy(foo.data(), &it[i], sizeof(size_t));
        udata.insert(udata.end(),foo.begin(),foo.end());
    }

    ZZ tmp = ZZFromBytes((const unsigned char *) &udata[0], udata.size());


    vector<unsigned char> tmp_vec;
    tmp_vec.resize(NumBytes(tmp)+1,0);
    BytesFromZZ((unsigned char *) &tmp_vec[0],tmp,tmp_vec.size());
    vector<size_t> res;
    for (size_t j = 0; j<tmp_vec.size(); j+=sizeof(size_t)) {
        res.push_back(*reinterpret_cast<size_t*>(&vector<unsigned char>{tmp_vec.begin()+j,tmp_vec.begin()+j+sizeof(size_t)}[0]));
    }

    return res;
}

static ZZ size_t_vecToZZ(vector<size_t> s_vec){
    std::vector<unsigned char> udata;
    for (int i = 0; i < s_vec.size(); ++i) {
        size_t a = s_vec[i];
//        std::vector<unsigned char> foo(sizeof(size_t));
        auto foo=reinterpret_cast<unsigned char*>(&a);
        vector<uint8_t> tmp{foo, foo + sizeof(size_t)};
        udata.insert(udata.end(),tmp.begin(),tmp.end());
    }
    return ZZFromBytes((const unsigned char *) &udata[0], udata.size());
}

static vector<size_t> ZZTosize_t_vec(ZZ zz) {
    vector<unsigned char> tmp_vec;
    tmp_vec.resize(16,0);
    BytesFromZZ((unsigned char *) &tmp_vec[0],zz,tmp_vec.size());
    vector<size_t> res;
    for (size_t j = 0; j<tmp_vec.size(); j+=sizeof(size_t)) {
        res.push_back(*reinterpret_cast<size_t*>(&vector<unsigned char>{tmp_vec.begin()+j,tmp_vec.begin()+j+sizeof(size_t)}[0]));
    }
    return res;
}


/**
 * first is the first part of a shingle
 * second is the second part of the shingle
 * occurrence, how manny time this occrred
 * compose states the composite of second
 */
struct shingle_hash{
    vector<size_t> first;
    size_t second, occurr;
    cycle compose;
    sm_i lvl;
};

// see if a is followed by b. used in kshingling
static bool isFollowed(const shingle_hash& a, const shingle_hash& b) {
    if (a.first.size() != b.first.size())throw invalid_argument("Comparing different shingles in isFollowed fxn");

    if (b.first.size() == 1)
        return (a.second == b.first.back());
    else if (b.first.size() > 1)
        return (a.second == b.first.back() and std::equal(b.first.begin(), b.first.end() - 1, a.first.begin() + 1));

    else throw invalid_argument("Comparing empty shingles in isFollowed fxn");
};


//Compare and help order struct shingle_hash from a vector
static bool operator<(const shingle_hash& a, const shingle_hash& b) {
    return (a.first < b.first) or
           (a.first == b.first and a.second < b.second) or
           (a.first == b.first and a.second == b.second and a.occurr < b.occurr);
    // we do not consider ordering cytcle since it would be same to ordering "second"
};


//Compare and help differetiate struct shingle_hash
static bool operator==(const shingle_hash& a, const shingle_hash& b) {
    return a.first == b.first and a.second == b.second and a.occurr == b.occurr and a.compose == b.compose and a.lvl == b.lvl;
};

static bool operator!=(const shingle_hash& a, const shingle_hash& b) {
    return !(a == b);
};

struct packet{
    ZZ first;
    size_t second, occurr;
    cycle compose;
    sm_i lvl;
};

static shingle_hash ZZtoShingleHash(const ZZ& zz){
    packet p;
    BytesFromZZ((uint8_t *) &p, zz, sizeof(packet));
    return shingle_hash{.first = ZZTosize_t_vec(p.first), .second = p.second,.lvl = p.lvl, .occurr = p.occurr,
            .compose = cycle{.cyc= p.compose.cyc, .head = p.compose.head, .tail = p.compose.tail}};


//    shingle_hash shingle;
//    BytesFromZZ((uint8_t *) &shingle, zz, sizeof(shingle_hash));
//    return shingle;
}


static ZZ ShingleHashtoZZ(shingle_hash shingle) {
    packet p = packet{.first = size_t_vecToZZ(shingle.first), .second = shingle.second, .lvl = shingle.lvl, .occurr = shingle.occurr,
                      .compose = cycle{.cyc= shingle.compose.cyc, .head = shingle.compose.head, .tail = shingle.compose.tail}};
    char *my_s_bytes = reinterpret_cast<char *>(&p);
    return ZZFromBytes((const uint8_t *) my_s_bytes, sizeof(packet));

//    char *my_s_bytes = reinterpret_cast<char *>(&shingle);
//    return ZZFromBytes((const uint8_t *) my_s_bytes, sizeof(shingle_hash));
}


static string to_string(const vector<size_t> a) {string res; for (size_t t : a) res+=to_string(t)+":"; res.pop_back(); return res;};

static ostream& operator<<(ostream &os, const shingle_hash shingle) {
    os << "fst: " + to_string(shingle.first) + ", sec: " + to_string(shingle.second) + ", occ: " +
          to_string(shingle.occurr) + ", lvl: "+to_string(shingle.lvl) +" "<<shingle.compose;
    return os;
};

class SetsOfContent : public SyncMethod {
public:
    SetsOfContent(size_t terminal_str_size, size_t levels, size_t partition, GenSync::SyncProtocol base_set_proto, size_t shingle_size = 2);

    ~SetsOfContent();

    string retriveString();

// functions for SyncMethods
    bool addStr(DataObject* str, vector<DataObject*> &datum,  bool sync) override;

    bool SyncClient(const shared_ptr<Communicant> &commSync, shared_ptr<SyncMethod> &setHost) override;

    bool SyncServer(const shared_ptr<Communicant> &commSync, shared_ptr<SyncMethod> &setHost) override;

    string getName() override {return "Sets of Content";}

    /**
     * Get the terminal strings to reconcile with another set
     * @return a vector of terminla string
     */
    vector<string> getTerminalDiffStr(vector<shingle_hash> diff_shingle);

    //getShinglesAt
    vector<ZZ> getShingles_ZZ() {
        vector<ZZ> res;
//        std::map<ZZ,vector<shingle_hash>> check_dup; // Check Duplication TODO: Delete this
        for (auto treelvl : myTree) {
            for (auto item:treelvl) {
//                if (check_dup[ShingleHashtoZZ(item)].empty())
//                    check_dup[ShingleHashtoZZ(item)].push_back(item);
//                else {
//                    cout<<"we have duplicates"<<endl;
//                    cout << ShingleHashtoZZ(item) << endl; // Check Duplication TODO: Delete this
//                    check_dup[ShingleHashtoZZ(item)].push_back(item);
//                    auto it  = check_dup.find(ShingleHashtoZZ(item));
//                    cout<<it->first<<endl;
//                }
                res.push_back(ShingleHashtoZZ(item));
            }
        }
        return res;
    };


protected:
    bool useExisting; /** Use Exiting connection for Communication */

private:

    string myString; // original input string
    size_t TermStrSize, Levels, Partition, HashShingleSize;

    GenSync::SyncProtocol baseSyncProtocol;

    vector<DataObject*> setPointers; // garbage collector

    vector<std::set<shingle_hash>> myTree, theirTree; // the hash shingle tree   // RECONCILLING TARGET

    map<size_t, string> Dictionary;  // terminla strings

    // origin, cycle information to reform this string rep
    map<size_t, vector<size_t>> Cyc_dict; // has to be unique

    //requests
    map<size_t,string> term_concern, term_query;
    map<size_t,size_t> cyc_concern, cyc_query;

    size_t str_to_hash(string str) {
        return std::hash<std::string>{}(str);
    };

    /**
     * Create content dependent partitions based on the input string
     * Update Dictionary
     * @param str origin string to be partitioned
     * @param win_size partition size is 2*win_size
     * @param space smaller-more partitions
     * @param shingle_size inter-relation of the string
     * @return vector of substring hashes in origin string order
     */
    vector<size_t> create_HashSet(string str,size_t win_size, size_t space=NOT_SET, size_t shingle_size=NOT_SET);

    vector<size_t> local_mins(vector<size_t> hash_val, size_t win_size);

    /**
     * Insert string into dictionary
     * @param str a substring
     * @return hash of the string
     * @throw if there is duplicates, suggest using new/multiple hashfunctions
     */
    size_t add_to_dictionary(string str);

    inline size_t min_between(const vector<size_t> &nums,size_t from, size_t to){
        if (nums.empty()) throw invalid_argument("min function does not take empty vector");
        size_t min = nums[0];
        for (size_t i = from; i <= to; ++i) {
            if (nums[i]<min) min = nums[i];
        }
        return min;
    }

    // extract the unique substring hashes from the shingle_hash vector
    vector<size_t> unique_substr_hash(std::set<shingle_hash> hash_set) {
        set < size_t > tmp;
        for (shingle_hash item : hash_set) {
            for (auto sub_item: item.first)tmp.insert(sub_item);
            tmp.insert(item.second);
        }
        return vector<size_t>(tmp.begin(), tmp.end());

    }

    void update_tree_shingles(vector<size_t> hash_vector, sm_i level);

    // functions for backtracking
    /**
     * peice shingle_hashes back to a vector of hashes in the string order
     * @param shingle_set
     * @param str_order
     * @param final_str a hash train in string order
     */
    bool shingle2hash_train(cycle& cyc_info, set<shingle_hash> shingle_set, vector<size_t>& final_str);

    std::map<vector<size_t>, vector<shingle_hash>> tree2shingle_dict(std::set<shingle_hash> tree_lvl);

    shingle_hash get_nxt_edge(vector<size_t>& current_edge, shingle_hash _shingle);

    bool empty_state(vector<shingle_hash> state);

    /**
     * sub fucntion for "get_all_strs_from" extracting string from a group of shigle_hashes
     * @param shingle_set shinge_hashes from a group
     * @return the string of that group
     */
    string get_str_from(vector<shingle_hash> shingle_set);

    /**
     * get the full shigle_hash set from a level and spratet them by groups to feed into "get_str_from"
     * @param level_shingle_set shingle_hashes from a level
     * @return reconstruct string from the ground lvl
     */
    vector<string> get_all_strs_from(vector<shingle_hash> level_shingle_set);

    /**
     *
     * @param shingle_hash_theirs
     * @param shingle_hash_mine
     * @param groupIDs
     * @return hashes of unknown
     */
    void prepare_querys(vector<shingle_hash> shingle_hash_theirs, vector<shingle_hash> shingle_hash_mine);
    void prepare_concerns(vector<shingle_hash> shingle_hash_theirs, vector<shingle_hash> shingle_hash_mine);

    // helper function , extract all in a shingle into a map of bool
    void extract2map(shingle_hash shingle, map<size_t,bool> & hash_map){
        hash_map[shingle.second] = true;
        for(size_t hash : shingle.first) hash_map[hash] = true;
    }

    void go_through_tree();

        // functions for Sync Methods

    void SendSyncParam(const shared_ptr<Communicant>& commSync, bool oneWay = false) override;

    void RecvSyncParam(const shared_ptr<Communicant>& commSync, bool oneWay = false) override;

    void configure(shared_ptr<SyncMethod>& setHost, long mbar);

    bool reconstructString(DataObject* & recovered_string, const list<DataObject *> & theirsMinusMine, const list<DataObject *> & mineMinusTheirs)override;
};
#endif //CPISYNCLIB_SETSOFCONTENT_H
