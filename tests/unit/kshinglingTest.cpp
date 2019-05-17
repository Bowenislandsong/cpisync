//
// Created by Bowen Song on 9/16/18.
//

#include <climits>
#include "kshinglingTest.h"
#include "time.h"


CPPUNIT_TEST_SUITE_REGISTRATION(kshinglingTest);

kshinglingTest::kshinglingTest() {
}

kshinglingTest::~kshinglingTest() {
}

void kshinglingTest::setUp() {
    const int SEED = 617;
    srand(SEED);
}

void kshinglingTest::tearDown() {
}

void kshinglingTest::testAll() {

    shingle s = shingle{.vex = "Bowe", .edge = "n", .occurr = 58};

    CPPUNIT_ASSERT(s == ZZtoT(TtoZZ(s), shingle()));

    // init a string of random byte (shortest,longest) possible string len
    string Alicetxt = randSampleTxt(50);  // generate a string, no longer than 1e4
//    Alicetxt = "katanatamasaka";
    //string Bobtxt = randStringEdit(Alicetxt,10);  // Generate a edited string

    clock_t t1 = clock();
    int shingle_size = ceil(log2(Alicetxt.size()));
    K_Shingle Alice = K_Shingle(shingle_size);

    auto cycle_num = Alice.inject(Alicetxt, true);

    string str;
    bool success = Alice.shingle2string(cycle_num, str);  // Get order of the cycle
    clock_t t2 = clock();

    cout << to_string(double(t2 - t1) / CLOCKS_PER_SEC) << endl;
    CPPUNIT_ASSERT(cycle_num > 0);
    CPPUNIT_ASSERT(Alice.getOriginString() == str and success);
    CPPUNIT_ASSERT(Alice.getShingles().size() > 0);


    //test with Half round sync

}