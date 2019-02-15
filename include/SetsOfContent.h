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
#include "ProcessData.h"

#include "ForkHandle.h" // tobe removed

using std::vector;
using std::hash;
using std::string;
using namespace NTL;
typedef unsigned short sm_i; // small index for lvl, MAX = 65,535 which is terminal string size * partition^lvl = file string size

// describes a cycling using first and last and cycle number. Used in backtracking. We pointou the first, then the last, then cycle value./
struct cycle {
    size_t head, len, cyc;
};

static bool operator==(const cycle& a, const cycle& b){
    return a.head == b.head and a.len == b.len and a.cyc == b.cyc;
};

static bool operator!=(const cycle& a, const cycle& b) {
    return !(a == b);
};

static ostream& operator<<(ostream &os, const cycle cyc) {
    os << "head: " + to_string(cyc.head) + ", len: " + to_string(cyc.len) + ", cyc: " +
          to_string(cyc.cyc);
    return os;
};

static cycle ZZtoCycle(const ZZ& zz){
    cycle cyc;
    BytesFromZZ((uint8_t *) &cyc, zz, sizeof(cycle));
    return cyc;
}


static ZZ CycletoZZ(cycle cyc) {
    char *my_s_bytes = reinterpret_cast<char *>(&cyc);
    return ZZFromBytes((const uint8_t *) my_s_bytes, sizeof(cycle));
}


/**
 * first is the first part of a shingle
 * second is the second part of the shingle
 * occurrence, how manny time this occrred
 * compose states the composite of second
 */
struct shingle_hash{
    size_t first, second, occurr;
    sm_i lvl;
};


//Compare and help order struct shingle_hash from a vector
static bool operator<(const shingle_hash& a, const shingle_hash& b) {
    return (a.first < b.first) or
           (a.first == b.first and a.second < b.second) or
           (a.first == b.first and a.second == b.second and a.occurr < b.occurr);
    // we do not consider ordering cytcle since it would be same to ordering "second"
};


//Compare and help differentiate struct shingle_hash
static bool operator==(const shingle_hash& a, const shingle_hash& b) {
    return a.first == b.first and a.second == b.second and a.occurr == b.occurr and a.lvl == b.lvl;
};

static bool operator!=(const shingle_hash& a, const shingle_hash& b) {
    return !(a == b);
};

static shingle_hash ZZtoShingleHash(const ZZ& zz){
    shingle_hash shingle;
    BytesFromZZ((uint8_t *) &shingle, zz, sizeof(shingle_hash));
    return shingle;
}


static ZZ ShingleHashtoZZ(shingle_hash shingle) {
    char *my_s_bytes = reinterpret_cast<char *>(&shingle);
    return ZZFromBytes((const uint8_t *) my_s_bytes, sizeof(shingle_hash));
}


static string to_string(const vector<size_t> a) {string res; for (size_t t : a) res+=to_string(t)+":"; res.pop_back(); return res;};

static ostream& operator<<(ostream &os, const shingle_hash shingle) {
    os << "fst: " + to_string(shingle.first) + ", sec: " + to_string(shingle.second) + ", occ: " +
          to_string(shingle.occurr) + ", lvl: "+to_string(shingle.lvl);
    return os;
};

class SetsOfContent : public SyncMethod {
public:
    SetsOfContent(size_t terminal_str_size, size_t levels, size_t partition, GenSync::SyncProtocol base_set_proto,
                  size_t shingle_size = 2, size_t ter_win_size = 2);

    ~SetsOfContent();

    string retriveString();

// functions for SyncMethods
    bool addStr(DataObject *str, vector<DataObject *> &datum, bool sync) override;

    bool SyncClient(const shared_ptr<Communicant> &commSync, list<DataObject*> &selfMinusOther, list<DataObject*> &otherMinusSelf) override;

    bool SyncServer(const shared_ptr<Communicant> &commSync, list<DataObject*> &selfMinusOther, list<DataObject*> &otherMinusSelf) override;

    string getName() override { return "Sets of Content"; }

    long getVirMem() override { return (long)highwater;};

    //getShinglesAt
    vector<ZZ> getShingles_ZZ() {
        vector<ZZ> res;
        for (auto treelvl : myTree) {
            for (auto item:treelvl) {
                res.push_back(ShingleHashtoZZ(item));
            }
        }
        return res;
    };


protected:
    bool useExisting; /** Use Exiting connection for Communication */

private:
    Resources initRes;
    size_t highwater;

    string myString; // original input string
    size_t TermStrSize, Levels, Partition, terShingleLen, terSpace;

    GenSync::SyncProtocol baseSyncProtocol;

    vector<DataObject *> setPointers; // garbage collector

    vector<std::set<shingle_hash>> myTree; // the hash shingle tree   // RECONCILLING TARGET

    map<size_t, string> Dictionary;  // terminla strings

    // origin, cycle information to reform this string rep
    map<size_t, vector<size_t>> Cyc_dict; // has to be unique

    //requests
    map<size_t, string> term_concern, term_query;
    map<size_t, cycle> cyc_concern, cyc_query;

    size_t str_to_hash(const string &str) {
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
    vector<size_t> create_HashSet(string str, size_t win_size, size_t space = NOT_SET, size_t shingle_size = NOT_SET);

    vector<size_t> local_mins(vector<size_t> hash_val, size_t win_size);

    /**
     * Insert string into dictionary
     * @param str a substring
     * @return hash of the string
     * @throw if there is duplicates, suggest using new/multiple hashfunctions
     */
    size_t add_to_dictionary(const string &str);

    inline size_t min_between(const vector<size_t> &nums, size_t from, size_t to) {
        if (nums.empty()) throw invalid_argument("min function does not take empty vector");
        size_t min = nums[0];
        for (size_t i = from; i <= to; ++i) {
            if (nums[i] < min) min = nums[i];
        }
        return min;
    }

    // extract the unique substring hashes from the shingle_hash vector
    vector<size_t> unique_substr_hash(std::set<shingle_hash> hash_set) {
        std::set<size_t> tmp;
        for (shingle_hash item : hash_set) {
            tmp.insert(item.first);
            tmp.insert(item.second);
        }
        return vector<size_t>(tmp.begin(), tmp.end());

    }

    void update_tree_shingles(vector<size_t> hash_vector, sm_i level);

    // getting the poteintial list of next shingles
    vector<shingle_hash>
    get_nxt_shingle_vec(const size_t cur_edge, const map<size_t, vector<shingle_hash>> &last_state_stack,
                        const map<size_t, vector<shingle_hash>> &original_state_stack);

    // functions for backtracking
    /**
     * peice shingle_hashes back to a vector of hashes in the string order
     * @param shingle_set
     * @param str_order
     * @param final_str a hash train in string order
     */
    bool shingle2hash_train(cycle &cyc_info, const set <shingle_hash> &shingle_set, vector<size_t> &final_str);

    std::map<size_t, vector<shingle_hash>> tree2shingle_dict(const std::set<shingle_hash> &tree_lvl);

    shingle_hash get_nxt_edge(size_t &current_edge, shingle_hash _shingle);

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
    void prepare_querys(const vector<shingle_hash> &shingle_hash_theirs, const vector<shingle_hash> &shingle_hash_mine);

//    void prepare_concerns(const vector<shingle_hash> &shingle_hash_theirs, const vector<shingle_hash> &shingle_hash_mine);

    void answer_queries(const map<size_t,bool>& queries, const vector<shingle_hash> &shingle_hash_mine);

    void go_through_tree();

    // functions for Sync Methods

    void SendSyncParam(const shared_ptr<Communicant> &commSync, bool oneWay = false) override;

    void RecvSyncParam(const shared_ptr<Communicant> &commSync, bool oneWay = false) override;

    void configure(shared_ptr<SyncMethod> &setHost, long mbar);

    bool reconstructString(DataObject *& recovered_string, const list<DataObject *>& mySetData) override;
};
#endif //CPISYNCLIB_SETSOFCONTENT_H
