//
// Created by Bowen Song on 9/27/18.
//
#include "StringSyncPerfTest.h"

// K-Shingling method does not scale because as string size increase,
// the lenght of each shingle needs to increase inorder to compensate the expoential time of baktracking
// In addition, as shingle size increase, the size for each element increases thus increase the communicaotn cost.
// At last, as shingle size increaases, the amount of differencecs between two shinlge set increases, due to shingle's redundancy
// set size increase as string sizxe increase.

CPPUNIT_TEST_SUITE_REGISTRATION(KshingleSyncPerf);

KshingleSyncPerf::KshingleSyncPerf() = default;

KshingleSyncPerf::~KshingleSyncPerf() = default;

// DEFAULT SETTINGS
const int shingleLen = 4;
const int editDist = 20;
const int strSize = 5;

const int tesPts = 6;// Test Pts per graph
const int target_confidence = 1;// Confidence interval
const int confidenceCap = 40; // after edit distance exceed confidenceCap, confidence go to 1.





void KshingleSyncPerf::kshingleTest3D() {
    PerformanceData test = PerformanceData(tesPts);
    vector<int> editDistRange;
//   vector<int> strSizeRange = {400, 600, 800, 1000, 1400, 1800};//, 2000, 2200, 2600, 3000};//, 5000, 7000, 9000, 10000};
//
//    test.kshingle3D(GenSync::SyncProtocol::CPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randSampleTxt);


    vector<int> strSizeRange = {400, 600, 800, 1000, 1400, 1800};//, 5000, 7000, 9000, 10000};
    test.kshingle3D(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, target_confidence, randAsciiStr, "$",
                    8002);


    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange, target_confidence,
                    randAsciiStr, "$", 8003);


    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff, editDistRange, strSizeRange, target_confidence,
                    randAsciiStr, "$", 8004);


//     vector<int> strSizeRange = {200};
//   test.kshingle3D(GenSync::SyncProtocol::CPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randSampleTxt);


//    PerformanceData test3 = PerformanceData(tesPts);
//    test3.kshingleBook3D(editDistRange,strSizeRange);
//    test3.genReport("kshingleBookTest3D");
//
//    PerformanceData test2 = PerformanceData(tesPts);
//    test2.kshingleCode3D(editDistRange,strSizeRange);
//    test2.genReport("kshingleTestCode3D");


}

void KshingleSyncPerf::setsofcontent3D() {
    PerformanceData test = PerformanceData(tesPts);
    vector<int> editDistRange;
//   vector<int> strSizeRange = {400, 600, 800, 1000, 1400, 1800};//, 2000, 2200, 2600, 3000};//, 5000, 7000, 9000, 10000};
//
//    test.kshingle3D(GenSync::SyncProtocol::CPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randSampleTxt);


    vector<int> strSizeRange{100000,200000,1000000,2000000,10000000,20000000};
    editDistRange = {5,10,50,100,500, 1000};
    vector<int> lvlRange = {5,5,6,6,7,7};
    vector<int> parRange = {3,4,5};

    string bookpath = string(std::getenv("HOME")) + "/Desktop/sync_database/BookText/";
//    test.setsofcontent(GenSync::SyncProtocol::IBLTSyncSetDiff, editDistRange, strSizeRange,{2}, {10}, 10, randSampleTxt, 8001,false);

//    test.setsofcontent(GenSync::SyncProtocol::IBLTSyncSetDiff, editDistRange, strSizeRange,{3}, {10}, {2},{4}, 10, randTxt,bookpath, 8002,1);

//    test.setsofcontent(GenSync::SyncProtocol::CPISync, {1000}, {2000000}, lvlRange, parRange, {2}, {4}, 1,
//                       randTxt, bookpath, 8002, 2);

//    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, lvlRange, {3}, {2}, {4}, 100,
//                       randTxt, bookpath, 8005, 1);

//    test.setsofcontent(GenSync::SyncProtocol::IBLTSyncSetDiff, editDistRange, strSizeRange,{5}, {10}, 100, randSampleTxt, 8001,false);


//    test.setsofcontent(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange, 2, 1, randSampleTxt, 8001);
//
//    test.setsofcontent(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange, 3, 1, randSampleTxt, 8001);
//
//    test.setsofcontent(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange, 4, 1, randSampleTxt, 8001);

//    strSizeRange = {1000000};
    editDistRange = {1000};
    vector<int> window = {2};
    vector<int> space = {2,4,6,8,10,12};
    parRange = {4};
    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange,lvlRange, parRange,window,space, 50, randTxt,bookpath, 8003,3);


    lvlRange = {4};





    vector<int> numError = {1, 2, 3, 4, 5, 6, 7, 8};
    window = {2};
    space = {4};



//   test.setsofcontent(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange,lvlRange, parRange,shingle,space, 100, randTxt,bookpath, 8002,3);
//    test.cascadingMissmatch(numError, window, space);
//    test.setsofcontent(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange,lvlRange, parRange, 1, randAsciiStr, 8001);

//    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange,lvlRange, parRange, 1, randSampleTxt, 8001);


//    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, 2, 1, randSampleTxt, 8001);
//
//    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, 3, 1, randSampleTxt, 8001);
//
//    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, 4, 1, randSampleTxt, 8001);
}

void KshingleSyncPerf::testStrataEst3D() {
    PerformanceData test = PerformanceData(tesPts);
    test.strataEst3D(make_pair(1000, 100000), target_confidence);
}
