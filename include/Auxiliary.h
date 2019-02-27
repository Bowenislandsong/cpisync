/* This code is part of the CPISync project developed at Boston University.  Please see the README for use and references. */

/* 
 * File:   Auxiliary.h
 * Auxiliary items shared throughout the program.  These include constants, types, and functions.
 *
 * Created on August 20, 2011, 10:07 PM
 */

#ifndef AUX_H
#define    AUX_H


#include <sstream>
#include <unistd.h>
#include <NTL/ZZ.h>
#include <string>
#include <stdexcept>
#include <map>
#include <sys/stat.h>
#include <vector>
#include <iterator>
#include <list>
#include <set>
#include <algorithm>
#include <csignal>
#include <algorithm>
#include <sys/wait.h>
#include <climits>
#include <cstring>
#include "ConstantsAndTypes.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <dirent.h>

// some standard names
using std::cout;
using std::clog;
using std::endl;
using std::vector;
using std::string;
using std::stringstream;
using std::istringstream;
using std::ostringstream;
using std::ostream;
using std::list;
using std::map;
using std::multiset;
using std::invalid_argument;
using std::runtime_error;
// FUNCTIONS

/**
 * Converts a string into a vector of bytes
 * @param data The string to be converted
 * @require string must have fewer than MAXINT characters
 * @return A vector of bytes, corresponding, one by one, to the characters of data
 */
inline vector<byte> StrToVec(const string &data) {
    vector<byte> result; // where we will be build the result to be returned

    const char *data_c_str = data.c_str();
    for (int ii = 0; ii < (int) data.length(); ii++)
        result.push_back(data_c_str[ii]);

    return result;
}

inline string ZZtoStr(const ZZ &zz) {
    string str;
    str.resize(NumBytes(zz), 0);
    BytesFromZZ((uint8_t *) &str[0], zz, str.size());
    return str;
}

inline ZZ StrtoZZ(const string &str) {
    return ZZFromBytes((const uint8_t *) str.data(), str.size());
}


/**
 * Converts a vector of bytes into a string.  The opposite of StrToVec.
 * @param data The vector of bytes to be converted
 * @require vector must have fewer than MAXINT characters
 * @return The string whose characters correspond, one by one, to the bytes of data.
 */
inline string VecToStr(vector<byte> data) {
    string result;
    for (unsigned char ii : data)
        result.push_back(ii);
    return result;
}

/**
 * Helper function to turn a string into a an item that can deal with input stream readers
 * @param str What should be converted
 * @return A converted item.  For example, if templated by T = ZZ_p, returns
 * a ZZ_p corresponding to str in the natural sense (i.e. "23" -> 23 as a ZZ_p).
 * @requires The templated type must be initialized, if necessary (e.g. the modulus selected
 * for a ZZ_p)
 */
template<class T>
inline T strTo(string str) {
    if (str.empty())
        throw invalid_argument(str);

    istringstream tmp(str);
    T result;
    tmp >> result;
    return result;
}

/**
 * Helper function to turn anything with a stream printing capability into a string
 * @param item The thing to be converted
 * @return A string representing the number
 */
template<class T>
inline string toStr(T item) {
    ostringstream tmp;
    tmp << item;
    return tmp.str();
}



/**
 * Reinterprets a ustring into a string
 */
inline string ustrToStr(const ustring &ustr) {
    return string(reinterpret_cast<const char *> ((unsigned char *) ustr.data()), ustr.length());
}

/**
 * Provides a string representing a human-readable version of a list of pointers
 */
template<class T>
string printListOfPtrs(list<T *> theList) {
    string result = "[";
    typename list<T *>::const_iterator iter;
    for (iter = theList.begin(); iter != theList.end(); iter++)
        result += toStr(**iter) + " ";
    result += "]";
    return result;
}

/**
 * Writes the elements of an array as ints to a return string
 */
template<class T>
string writeInts(T *data, int len) {
    string result;
    for (int ii = 0; ii < len; ii++)
        result += toStr((char) data[ii]) + " (" + toStr((int) data[ii]) + ") ";
    return result;
}

/**
 * Prints the contents of a map.
 */
template<class S, class T>
inline string printMap(map<S, T> theMap) {
    string result;
    typename map<S, T>::const_iterator iter;
    for (iter = theMap.begin(); iter != theMap.end(); iter++) {
        result += toStr(iter->first) + " -> " + toStr(iter->second) + "\n";
    }
    return result;
}

// MULTI-SET OPERATIONS

/**
 * Prints the contents of a multiset of strings into a string in a human-readable fashion
 * @param container The multiset whose contents should be shown in the string
 * @return a string representing the contents of the container
 */
inline string multisetPrint(const multiset<string> &container) {
    string result;
    multiset<string>::const_iterator ii;
    for (ii = container.begin();
         ii != container.end();
         ii++)
        result += "[" + *ii + "] ";
    //result += "["+ writeInts(ii->data(),ii->length()) + "] ";
    return result;
}

/**
 * Returns the multi-set intersection of <first> and <second>.
 * Not particularly efficient ... but it works.
 * @param first A multiset of objects
 * @param second A multiset of the same type of objects as <first>
 * @return the resulting multiset
 */
template<class T>
multiset<T> multisetIntersect(const multiset<T> first, const multiset<T> second) {
    vector<T> resultVec;
    std::set_intersection(first.begin(), first.end(), second.begin(), second.end(), back_inserter(resultVec));
    // convert the result to a multiset
    multiset<T> result(resultVec.begin(), resultVec.end());
    return result;
}

/**
 * Returns the multi-set difference <first> - <second>.
 * Not particularly efficient ... but it works.
 * @param first A multiset of objects
 * @param second A multiset of the same type of objects as <first>
 * @return the resulting multiset
 */
template<class T>
multiset<T> multisetDiff(const multiset<T> first, const multiset<T> second) {
    vector<T> resultVec;
    std::set_difference(first.begin(), first.end(), second.begin(), second.end(), back_inserter(resultVec));
    // convert the result to a multiset
    multiset<T> result(resultVec.begin(), resultVec.end());
    return result;
}

/**
 * Functor for comparing pointers to objects by dereferencing these pointers and comparing the resulting objects
 * @tparam T A pointer to an object that s.t. (*T)::operator<(const (*T)&) is defined
 */
template<typename T>
class cmp {
public:
    bool operator()(T a, T b) {
        return (*a) < (*b);
    }
};

/**
 * Calculates the set-difference of two containers of pointers, [begA, endA] - [begB, endB].
 * The set-difference is calculated using the cmp functor as a way to order the container
 * Results are appended to coll.
 * @param [begA, endA] Iterators pointing to the beginning and end of the first container.
 * @param [begB, endB] Iterators pointing to the beginning and end of the second container.
 * @param coll Iterator onto which the results of the set difference will be pushed.
 */
template <class IteratorA, class IteratorB, class IteratorOut>
void rangeDiff(IteratorA begA, IteratorA endA, IteratorB begB, IteratorB endB, IteratorOut coll) {
    typedef typename std::iterator_traits<IteratorA>::value_type T;
    set_difference(begA, endA, begB, endB, coll, cmp<T>());
}

/**
 * Returns the multi-set union <first> U <second>.
 * Not particularly efficient ... but it works.
 * @param first A multiset of objects
 * @param second A multiset of the same type of objects as <first>
 * @return the resulting multiset
 */
template<class T>
multiset<T> multisetUnion(const multiset<T> first, const multiset<T> second) {
    vector<T> resultVec;
    std::set_union(first.begin(), first.end(), second.begin(), second.end(), back_inserter(resultVec));
    // convert the result to a multiset
    multiset<T> result(resultVec.begin(), resultVec.end());
    return result;
}

/**
 * Returns a subset of <size> elements of <first>
 * @param first The multiset from which to choose elements
 * @param size The number of elements to choose
 * @requires <size> < the number of elements in <first>
 * @return A subset of <first> of size <size>
 */
template<class T>
multiset<T> multisetSubset(const multiset<T> first, const int size) {
    multiset<T> result;
    typename multiset<T>::iterator it; // need typename for dependent scope (?)
    int count = 0;
    for (it = first.begin(); count < size; it++, count++)
        result.insert(*it);
    return result;
}

/**
 A generic p-ary tree of type T
 */
template<typename T>
class paryTree {
public:

    /**
     * Construct a p-ary tree of a fixed arity
     * @param datum The actual datum to add - will be deallocated by the destructor
     * @param pary The number of children per node.
     */
    paryTree(T *theDatum, int pary) : arity(pary) {
        datum = theDatum;
        child = new paryTree<T> *[arity];
        for (int ii = 0; ii < arity; ii++) child[ii] = NULL;
    }

    /** Destructor */
    ~paryTree() {
        delete[] child; /** An array of children of the current node. */
        delete datum; /** The payload of the current node. */
    }

    /** Accessor */
    T *getDatum() {
        return datum;
    }

    paryTree<T> **child; /** Full access to all the children of the node. */
private:
    long arity;
    T *datum;
};

const int min_base64 = 62; // first character of base-64 text
const int signed_shift = 128; // shift to get from unsigned to signed

/**
 * Encodes a given ASCII c-style string into a (base64) string using only characters from '>' to '~'
 * @param bytes_to_encode The bytes to encode base 64
 * @param len The length of the bytes array
 * @return An ASCII-armored string.
 */
inline string base64_encode(char const *bytes_to_encode, unsigned int in_len) {
    string ret;

    int round3 = 3 * (in_len % 3 == 0 ? in_len / 3 : 1 + (in_len / 3)); // the number of whole groups of 3
    // every 3 ASCII characters get converted into four base64 characters
    for (int ii = 0; ii < round3; ii += 3) {
        int group = signed_shift + bytes_to_encode[ii] +
                    256 * (ii + 1 >= (int) in_len ? 0 : signed_shift + bytes_to_encode[ii + 1]) +
                    256 * 256 * (ii + 2 >= (int) in_len ? 0 : signed_shift + bytes_to_encode[ii + 2]);
        ret += (char) min_base64 + group % 64;
        ret += (char) min_base64 + (group >> 6) % 64;
        ret += (char) min_base64 + (group >> 12) % 64;
        ret += (char) min_base64 + (group >> 18) % 64;
    }
    // replace the last characters with "=" as needed
    if (in_len % 3 >= 1)
        ret[ret.length() - 1] = '=';
    if (in_len % 3 == 1)
        ret[ret.length() - 2] = '=';

    return ret;
}

/**
 * Decodes a string of characters in the ASCII range '>' to '~' into an ASCII string.
 * The exact inverse of base64_encode
 */

inline string base64_decode(std::string const &encoded_string) {
    int in_len = encoded_string.length();

    if (in_len <= 0) {
        return "";
    } // edit
    char tmp[in_len];
    strncpy(tmp, encoded_string.data(), in_len);

    // record how much padding was in the string, and remove it
    int rem = 0;
    if (tmp[in_len - 1] == '=') {
        rem++;
        tmp[in_len - 1] = min_base64;
    }
    if (tmp[in_len - 2] == '=') {
        rem++;
        tmp[in_len - 2] = min_base64;
    }

    string ret = "";
    for (int ii = 0; ii < in_len; ii += 4) {
        unsigned long group =
                (tmp[ii] - min_base64) + 64 * (tmp[ii + 1] - min_base64) + 64 * 64 * (tmp[ii + 2] - min_base64) +
                64 * 64 * 64 * (tmp[ii + 3] - min_base64);
        ret += (char) (group % 256) - signed_shift;
        ret += (char) ((group >> 8) % 256) - signed_shift;
        ret += (char) ((group >> 16) % 256) - signed_shift;
    }

    if (rem > 0)
        ret.erase(ret.length() -
                  rem); // erase the last few characters, depending on the number of ='s in the base64 string

    return ret;
}

// additions

/**
 * A simple interface to base64_encode below
 * @param base64_chars
 * @return 
 */
inline string base64_encode(const string bytes, unsigned int in_len) {
    string foo = base64_encode(bytes.data(), in_len);
    return foo;
}

/**
 * @return The minimum of two NTL ZZ objects
 */
inline ZZ min(const ZZ &aa, const ZZ &bb) {
    if (compare(aa, bb) == 1) // (aa>?bb)
        return aa;
    else
        return bb;
}

/**
 * @return A random integer in [lower, upper]
 * @require srand() must've been called
 */
inline int randLenBetween(int lower, int upper) {
    int length = (rand() % (upper + 1));
    if (length < lower) length = lower;
    return length;
}

/**
 * @return A random long
 * @require srand() must've been called
 */
inline long randLong() {
    return (static_cast<long>(rand()) << (sizeof(int) * CHAR_BIT)) |
           rand(); // lshift the amount of bits in an int and then bitwise or a random int
}

/**
 * @return A random byte
 * @require srand() must've been called
 */
inline byte randByte() {
    return (byte) (rand() % (int) pow(2, CHAR_BIT));
}

/**
 * @return A string of random characters with a random length in [lower, upper]
 * @require srand() must've been called
 */
inline string randString(int lower = 0, int upper = 10) {
    stringstream str;

    // pick a length in between lower and upper, inclusive
    int length = randLenBetween(lower, upper);

    for (int jj = 0; jj < length; jj++)
        str << (char) randByte(); // generate a random character and add to the stringstream

    return str.str();
}

inline string randAsciiStr(int len = 10, string stop_word = "$") {
    string str;

    for (int jj = 0; jj < len; ++jj) {
        auto intchar = rand() % 126;  // avoid random string to be "$" changed to "%"
        if (intchar == int(stop_word[0]) || intchar == 0)
            intchar++;// avoid random string to be "$" changed to "%" and avoid \0 which is NULL
        str += toascii(intchar);

    }
    return str;
}

inline string randCharacters(int len = 10) {
    string str;

    for (int jj = 0; jj < len; ++jj) {
        int intchar;
        (rand() % 2) ? intchar = randLenBetween(65, 90) : intchar = randLenBetween(97, 122);
        str += toascii(intchar);

    }
    return str;
}

inline string subprocess_commandline(const char *command) {
    // borrowed from https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-output-of-command-within-c-using-posix
    char buffer[128];
    string result = "";
    FILE *pipe = popen(command, "r");
    if (!pipe) Logger::error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (exception e) {
        pclose(pipe);
        cout << "We fialed to get command line response: " << e.what() << endl;
        Logger::error("Failed to read command reply");
    }
    pclose(pipe);
    return result;
}

struct rsync_stats {
    size_t xmit, recv;
};

inline ostream &operator<<(ostream &os, const rsync_stats &stats) {
    os << "xmit: " + to_string(stats.xmit) + ", recv: " + to_string(stats.recv);
    return os;
};

inline string extractStringIn(string org, string from, string to) {
    auto start = org.find(from);
    if (start == string::npos) return "";
    org = org.substr(start + from.size());
    auto end = org.find(to);
    if (end == string::npos) return "";
    return org.substr(0, end);
}

// write string to file and return true if success
inline void writeStrToFile(string file_name, string content) {
    ofstream myfile;
    myfile.open(file_name, ios::trunc);
    myfile << content;
    myfile.close();
}

inline rsync_stats getRsyncStats(string origin, string target, bool full_report = false) {
// only works for one type of rsync outputs
    rsync_stats stats;

    string res = subprocess_commandline(("rsync --checksum --no-whole-file --progress --stats " + origin + " " +
                                         target).c_str());  // -a archive -z compress -v for verbose
//    stats.time = stod(extractStringIn(res, "File list generation time: ", "seconds"));
//    stats.time += stod(extractStringIn(res, "File list transfer time: ", "seconds"));
    stats.xmit = stoll(extractStringIn(res, "Total bytes sent: ", "\n"));
    stats.recv = stoll(extractStringIn(res, "Total bytes received: ", "\n"));
    if (full_report)
        cout << res << endl;
    return stats;
}

inline string scanTxtFromFile(string dir, int len) {
    std::string line;
    std::ifstream myfile(dir); //"./tests/SampleTxt.txt"
    ostringstream txt;
    long long str_len = 0;
    if (myfile.is_open()) {
        while (getline(myfile, line) and (str_len += line.size()) < len) txt << line;
        txt << line.substr(0, len - str_len + line.size());
        myfile.close();
    } else {
        throw invalid_argument("Directory " + dir + " does not exist.-");
    }
    return txt.str();
}

/**
 * Chekc if file or directory path exist
 * @param path
 * @return
 */
inline bool isPathExist(const string &path) {
    struct stat buf;
    return (stat(path.c_str(), &buf) == 0);
}

/**
 * Check if it is a file or directory
 * throw error if it is either
 * @param path
 * @return
 */
inline bool isFile(const string &path) {
    struct stat buf;
    int a = stat(path.c_str(), &buf);
    if (stat(path.c_str(), &buf) == 0) {
        if (buf.st_mode & S_IFREG)
            return true;
        else if (buf.st_mode & S_IFDIR)
            return false;
        else
            throw invalid_argument("Given path: " + path + " exist, but it is not a file nor directory");
    }
    throw invalid_argument("Given path: " + path + " does not exist.");
}

inline size_t getFileSize(const string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return st.st_size;
}

/**
 * count the number of txt file avaliable in a folder
 */
inline vector<string> getFileList(string dir_path, string file_type = ".txt") {
    vector<string> f_lst;
    if (isPathExist(dir_path) and not isFile(dir_path)) {
        DIR *dir;
        struct dirent *dirp;
        if ((dir = opendir(dir_path.c_str())) != NULL) {
            while ((dirp = readdir(dir)) != NULL) {
                string f_name = string(dirp->d_name);
                if (f_name.find(file_type) != std::string::npos)
                    f_lst.push_back(f_name);
            }
            closedir(dir);
        } else {
            throw invalid_argument("Failed at opening directory: " + dir_path);
        }
    }
    return f_lst;
}

/**
 * Get string of length=len from directory or file.
 * throw error if file is smaller than requrested string size
 * throw error if path does not exist
 * file: "./tests/SampleTxt.txt"
 * dir: ./tests/
 * @param len
 * @param loc
 * @return
 */
inline string randTxt(int len, string loc) {
    string full_txt;
    size_t MAX_LEN;
    if (isFile(loc)) { // it is a file
        MAX_LEN = getFileSize(loc);
        if (MAX_LEN < len)
            throw invalid_argument(
                    "Requested string size exceeds file size. Current file size: " + to_string(MAX_LEN));

        full_txt = scanTxtFromFile(loc, MAX_LEN);
        (len > full_txt.size()) ? len = full_txt.size() : 0;
        int start_pt = randLenBetween(0, full_txt.size() - len);
        return full_txt.substr(start_pt, len);
    } else { // it is a directory
        vector<string> file_lst = getFileList(loc);
//        std::random_shuffle ( file_lst.begin(), file_lst.end() );
        while (len > full_txt.size()) {
            string full_path = loc + file_lst[randLenBetween(0, file_lst.size() - 1)];
            int file_size = getFileSize(full_path);
            if (len < file_size + full_txt.size()) file_size = len - full_txt.size();

            full_txt += randTxt(file_size, full_path);
        }
        return full_txt;
    }


}

inline string randSampleTxt(int len) {
    int MAX_LEN = (int) 2e6; // the sample file is 1e5 characters long
    if (len > MAX_LEN) throw invalid_argument("rand Sample Txt can not be more than " + to_string(MAX_LEN));
    string full_txt = scanTxtFromFile("./tests/SampleTxt.txt", MAX_LEN);
    if (len == MAX_LEN) return full_txt;
    int start_pt = randLenBetween(0, full_txt.size() - len - 1);

    return full_txt.substr(start_pt, len);
}

inline string randSampleCode(int len) {
    int MAX_LEN = (int) 1e5; // the sample file is 1e5 characters long
    if (len > MAX_LEN) throw invalid_argument("rand Sample Code can not be more than " + to_string(MAX_LEN));
    string full_txt = scanTxtFromFile("./tests/SampleCode.txt", MAX_LEN);
    if (len == MAX_LEN) return full_txt;
    int start_pt = randLenBetween(0, full_txt.size() - len - 1);

    return full_txt.substr(start_pt, len);
}

/**
 * Generate a string with upperI number of random insertions of the original string
 * @param upperI number of insertion upper bound
 * @return Edited string with upperI insertions away from the original string
 */
inline string randStringInsert(string str, int upperI) {
    for (int jj = 0; jj < upperI; jj++) {
        //pick a place to edit
        int pos;
        if (str == "")pos = 0;
        else pos = randLenBetween(0, str.size() - 1);

        str = str.substr(0, pos) + randCharacters(1) + str.substr(pos);
    }
    return str;
}

/**
 * Generate a string with upperD number of random deletions of the original string
 * @param upperD number of deletion upper bound
 * @return Edited string with upperD deletions away from the original string
 */
inline string randStringDel(string str, int upperD) {
    if (str.size() <= upperD) {
        return "";
    }

    for (int jj = 0; jj < upperD; jj++) {
        //pick a place to edit
        int pos = randLenBetween(0, str.size() - 1);

        str = str.substr(0, pos) + str.substr(pos + 1);
    }
    return str;
}

/**
 * Generate a string with upperE number of random edits of the original string
 * @param upperE Edit upper bound
 * @return Edited string upperE edit distance away from the original string
 */
inline string randStringEdit(string str, int upperE) {
    for (int jj = 0; jj < upperE; jj++) {
        str = (rand() % 2 == 0) ? randStringDel(str, 1) : randStringInsert(str, 1);
    }
    return str;
}

/**
 * Generate a string with upperE number of random edits at random numLoc places of the original string
 * @param numLoc number of locations to have the edit burst
 * @param upperE Edit upper bound
 * @return Edited string upperE edit distance away from the original string
 */
inline string randStringEditBurst(string str, int burstE, int numLoc, string loc) {
    for (int ii = 0; ii < numLoc; ++ii) {
        int tmpBurst = (burstE > str.size()) ? str.size() : burstE;
        int pos = randLenBetween(0, str.size() - tmpBurst);
//        str = str.substr(0,pos)+randStringEdit(str.substr(pos,tmpBurst),burstE)+str.substr(pos+tmpBurst);
        if (rand() % 2 == 0) //delete
            str = str.substr(0, pos) + str.substr(pos + tmpBurst);
        else  // insert
            str = str.substr(0, pos) + randTxt(tmpBurst, loc) + str.substr(pos);
//            str = str.substr(0, pos) + str.substr(randLenBetween(0, str.size() - tmpBurst), tmpBurst) + str.substr(pos);
    }
    return str;
}

inline string randStringEditBurst(string str, int upperE, string loc) {
    while (upperE > 0) {
        int burst = randLenBetween(1, upperE);
        upperE -= burst;
        str = randStringEditBurst(str, burst, 1, loc);
    }
    return str;
}

/**
 * @return A random integer converted to a string
 * @require srand() must've been called
 */
inline string randIntString() {
    return toStr(rand());
}

/**
 * @return A random double in [lower, upper]
 * @require srand() must've been called
 */
inline double randDouble(double lower = 0.0, double upper = 1.0) {
    return ((double) rand() * (upper - lower)) / (double) RAND_MAX + lower;
}

/**
 * @return A random ZZ s.t. it is <= ZZ(LONG_MAX)
 * @require srand() must've been called
 */
inline ZZ randZZ() {
    return ZZ(randLong());
}

/**
 * Converts an enum to a byte, signalling an compile-time error if the enum's underlying class is not byte.
 */
template<class T>
inline byte enumToByte(T theEnum) {
    static_assert(std::is_same<byte, typename std::underlying_type<T>::type>::value,
                  "Underlying enum class is not byte - cannot convert to byte!");
    return static_cast< byte >(theEnum);
};

/**
 * An awkward helper for iterating enums.
 * @param curr The current enum value
 * @return the next enum value
 */
template<typename T>
inline T &operator++(T &curr) {
    curr = (T) (((int) (curr) + 1));
    return curr;
}

/**
 * Get the temp directory of the system (POSIX).
 * In C++17, this can be replaced with std::filesystem::temp_directory_path.
 * @return path to temp directory
 */
inline string temporaryDir() {
    // possible environment variables containing path to temp directory
    const char *opts[] = {"TMPDIR", "TMP", "TEMP", "TEMPDIR"};

    // return the first defined env var in opts
    for (const char *ss : opts) {

        // true iff ss is an env var
        if (const char *path = getenv(ss)) {
            return string(path);
        }
    }

    // default temp directory if no env var is found
    return "/tmp";
}

template<typename T>
inline T getMedian(vector<T> vec) {
    std::sort(vec.begin(), vec.end());
    if (vec.size() % 2 == 0) return vec[vec.size() / 2];
    else return (vec[floor(vec.size() / 2)] + vec[ceil(vec.size() / 2)]) / 2;
};

#endif    /* AUX_H */

