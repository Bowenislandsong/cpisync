//
// Created by Bowen Song on 3/30/19.
//

#include "RCDS.h"

//choose set recon proto
// Mini terminal size default 10
// if not set, levels and partition numbers are subject to different string size
RCDS::RCDS(GenSync::SyncProtocol base_set_proto, size_t terminal_str_size, size_t levels, size_t partition)
        : baseSyncProtocol(base_set_proto), TermStrSize(terminal_str_size), Levels(levels), Partition(partition) {

    Quota_mode = true; // If true, we dont change the files after sync and just give a comm quota
}

bool RCDS::addStr(DataObject *str, vector<DataObject *> &datum, bool sync) {
    if (!isFile((FolderName = str->to_string()))) { // we currently just care about folder
        if (FolderName.back() == '/')FolderName.pop_back();
        for (string f_name : walkRelDir(FolderName))
            setPointers.push_back(new DataObject(f_name));
        return true;
    }
    return false;
}

// authoritative source
bool RCDS::SyncServer(const shared_ptr<Communicant> &commSync, list<DataObject *> &selfMinusOther,
                      list<DataObject *> &otherMinusSelf) {
    Logger::gLog(Logger::METHOD, "Entering RCDS::SyncServer");

    commSync->commListen();
    cout << "we linked-server" << endl;
    // use CPI/interactive CPI (fixed)
    vector<DataObject *> heuristic_set = heuristic_check(setPointers);
    setReconServer(commSync, 10e4, sizeof(size_t), heuristic_set, selfMinusOther, otherMinusSelf);

    (FolderName.back() == '/') ? 0 : FolderName += "/";

    commSync->commSend(selfMinusOther.size());
    for (DataObject *f:selfMinusOther) {
        string f_name = hash2filename(f->to_ZZ());

        commSync->commSend((f_name.empty()) ? "$" : f_name);
        if (f_name.empty())continue;
        int mode = 0;
        (commSync->commRecv_byte() == SYNC_OK_FLAG) ? mode = 1 : mode = 2;

        if (mode == 2) {
            Logger::gLog(Logger::METHOD, "We use full sync");
            commSync->commSend(scanTxtFromFile(FolderName + f_name, INT_MAX));
        } else if (mode == 1) {
            Logger::gLog(Logger::METHOD, "We use RCDS");
            int levels = (int) floor(log10(getFileSize(FolderName + f_name)));
            int par = 4;
            commSync->commSend(levels);
            stringSyncServer(commSync, FolderName + f_name, levels, par);
        } else Logger::error_and_quit("Unkonwn Sync Mode, should never happen in RCDS");

    }
    commSync->commClose();
    return true;
}

bool RCDS::SyncClient(const shared_ptr<Communicant> &commSync, list<DataObject *> &selfMinusOther,
                      list<DataObject *> &otherMinusSelf) {
    Logger::gLog(Logger::METHOD, "Entering RCDS::SyncClient");

    commSync->commConnect();
    cout << "we linked-Cient" << endl;
    // use CPI/interactive CPI (fixed)
    vector<DataObject *> heuristic_set = heuristic_check(setPointers);

    setReconClient(commSync, 10e4, sizeof(size_t), heuristic_set, selfMinusOther, otherMinusSelf);

    (FolderName.back() == '/') ? 0 : FolderName += "/";

    size_t diff_size = commSync->commRecv_size_t();
    for (int i = 0; i < diff_size; ++i) {
        //mode 1: I have a file, lets sync.
        //mode 2: i don't have this file, send me the whole thing.
        int mode = 0;
        string f_name;
        if ((f_name = commSync->commRecv_string()) == "$")continue;
        cout << FolderName + f_name << endl;
        if (isPathExist(FolderName + f_name)) {
            commSync->commSend(SYNC_OK_FLAG);
            mode = 1;
        } else {
            commSync->commSend(SYNC_NO_INFO);
            mode = 2;
        }

        if (mode == 2) {
            cout<<"mode2"<<endl;
            Logger::gLog(Logger::METHOD, "We use full sync");
            if (Quota_mode)
                commSync->commRecv_string();
            else
                writeStrToFile(FolderName + f_name, commSync->commRecv_string());
        } else if (mode == 1) {
            cout<<"mode1"<<endl;
            Logger::gLog(Logger::METHOD, "We use RCDS");
            int levels = commSync->commRecv_int();
            int par = 4;
            string syncContent = stringSyncClient(commSync, FolderName + f_name, levels, par);
            if (!Quota_mode) writeStrToFile(FolderName + f_name, syncContent);

        } else Logger::error_and_quit("Unkonwn Sync Mode, should never happen in RCDS");

    }

    commSync->commClose();
    return true;
}

bool RCDS::stringSyncServer(const shared_ptr<Communicant> &commSync, string absfilename, int level, int partition) {

    return true;
}

string RCDS::stringSyncClient(const shared_ptr<Communicant> &commSync, string absfilename, int level, int partition) {
    return "";
}