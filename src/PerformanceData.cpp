////
//// Created by Bowen on 10/9/18.
////

#include "PerformanceData.h"

PerformanceData::~PerformanceData() = default;

void PerformanceData::kshingle3D(GenSync::SyncProtocol setReconProto, vector<int> edit_distRange,
                                 vector<int> str_sizeRange, int confidence, string (*stringInput)(int), int portnum) {
    string protoName, str_type;
    if (GenSync::SyncProtocol::CPISync == setReconProto) protoName = "CPISync";
    if (GenSync::SyncProtocol::IBLTSyncSetDiff == setReconProto) protoName = "IBLTSyncSetDiff";
    if (GenSync::SyncProtocol::InteractiveCPISync == setReconProto) protoName = "InteractiveCPISync";

    if (*stringInput == randAsciiStr) str_type = "RandAscii";
    if (*stringInput == randSampleTxt) str_type = "RandText";
    if (*stringInput == randSampleCode) str_type = "RandCode";

    PlotRegister plot;
    plot.create("kshingle " + protoName + " " + str_type,
                                     {"Str Size", "Edit Diff", "Comm (bits)", "Time Set(s)", "Time Str(s)",
                                      "Space (bits)", "Set Recon True", "Str Recon True"});
    //TODO: Separate Comm, and Time, Separate Faile rate.

    for (int str_size : str_sizeRange) {
        cout << to_string(str_size) << endl;
        edit_distRange.clear();
        for (int i = 1; i <= tesPts; ++i) edit_distRange.push_back((int) ((str_size * i) / 4000));
        for (int edit_dist : edit_distRange) {

//            int shingle_len = ceil(log2(str_size));
            int shingle_len = ceil(log10(str_size));

            for (int con = 0; con < confidence; ++con) {


                GenSync Alice = GenSync::Builder().
                        setProtocol(setReconProto).
                        setStringProto(GenSync::StringSyncProtocol::kshinglingSync).
                        setComm(GenSync::SyncComm::socket).
                        setPort(portnum).
                        setShingleLen(shingle_len).
                        build();


                DataObject *Alicetxt = new DataObject(stringInput(str_size));

                Alice.addStr(Alicetxt, false);
                GenSync Bob = GenSync::Builder().
                        setProtocol(setReconProto).
                        setStringProto(GenSync::StringSyncProtocol::kshinglingSync).
                        setComm(GenSync::SyncComm::socket).
                        setPort(portnum).
                        setShingleLen(shingle_len).
                        build();

                DataObject *Bobtxt = new DataObject(randStringEdit((*Alicetxt).to_string(), edit_dist));

// Flag true includes backtracking, return false if backtracking fails in the alloted amoun tog memory
                auto str_s = clock();
//                    struct timespec start, finish;
//                    double str_time;

//                    clock_gettime(CLOCK_MONOTONIC, &start);

                bool success_StrRecon = Bob.addStr(Bobtxt, true);

//                    clock_gettime(CLOCK_MONOTONIC, &finish);
//
//                    str_time = (finish.tv_sec - start.tv_sec);
//                    str_time += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
                double str_time = (double) (clock() - str_s) / CLOCKS_PER_SEC;


                multiset<string> alice_set;
                for (auto item : Alice.dumpElements()) alice_set.insert(item->to_string());
                multiset<string> bob_set;
                for (auto item : Bob.dumpElements()) bob_set.insert(item->to_string());

                int success_SetRecon = multisetDiff(alice_set,
                                                    bob_set).size();// separate set recon success from string recon
//                    auto bobtxtis = Alice.dumpString()->to_string();
//                    bool success_SetRecon = ((*Bobtxt).to_string() ==
//                                             Alice.dumpString()->to_string()); // str Recon is deterministic, if not success , set recon is the problem
                forkHandleReport report = forkHandle(Alice, Bob, false);

                plot.add({to_string(str_size), to_string(edit_dist), to_string(report.bytesTot),
                          to_string(report.CPUtime), to_string(str_time),
                          to_string(Bob.getVirMem(0)), to_string(success_SetRecon), to_string(success_StrRecon)});

                delete Alicetxt;
                delete Bobtxt;

            }
        }
        plot.update();
    }
}


void PerformanceData::setsofcontent(GenSync::SyncProtocol setReconProto, vector<int> edit_distRange,
                                 vector<int> str_sizeRange, vector<int> levelRange, vector<int> partitionRange, int confidence, string (*stringInput)(int), int portnum, bool changing_tree_par, vector<int> TershingleLen, vector<int> space) {
    string protoName, str_type;
    if (GenSync::SyncProtocol::IBLTSyncSetDiff == setReconProto) protoName = "IBLTSyncSetDiff";
    if (GenSync::SyncProtocol::InteractiveCPISync == setReconProto) protoName = "InteractiveCPISync";

    if (*stringInput == randAsciiStr) str_type = "RandAscii";
    if (*stringInput == randSampleTxt) str_type = "RandText";
    if (*stringInput == randSampleCode) str_type = "RandCode";

    string last_passed_before_exception;
    PlotRegister plot;
    if (changing_tree_par) {
                plot.create("Sets of Content " + protoName + " " + str_type + "2mStr20kED",
                    {"Level", "Partition", "Comm (bytes)", "Actual Sym Diff", "Time Tree(s)",
                     "Time Recon(s)", "Time Backtrack (included in Time Recon) (s)",
                     "Str Recon True", "Tree Heap SIze", "High Water Heap"});
    }
    else {
        plot.create("Sets of Content " + protoName + " " + str_type + "2mStr20kED4L8P",
                    {"TershingleLen", "space", "Comm (bytes)", "Actual Sym Diff", "Time Tree(s)",
                     "Time Recon(s)", "Time Backtrack (included in Time Recon) (s)",
                     "Str Recon True", "Tree Heap SIze", "High Water Heap"});
    }
//    else {
//    plot.create(
//            "Sets of Content " + protoName + " " + str_type + " lvl: " + to_string(levelRange.front()),
//            {"Sting Size", "Edit Dist %", "Comm (bytes)", "Actual Sym Diff", "Time Tree(s)",
//             "Time Recon(s)", "Time Backtrack (included in Time Recon) (s)",
//             "Str Recon True", "Tree Heap SIze", "High Water Heap"});
//    }
    //TODO: Separate Comm, and Time, Separate Faile rate.
    for (int str_size : str_sizeRange) {
//        cout << " - Sets of Content " + protoName + " " + str_type + "str Size: " + to_string(str_size) << endl;
        for (int edit_dist : edit_distRange) {

            for (int lvl:levelRange) {
//                cout << " - Sets of Content " + protoName + " " + str_type + "(" + to_string(lvl) + " lvl)" << endl;

                for (int par: partitionRange) {

                    for (int t : TershingleLen) {
                        for (int s : space) {

                            for (int con = 0; con < confidence; ++con) {
                                try {
//                            cout << "level: " << lvl << ", partitions: " << par
//                                 << ", Confidence: " << con << endl;

                                    Resources initRes;
//                            initResources(initRes);

                                    GenSync Alice = GenSync::Builder().
                                            setStringProto(GenSync::StringSyncProtocol::SetsOfContent).
                                            setProtocol(setReconProto).
                                            setComm(GenSync::SyncComm::socket).
                                            setTerminalStrSize(100).
                                            setNumPartitions(par).
                                            setShingleLen(t).
                                            setSpace(s).
                                            setlvl(lvl).
                                            setPort(portnum).
                                            build();

                                    last_passed_before_exception = "Alice GenSync"; // success Tag


                                    DataObject *Alicetxt = new DataObject(stringInput(str_size));

                                    last_passed_before_exception = "Alice Create String"; // success Tag
//                            cout<<"Alice String Size"<<Alicetxt->to_string().size()<<endl;

                                    clock_t strStart = clock();
                                    Alice.addStr(Alicetxt, false);
                                    auto tree_time = (double) (clock() - strStart) / CLOCKS_PER_SEC;
                                    resourceReport(initRes);

                                    last_passed_before_exception = "Alice Add String"; // success Tag

                                    GenSync Bob = GenSync::Builder().
                                            setStringProto(GenSync::StringSyncProtocol::SetsOfContent).
                                            setProtocol(setReconProto).
                                            setComm(GenSync::SyncComm::socket).
                                            setTerminalStrSize(100).
                                            setNumPartitions(par).
                                            setShingleLen(t).
                                            setSpace(s).
                                            setlvl(lvl).
                                            setPort(portnum).
                                            build();

                                    last_passed_before_exception = "Bob GenSync"; // success Tag

                                    string bobtmpstring = randStringEditBurst((*Alicetxt).to_string(),
                                                                              (int) (str_size / edit_dist));
                                    if (bobtmpstring.size() < pow(par, lvl))
                                        bobtmpstring += randCharacters(pow(par, lvl) - bobtmpstring.size());

                                    DataObject *Bobtxt = new DataObject(bobtmpstring);

                                    last_passed_before_exception = "Bob Create String"; // success Tag
//                            cout<<"BOB String Size"<<Bobtxt->to_string().size()<<endl;
                                    Bob.addStr(Bobtxt, false);

                                    last_passed_before_exception = "Bob Add String"; // success Tag

                                    forkHandleReport report = forkHandle(Alice, Bob, false);

                                    last_passed_before_exception = "String Recon"; // success Tag

                                    bool success_StrRecon = (Alice.dumpString()->to_string() == Bobtxt->to_string());

                                    if (!success_StrRecon) // success Tag
                                        last_passed_before_exception +=
                                                ", Alice str size: " + to_string(Alice.dumpString()->to_string().size())
                                                + "Bob Str size: " + to_string(Bobtxt->to_string().size());
                                    if (changing_tree_par) {
                                        plot.add({to_string(lvl), to_string(par),
                                                  to_string(report.bytesTot + report.bytesRTot),
                                                  to_string(Alice.getTotalSetDiffSize()), to_string(tree_time),
                                                  to_string(report.totalTime),
                                                  to_string(Alice.getTime().front().second),
                                                  to_string(success_StrRecon), to_string(initRes.VmemUsed),
                                                  to_string(Alice.getVirMem(0))});
                                    }
                                    else {
                                        plot.add({to_string(t), to_string(s),
                                                  to_string(report.bytesTot + report.bytesRTot),
                                                  to_string(Alice.getTotalSetDiffSize()), to_string(tree_time),
                                                  to_string(report.totalTime),
                                                  to_string(Alice.getTime().front().second),
                                                  to_string(success_StrRecon), to_string(initRes.VmemUsed),
                                                  to_string(Alice.getVirMem(0))});
                                    }
//                                    else {
//                                        plot.add({to_string(str_size), to_string((double) 1 / edit_dist),
//                                                  to_string(report.bytesTot + report.bytesRTot),
//                                                  to_string(Alice.getTotalSetDiffSize()), to_string(tree_time),
//                                                  to_string(report.totalTime),
//                                                  to_string(Alice.getTime().front().second),
//                                                  to_string(success_StrRecon), to_string(initRes.VmemUsed),
//                                                  to_string(Alice.getVirMem(0))});
//                                    }

                                    delete Alicetxt;
                                    delete Bobtxt;
                                } catch (std::exception) {
                                    cout << "We failed after " << last_passed_before_exception << endl;
                                    if (changing_tree_par) {
                                        plot.add({to_string(lvl), to_string(par), to_string(0),
                                                  to_string(0), to_string(0), to_string(0), to_string(0), to_string(0),
                                                  to_string(0), to_string(0)});
                                    } else {
                                        plot.add({to_string(str_size), to_string((double) 1 / edit_dist), to_string(0),
                                                  to_string(0), to_string(0), to_string(0), to_string(0), to_string(0),
                                                  to_string(0), to_string(0)});
                                    }
                                }
                            }
                            plot.update();
                        }
                    }
                }
            }
        }

    }
}



void PerformanceData::strataEst3D(pair<size_t, size_t> set_sizeRange, int confidence) {
    int set_sizeinterval = floor((set_sizeRange.second - set_sizeRange.first) / tesPts);

    PlotRegister plot;
    plot.create("Strata Est",{"Set Size","Set Diff","Est"});

//#if __APPLE__
//    confidence /=omp_get_max_threads();
//#pragma omp parallel num_threads(omp_get_max_threads())
//#endif
    for (int set_size = set_sizeRange.first; set_size <= set_sizeRange.second; set_size += set_sizeinterval) {
    (set_size < set_sizeRange.first + (set_sizeRange.second-set_sizeRange.first)/2) ? confidence : confidence=5;
    cout<<"Current Set Size:"+to_string(set_size)<<endl;
        printMemUsage();
        int top_set_diff = set_size / 10;
        int set_diffinterval = floor((top_set_diff) / tesPts);

        for (int set_diff = 0; set_diff <= top_set_diff; set_diff += set_diffinterval) {

//            if (set_size>set_sizeRange.second/2)confidence = 100;
//            printMemUsage();
            //printMemUsage();
//#if __APPLE__
//#pragma omp critical
//#endif
            for (int conf = 0; conf < confidence; ++conf) {

                StrataEst Alice = StrataEst(sizeof(DataObject));
                StrataEst Bob = StrataEst(sizeof(DataObject));
//#if __APPLE__
//#pragma omp parallel firstprivate(Alice,Bob)
//#endif
                for (int j = 0; j < set_size; ++j) {
                    auto tmp = randZZ();
                    if (j < set_size - ceil(set_diff / 2)) Alice.insert(new DataObject(tmp));

                    if (j >= ceil(set_diff / 2)) Bob.insert(new DataObject(tmp));
                }
                plot.add({to_string(set_size), to_string(set_diff), to_string((Alice -= Bob).estimate())});
            }
            //printMemUsage();

        }
//#if __APPLE__
//#pragma omp critical
//#endif
	plot.update();
    }
}



// Graph and Plot functions

PlotRegister::PlotRegister(){}

PlotRegister::~PlotRegister() {}

void PlotRegister::create(string _title, vector<string> _labels) {
    data.clear();
    title = _title;
    labels = _labels;
    init();
}

void PlotRegister::init() {
    ofstream myfile;
    //TODO: do soemthing about the directories, this hard coding is not a long term solution
    myfile.open(title + ".txt");

    myfile<< accumulate(std::next(labels.begin()), labels.end(),
                    labels[0], // start with first element
                    [](string a, string b) {
                        return a + "|" + b;
                    }) + "\n";


//    myfile<<accumulate(labels.begin(), labels.end(), string("|")) + "\n";
    myfile.close();
}

void PlotRegister::update() {
    ofstream myfile(title+".txt",ios::app);
    if (!myfile.good()) init();
    for (auto datum : data)
        myfile<< accumulate(next(datum.begin()), datum.end(),
                            datum[0], // start with first element
                            [](string a, string b) {
                                return a + " " + b;
                            }) + "\n";
    data.clear();
    myfile.close();
}

void PlotRegister::add(vector<string> datum) {
    data.push_back(datum);
}

