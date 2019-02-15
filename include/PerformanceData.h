//
// Created by Bowen on 10/9/18.
//

#ifndef CPISYNCLIB_PERFORMANCEDATA_H
#define CPISYNCLIB_PERFORMANCEDATA_H

#include "GenSync.h"
#include "kshinglingSync.h"
#include "StrataEst.h"
#include "ForkHandle.h"
#include <fstream>
#include <omp.h>
#include <numeric>
#include <thread>

class PerformanceData {
public:
    PerformanceData(int tes_pts) {
        tesPts = tes_pts;
    };

    ~PerformanceData();

    enum class StringReconProtocol {
        UNDEFINED,
        BEGIN,
        KshinglingSync = BEGIN,
        SetsOfContent,
        END
    };


    void kshingle3D(GenSync::SyncProtocol setReconProto, vector<int> edit_distRange,
                    vector<int> str_sizeRange, int confidence, string (*stringInput)(int), int portnum);

    void setsofcontent(GenSync::SyncProtocol setReconProto, vector<int> edit_distRange,
                       vector<int> str_sizeRange, vector<int> levelRange, vector<int> partitionRange, int confidence, string (*stringInput)(int), int portnum,bool changing_tree_par, vector<int> TershingleLen = {2}, vector<int> space = {2});

    void strataEst3D(pair<size_t, size_t> set_sizeRange, int confidence);

private:

    int  mbar, tesPts;
    size_t bits;



    /**
     * explore relation of edit distance and set difference in kshingling
     * @param shingle_len
     * @param str_size
     * @param edit_dist
     */
    int setdiff(int shingle_len, int str_size, int edit_dist);


};


class PlotRegister { // Export Data into a txt file for external code to graph
public:
    PlotRegister();

    ~PlotRegister();

    void create(string _title, vector<string> _labels);// init - open a file with a title and labels
    void add(vector<string> datum); // add to data
    void update(); // bulk insert to what is in the data, clear data, close file after.
private:

    void init();

    string title; // title of the graph, used as file name
    vector<string> labels; //label including units
    vector<vector<string>> data; // Number of
};

#endif //CPISYNCLIB_PERFORMANCEDATA_H
