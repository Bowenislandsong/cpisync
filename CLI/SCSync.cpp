//
// Created by Bowen Song on 3/7/19.
//

#include "SCSync.h"

commandline_interface::commandline_interface() : STR_RECON_PROTO(GenSync::StringSyncProtocol::SetsOfContent),
                                                 SET_RECON_PROTO(GenSync::SyncProtocol::CPISync),
                                                 isMute(false), canSync(true) {
    param.lvl = NOT_SET;
    param.par = NOT_SET;
    param.shingle_size = NOT_SET;
    param.port = 8001;
    param.host_name = "localhost";
}


commandline_interface::~commandline_interface() {}


void commandline_interface::help() {
    cout << "SCSync is a string reconciliation protocol dependenting on set reconciliation algorithms. "
            "The protocol save communication cost by exploiting file content similarities."
            "The default set up is using Sets of Content protocol based on CPISync."
            "This protocol currently supports 2G TEXT FILE synchronization."
            "Please find source Code at https://github.com/Bowenislandsong/cpisync " << endl;
    cout << "Usage : SCSync [OPTION] ... <SRC> ... <DEST>" << endl;
    cout << "Options" << endl;
}

void commandline_interface::Sync() {
    // check if all parameters are there
    if (!canSync and error_msg.empty()) help();
    else if (canSync and !error_msg.empty())
        cout << error_msg << " For how to use SCSync, use -h to show help." << endl;
    else { // all good, let's sync

        DataObject *txt_pt = new DataObject(scanTxtFromFile((param.file_path), INT_MAX));

        // quote file size for both
        int src_size = 1000000, dest_size = 1000000;

        // set the unset parameters;
        if (STR_RECON_PROTO == GenSync::StringSyncProtocol::SetsOfContent) {
            if (param.lvl <= NOT_SET)
                param.lvl = (int) floor(log10(max(src_size, dest_size)));
            if (param.par <= NOT_SET)param.par = 4;
            param.shingle_size = 2;
        } else if (STR_RECON_PROTO == GenSync::StringSyncProtocol::kshinglingSync) {
            if (param.shingle_size <= NOT_SET)
                param.shingle_size = (int) floor(log2(max(src_size, dest_size)));
            Logger::error_and_quit("kshingling Currently not implemented in SCSync.");
        }



        // get the strings inserted
        GenSync mySync = GenSync::Builder().
                setStringProto(STR_RECON_PROTO).
                setProtocol(SET_RECON_PROTO).
                setComm(GenSync::SyncComm::socket).
                setTerminalStrSize(10).
                setNumPartitions(param.par).
                setHost(param.host_name).
                setShingleLen(param.shingle_size).
                setSpace(param.par * 2).
                setlvl(param.lvl).
                setPort(param.port).
                build();


        clock_t start_time = clock();
        mySync.addStr(txt_pt, false);
        double str_time = (double) (clock() - start_time) / CLOCKS_PER_SEC;
        bool success;
        if (param.isSRC) {
            start_time = clock();
            Logger::gLog(Logger::COMM, "created a server process");
            success = mySync.listenSync(0, false); // src
        } else {

            start_time = clock();
            success = mySync.startSync(0, false); // dest
        }

        double totalTime = (double) (clock() - start_time) / CLOCKS_PER_SEC;
        long bytesRTot = mySync.getRecvBytesTot(0);
        long bytesXTot = mySync.getXmitBytesTot(0);

        if (!isMute) {
            cout << "Number of Bytes Transmitted: " << bytesXTot << endl;
            cout << "Number of Bytes Received: " << bytesRTot << endl;
            if (STR_RECON_PROTO == GenSync::StringSyncProtocol::SetsOfContent)
                cout << "Literal Data Transferred in Bytes: " << mySync.getCustomResult("Literal comm") << endl;
            cout << "Time spent on preparation (Partition Tree):" << str_time << endl;
            cout << "Time spent on reconciliation: " << totalTime << endl;

            if (success) {

                string new_file = mySync.dumpString()->to_string();
                cout << "Set Recon Success, new file size in bytes: " << new_file.size() << endl;

                writeStrToFile(dest_path, new_file);
            } else
                cout << "Set Recon Failed, File NOT Synchronized." << endl;

            cout << "\n" << "Total Number of Bytes Communicated: " << bytesXTot + bytesRTot << endl;

        }
        delete txt_pt;
    }
}


void commandline_interface::parse_arg(std::pair<option, int> arg) {
    if (arg.first == option::UNDEFINED) return;
    switch (arg.first) {
        case option::U_CPI:
            SET_RECON_PROTO = GenSync::SyncProtocol::CPISync;
            break;
        case option::U_IBLT:
            SET_RECON_PROTO = GenSync::SyncProtocol::IBLTSyncSetDiff;
            break;
        case option::U_InterCPI:
            SET_RECON_PROTO = GenSync::SyncProtocol::InteractiveCPISync;
            break;
        case option::help:
            help();
            break;
        case option::SetsOfC:
            STR_RECON_PROTO = GenSync::StringSyncProtocol::SetsOfContent;
            break;
        case option::KShingle:
            STR_RECON_PROTO = GenSync::StringSyncProtocol::kshinglingSync;
            break;
        case option::mute:
            isMute = true;
            break;
        case option::shingleSize:
            param.shingle_size = arg.second;
            break;
        case option::lvl:
            param.lvl = arg.second;
            break;
        case option::role:
            if (arg.second == 0) param.isSRC = true;
            else param.isSRC = false;
            break;
        case option::par:
            param.par = arg.second;
            break;
        case option::port:
            param.port = arg.second;
            break;
        default:
            break;
    };
}
