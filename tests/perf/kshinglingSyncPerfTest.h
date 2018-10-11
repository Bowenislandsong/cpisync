//
// Created by Bowen Song on 9/27/18.
//

#ifndef CPISYNCLIB_KSHINGLINGSYNCPERFTEST_H
#define CPISYNCLIB_KSHINGLINGSYNCPERFTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "kshinglingSync.h"
#include "PerformanceData.h"

class KshingleSyncPerf : public CPPUNIT_NS::TestFixture {
CPPUNIT_TEST_SUITE(KshingleSyncPerf);
        CPPUNIT_TEST(testklgperf);
        CPPUNIT_TEST(testFixedKperf);
    CPPUNIT_TEST_SUITE_END();

public:
    KshingleSyncPerf();

    ~KshingleSyncPerf();

    // k = lg(string len)
    void testklgperf();

    // k = fixed constant
    void testFixedKperf();


private:
    // file parameters
    vector<vector<long>> F_editDistance;  // X
    vector<vector<long>> F_commCost;  // Y
    vector<string> F_legend;
    string F_title;
    long String_Size;


    struct thread_data {
        PerformanceData *test;
        int strSize_p, shingleLen_p, editDist_p;
    };

    static void *kshingleSynctest(void * threadArg){
        struct thread_data *my_data;
        my_data = (struct thread_data*)threadArg;

        my_data->test->prepareStringRecon(my_data->strSize_p, my_data->shingleLen_p, my_data->editDist_p);

        // Declear what set reconciliation we are testing
        auto setReconProto = {GenSync::SyncProtocol::CPISync, GenSync::SyncProtocol::InteractiveCPISync};
        // Declear what string reconciliation we are testing
        auto strReconProto = {PerformanceData::StringReconProtocol::KshinglingSync};

        for (auto strRecon: strReconProto) { // go through all Str Recon methods mentioned above
            for (auto setRecon: setReconProto) { // go through all set Recon methods mentioned above
                my_data->test->prepareSetComm(strRecon, setRecon);
                my_data->test->calCostReport();
            }
        }
    };
};
#endif //CPISYNCLIB_KSHINGLINGSYNCPERFTEST_H
