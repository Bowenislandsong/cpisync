/* This code is part of the CPISync project developed at Boston University.  Please see the README for use and references. */

#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include "GenSync.h"
#include "Exceptions.h"
#include "CPISync.h"
#include "CommSocket.h"
#include "CommString.h"
#include "ProbCPISync.h"
#include "InterCPISync.h"
#include "FullSync.h"
#include "IBLTSync.h"
#include "IBLTSync_HalfRound.h"
#include "CPISync_HalfRound.h"
#include "FullSync.h"
#include "IBLTSync_SetDiff.h"
#include "kshinglingSync.h"
#include "SetsOfContent.h"
#include "RCDS.h"


/**
 * Construct a default GenSync object - communicants and objects will have to be added later
 */
GenSync::GenSync() = default;

/**
 * Construct a specific GenSync object
 */
GenSync::GenSync(const vector<shared_ptr<Communicant>> &cVec, const vector<shared_ptr<SyncMethod>> &mVec,
                 const list<DataObject *> &data) {
    myCommVec = cVec;
    mySyncVec = mVec;
    outFile = nullptr; // no output file is being used
    // add each datum one by one
    auto itData = data.begin();
    for (; itData != data.end(); itData++)
        addElem(*itData);
}

GenSync::GenSync(const vector<shared_ptr<Communicant>> &cVec, const vector<shared_ptr<SyncMethod>> &mVec,
                 string fileName) {
    myCommVec = cVec;
    mySyncVec = mVec;
    outFile = nullptr; // add elements without writing to the file at first
    Logger::gLog(Logger::METHOD, "Entering GenSync::GenSync");
    // read data from a file
    Logger::gLog(Logger::METHOD, "Utilizing file: " + fileName);
    ifstream inFile(fileName.c_str());
    string str;
    for (getline(inFile, str); inFile.good(); getline(inFile, str)) {
        addElem(new DataObject(str)); // add this datum to our list
        Logger::gLog(Logger::METHOD_DETAILS, "... added set element " + str);
    }
    inFile.close();

    // register the file to which new data should be appended
    outFile = std::make_shared<ofstream>(fileName.c_str(), ios::app);

}

// destruct a gensync object

GenSync::~GenSync() {
    // clear out memory
    myData.clear();

    //    vector<shared_ptr<SyncMethod>>::iterator itAgt = mySyncVec.begin();
    //    for (; itAgt != mySyncVec.end(); itAgt++) {
    //        delete *itAgt;
    //    }
    mySyncVec.clear();

    //    vector<shared_ptr<Communicant>>::iterator itComm = myCommVec.begin();
    //    for (; itComm != myCommVec.end(); itComm++) {
    //        delete *itComm;
    //    }
    myCommVec.clear();

    // close and free the output file
    if (outFile != nullptr) {
        outFile->close();
        outFile.reset();
    }
}


// listen, receive data and conduct synchronization

bool GenSync::listenSync(int method_num, bool isRecon) {
    Logger::gLog(Logger::METHOD, "Entering GenSync::listenSync");
    // find the right syncAgent
    auto syncAgent = mySyncVec.begin();
    advance(syncAgent, method_num);

    bool syncSuccess = true; // true if all syncs so far were successful

    // ask each communicant to listen, one by one
    vector<shared_ptr<Communicant>>::iterator itComm;
    list<DataObject *> selfMinusOther, otherMinusSelf;

    for (itComm = myCommVec.begin(); itComm != myCommVec.end(); ++itComm) {
        // initialize variables
        selfMinusOther.clear();
        otherMinusSelf.clear();
        try {
            syncSuccess &= (*syncAgent)->SyncServer(*itComm, selfMinusOther, otherMinusSelf);

        } catch (SyncFailureException s) {
            Logger::error_and_quit(s.what());
            return false;
        }

        // add any items that were found in the reconciliation
        list<DataObject *>::iterator itDO;
        for (itDO = otherMinusSelf.begin(); itDO != otherMinusSelf.end(); itDO++) {
            addElem(*itDO);
        }

        if (!isRecon) {
            // newly added --- worked for general test
            delElemGroup(selfMinusOther);
        }
//TODO: if not one way, enable this and set up a flag for one way , mind sync client or sync server
//        if ((*syncAgent)->isStringReconMethod()) { // If it is string reconciliation
//        clock_t time = clock();
//        syncSuccess = (*syncAgent)->reconstructString(
//                myString, myData); // reconstruct the string based on the new information from set reconciliation
//        timeFrame.emplace_back("Str Reconstruction Time",(double) (clock() - time) / CLOCKS_PER_SEC);
//        }


    }

    return syncSuccess;
}

// request connection, send data and get the result
bool GenSync::startSync(int method_num, bool isRecon) {
    Logger::gLog(Logger::METHOD, "Entering GenSync::startSync");
    // find the right syncAgent
    auto syncAgentIt = mySyncVec.begin();
    advance(syncAgentIt, method_num);

    bool syncSuccess = true; // true if all syncs so far were successful
    vector<shared_ptr<Communicant>>::iterator itComm;
    list<DataObject *> selfMinusOther, otherMinusSelf;

    for (itComm = myCommVec.begin(); itComm != myCommVec.end(); ++itComm) {
        // initialize variables
        selfMinusOther.clear();
        otherMinusSelf.clear();

        // do the sync
        try {
            if ((*syncAgentIt)->isStringReconMethod() and
                !(*syncAgentIt)->SyncClient(*itComm, selfMinusOther, otherMinusSelf, CustomResult)) {
                Logger::gLog(Logger::METHOD, "Sync to " + (*itComm)->getName() + " failed!");
                syncSuccess = false;
            } else {
                if (!(*syncAgentIt)->SyncClient(*itComm, selfMinusOther, otherMinusSelf)) {
                    Logger::gLog(Logger::METHOD, "Sync to " + (*itComm)->getName() + " failed!");
                    syncSuccess = false;
                }
            }
        } catch (SyncFailureException s) {
            Logger::error_and_quit(s.what());
            return false;
        }

        // add any items that were found in the reconciliation
        list<DataObject *>::iterator itDO;
        for (itDO = otherMinusSelf.begin(); itDO != otherMinusSelf.end(); itDO++)
            addElem(*itDO);

        if (!isRecon) {
            delElemGroup(selfMinusOther);
        }
        if ((*syncAgentIt)->isStringReconMethod()) { // If it is string reconciliation
            clock_t time = clock();
            if (!(*syncAgentIt)->reconstructString(
                    myString,
                    myData)) { syncSuccess = false; } // reconstruct the string based on the new information from set reconciliation
            pushCustomResult("Str Reconstruction Time", (double) (clock() - time) / CLOCKS_PER_SEC);
        }

    }

    Logger::gLog(Logger::METHOD, "Sync succeeded:  " + toStr(syncSuccess));
    return syncSuccess;

}

// add element

void GenSync::addElem(DataObject *newDatum) {
    Logger::gLog(Logger::METHOD, "Entering GenSync::addElem");
    // store locally
    myData.push_back(newDatum);

    // update sync methods' metadata
    vector<shared_ptr<SyncMethod>>::iterator itAgt;
    for (itAgt = mySyncVec.begin(); itAgt != mySyncVec.end(); ++itAgt) {
        if (!(*itAgt)->addElem(newDatum))
            Logger::error_and_quit("Could not add item " + newDatum->to_string() +
                                   ".  Please considering increasing the number of bits per set element.");
    }

    // update file
    if (outFile != nullptr)
        (*outFile) << newDatum->to_string() << endl;
}

// add string

bool GenSync::addStr(DataObject *newStr, bool backtrack) {
    Logger::gLog(Logger::METHOD, "Entering GenSync::addStr");
    // store locally
    myString = newStr;
    bool back_track_success = false;
    // update synch methods' metadata
    vector<shared_ptr<SyncMethod>>::iterator itAgt;
    for (itAgt = mySyncVec.begin(); itAgt != mySyncVec.end(); ++itAgt) {
        vector<DataObject *> Elems;
        back_track_success = (*itAgt)->addStr(newStr, Elems, backtrack);
//        if (Elems.empty()) return false;
        for (DataObject *item : Elems) addElem(item);
    }

    // update file
    if (outFile != nullptr)
        (*outFile) << newStr->to_string() << endl;

    return back_track_success;
}
// delete element

void GenSync::delElemGroup(list<DataObject *> newDatumList) {
    //throw new UnimplementedMethodException("GenSync::delElem");
    // There are only 2 types, numbes of strings (check fact) handle both
    Logger::gLog(Logger::METHOD, "Entering GenSync::delElem");

    std::map<ZZ, bool> delmap;
    for (auto it : newDatumList) {
        delmap[it->to_ZZ()] = true;
    }

    list<DataObject *> lst;
    for (auto item : myData) {
        if (delmap.find(item->to_ZZ()) != delmap.end()) {
            lst.push_back(item);
            for (auto itAgt = mySyncVec.begin(); itAgt != mySyncVec.end(); ++itAgt) {
                if (!(*itAgt)->delElem(item)) {
                    Logger::error_and_quit("Could not del item . check if item is first inserted.");
                }
            }
        }
    }

    for (auto it = lst.begin(); it != lst.end(); ++it) {
        myData.remove(*it);
    }

}



// insert a communicant in the vector at the index position

void GenSync::addComm(shared_ptr<Communicant> newComm, int index) {
    Logger::gLog(Logger::METHOD, "Entering GenSync::addComm");
    vector<shared_ptr<Communicant>>::iterator itComm;

    itComm = myCommVec.begin();
    if (index == 0) {
        myCommVec.push_back(newComm);
    } else {
        advance(itComm, index - 1);
        myCommVec.insert(itComm, newComm);
    }

}

/**
 *  delete a communicant from the vector at the index position
 * */
void GenSync::delComm(int index) {
    vector<shared_ptr<Communicant>>::iterator itComm;

    itComm = myCommVec.begin();
    if (index == 0) {
        myCommVec.erase(itComm);
    } else {
        advance(itComm, index - 1);
        myCommVec.erase(itComm);
    }

}

void GenSync::delComm(shared_ptr<Communicant> oldComm) {
    myCommVec.erase(std::remove(myCommVec.begin(), myCommVec.end(), oldComm), myCommVec.end());
}

int GenSync::numComm() {
    return myCommVec.size();
}

// insert a syncmethod in the vector at the index position
void GenSync::addSyncAgt(shared_ptr<SyncMethod> newAgt, int index) {
    Logger::gLog(Logger::METHOD, "Entering GenSync::addSyncAgt");
    // create and populate the new agent
    list<DataObject *>::iterator itData;
    for (itData = myData.begin(); itData != myData.end(); itData++)
        if (!newAgt->addElem(*itData))
            Logger::error_and_quit("Was not able to add an item to the next syncagent.");

    // add the agent to the sync agents vector
    auto idxIter = mySyncVec.begin();
    advance(idxIter, index);
    mySyncVec.insert(idxIter, newAgt);
}


// delete a syncmethod from the vector at the index position

void GenSync::delSyncAgt(int index) {
    mySyncVec.erase(getSyncAgt(index));
}

vector<shared_ptr<SyncMethod>>::iterator GenSync::getSyncAgt(int index) {
    vector<shared_ptr<SyncMethod>>::iterator itAgt;

    itAgt = mySyncVec.begin();
    advance(itAgt, index);

    return itAgt;
}

const list<DataObject *> GenSync::dumpElements() {
    return myData;
}

const DataObject *GenSync::dumpString() {
    return myString;
}

const void GenSync::pushCustomResult(string name, double res) {
    CustomResult[name] = res;
}

const double GenSync::getCustomResult(string name) {
    return CustomResult[name];
}

const long GenSync::getXmitBytes(int commIndex) const {
    return myCommVec[commIndex]->getXmitBytes();
}

const long GenSync::getRecvBytes(int commIndex) const {
    return myCommVec[commIndex]->getRecvBytes();
}

const long GenSync::getXmitBytesTot(int commIndex) const {
    return myCommVec[commIndex]->getXmitBytesTot();
}

const long GenSync::getRecvBytesTot(int commIndex) const {
    return myCommVec[commIndex]->getRecvBytesTot();
}

const double GenSync::getSyncTime(int commIndex) const {
    shared_ptr<Communicant> comm = myCommVec[commIndex];

    // true iff there has been a sync (since sync resets comm counters)
    if (comm->getTotalTime() != comm->getResetTime()) {
        return (double) (clock() - comm->getResetTime()) / CLOCKS_PER_SEC;
    } else {
        return (double) comm->getTotalTime() / CLOCKS_PER_SEC;
    }

}

int GenSync::getPort(int commIndex) {
    // null iff comm isn't a CommSocket
    if (auto cs = dynamic_cast<CommSocket *>(myCommVec[commIndex].get())) {
        return cs->getPort();
    } else {
        return -1;
    }

}

const long GenSync::getVirMem(int syncIndex) const {
    return mySyncVec[syncIndex]->getVirMem();
}

// Builder methods

GenSync GenSync::Builder::build() {
    // variables of possible use
    vector<shared_ptr<Communicant>> theComms;
    vector<shared_ptr<SyncMethod>> theMeths;

    // check pre-conditions
    if (proto == SyncProtocol::UNDEFINED)
        Logger::error_and_quit("The synchronization protocol has not been defined.");
    if (comm == SyncComm::UNDEFINED)
        Logger::error_and_quit("No communication protocol defined.");
    if (stringProto != StringSyncProtocol::UNDEFINED and proto == SyncProtocol::UNDEFINED)
        Logger::error_and_quit("String synchronization protocol requires base set sync protocol.");

    // setup
    switch (comm) {
        case SyncComm::socket:
            myComm = make_shared<CommSocket>(port, host);
            Logger::gLog(Logger::METHOD, "Connecting to host " + host + " on port " + toStr(port));
            break;
        case SyncComm::string:
            myComm = make_shared<CommString>(ioStr, base64);
            Logger::gLog(Logger::METHOD, "Connecting to " + toStr(base64 ? "base64" : "") + " string " + ioStr);
            break;
        default:
            throw invalid_argument("I don't know how to set up communication through the provided requested mode.");
    }
    theComms.push_back(myComm);

    invalid_argument noMbar("Must define <mbar> explicitly for this sync.");
    switch (proto) {
        case SyncProtocol::CPISync:
            if (mbar == Builder::UNDEFINED)
                throw noMbar;
            myMeth = make_shared<ProbCPISync>(mbar, bits, errorProb);
            break;
        case SyncProtocol::InteractiveCPISync:
            if (mbar == Builder::UNDEFINED)
                throw noMbar;
            myMeth = make_shared<InterCPISync>(mbar, bits, errorProb, numParts);
            break;
        case SyncProtocol::OneWayCPISync:
            if (mbar == Builder::UNDEFINED)
                throw noMbar;
            myMeth = make_shared<CPISync_HalfRound>(mbar, bits, errorProb);
            break;
        case SyncProtocol::FullSync:
            myMeth = make_shared<FullSync>();
            break;
        case SyncProtocol::IBLTSync:
            myMeth = make_shared<IBLTSync>(numExpElem, bits);
            break;
        case SyncProtocol::OneWayIBLTSync:
            myMeth = make_shared<IBLTSync_HalfRound>(numExpElem, bits);
            break;
        case SyncProtocol::IBLTSyncSetDiff:
            myMeth = make_shared<IBLTSync_SetDiff>(mbar, bits);
            break;
        default:
            throw invalid_argument("I don't know how to synchronize with this protocol.");
    }

    switch (stringProto) {
        case StringSyncProtocol::kshinglingSync:
            myMeth = make_shared<kshinglingSync>(proto, shingleLen, stopWord);
            break;
        case StringSyncProtocol::SetsOfContent:
            myMeth = make_shared<SetsOfContent>(TerminalStrSize, lvl, numParts, proto, shingleLen, baseSpace);
            break;
        case StringSyncProtocol::RCDS:
            myMeth = make_shared<RCDS>(proto);
            break;
        default: // do nothing
            break;
    }

    theMeths.push_back(myMeth);

    return GenSync(theComms, theMeths);
}

// static consts

const string GenSync::Builder::DFT_HOST = "localhost";
const string GenSync::Builder::DFT_IO;
const int GenSync::Builder::DFT_ERROR = 8;