//
// Created by Bowen Song on 3/7/19.
//

#ifndef CPISYNCLIB_SCSYNC_H
#define CPISYNCLIB_SCSYNC_H

#include "Auxiliary.h"
#include "SetsOfContent.h"
#include "StrataEst.h"
#include "IBLTSync_SetDiff.h"
#include "CPISync.h"
#include "InterCPISync.h"
#include "ProbCPISync.h"
#include "GenSync.h"
#include "ForkHandle.h"
#include "kshinglingSync.h"
#include "DataObject.h"

class commandline_interface {
public:
    commandline_interface();

    ~commandline_interface();

    enum class option {
        UNDEFINED,
        BEGIN,
        U_CPI = BEGIN,
        U_InterCPI,
        U_IBLT,//all set recon
        SetsOfC,
        KShingle,// all string recon
        lvl, //for sets of content
        par, //for sets of content
        shingleSize,// for kshingle and UD
        port,
        remoteHost,
        mute,
        help,
        END
    };

    struct Param {
        int lvl, par, shingle_size, port;
        string remoteHost;
        bool isSRC;
    };

    // parse input tags one by one
    void parse_arg(std::pair<option, int> arg);

    // check paramters, if not right, spit out man page
    // sync the two files if possible
    void Sync();


    void addSRC(const string &path) {
        if (!isValidFile(path)) {
            canSync = false;
            error_msg = "Invalid source path";
        } else {
            src_path = path;
        }
    };

    void addDST(const string &path) {
        if (!isValidFile(path)) {
            canSync = false;
            error_msg = "Invalid destination path";
        } else {
            dst_path = path;
        }
    };

    // translate tags
    pair<option, int> getTag(string tag) {
        if (option_map.find(tag) != option_map.end())
            return {option_map.find(tag)->second, 0};
        else if (option_map.find(tag.substr(0, tag.find("="))) != option_map.end())
            return {option_map.find(tag.substr(0, tag.find("=")))->second, stoi(tag.substr(tag.find("=") + 1))};
        else
            return {option::UNDEFINED, 0};
    }

private:

    GenSync::StringSyncProtocol STR_RECON_PROTO;
    GenSync::SyncProtocol SET_RECON_PROTO;
    bool isMute, canSync;
    string src_path, dst_path, error_msg;
    Param param;


    map<string, enum option> option_map{
            {"-cpi", option::U_CPI},
            {"-intercpi", option::U_InterCPI},
            {"-iblt", option::U_IBLT},
            {"-sc", option::SetsOfC},
            {"-kshingle", option::KShingle},
            {"-l", option::lvl},
            {"-par", option::par},
            {"-p", option::port},
            {"-s", option::shingleSize},
            {"-rh", option::remoteHost},
            {"-mute", option::mute},
            {"-h", option::help},
    };

    static const int NOT_SET = 0;

    // print out man page
    void help();

    // check if file exist
    // check if it is txt file
    bool isValidFile(const string &path) {
        return isFile(path) and path.substr(path.find(".")) == ".txt";
    }

};


int main(int argc, char *argv[]) {
    commandline_interface cli;
    for (int i = 1; i < argc - 2; ++i) { // last two are src and dest address
        cli.parse_arg(cli.getTag(string(argv[i])));
    }

    cli.addSRC(string(argv[argc - 2]));
    cli.addDST(string(argv[argc - 1]));

    cli.Sync();
}

#endif //CPISYNCLIB_SCSYNC_H
