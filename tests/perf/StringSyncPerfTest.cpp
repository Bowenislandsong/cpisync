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
//    vector<int> editDistRange;
//
//
//    vector<int> strSizeRange{100000,500000,1000000,1500000,2000000,2500000};
//    editDistRange = {10000};
//    vector<int> lvlRange = {6};
//    vector<int> parRange;
//
////    string path = string(std::getenv("HOME")) + "/Desktop/sync_database/BookText/";
//    string path = string(std::getenv("HOME")) + "/Desktop/sync_database/Repo";
//
//
////    vector<int> window = {2};
////    vector<int> space = {8};
////    parRange = {4};
//    int conf = 10;
//
//    // parallel processes
//    int nProcesses = 0;
//
//    pid_t pID[nProcesses];
//    for (int i = 0; i < nProcesses; ++i) {
//        if ((pID[i] = fork()) < 0) {
//            perror("fork");
//            abort();
//        } else if (pID[i] == 0) {
//            cout << "Chlid: " << i << endl;
//            srand(i + 1);
//            test.setsofcontentREPO(GenSync::SyncProtocol::CPISync, editDistRange, conf,  path, 1 + i);
//
//            exit(0);
//        }
//    }
//
//    int child_state;
//    cout << "Parent on job" << endl;
//    srand(time(NULL));
//    test.setsofcontentREPO(GenSync::SyncProtocol::CPISync, editDistRange, conf, path, 0);
//    cout << "child " << wait(&child_state) << " done and well" << endl;
//
//
////    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange,lvlRange, parRange,window,space, 1, randTxt,bookpath, 3,3);
//
//
//
//



//    vector<int> numError = {1, 2, 3, 4, 5, 6, 7, 8};
//    window = {2};
//    space = {4};



//    test.cascadingMissmatch(numError, window, space);

}

void KshingleSyncPerf::testStrataEst3D() {
    PerformanceData test = PerformanceData(tesPts);
    test.strataEst3D(make_pair(1000, 100000), target_confidence);
}
