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
        role,
        mute,
        help,
        END
    };

    struct Param {
        int lvl, par, shingle_size, port;
        string remoteHost_src, remoteHost_dest, host_name, file_path;
        bool isSRC;
    };

    // print out man page
    void help();

    // parse input tags one by one
    void parse_arg(std::pair<option, int> arg);

    // check paramters, if not right, spit out man page
    // sync the two files if possible
    void Sync();


    void addSRC(string path) {
        if (path.find(":") != string::npos) { //it is on remote host
            param.remoteHost_src = path.substr(0, path.find(":")); // assume there is only one remote host
            path = path.substr(path.find(":") + 1);
            cout << "remote host addr:" << param.remoteHost_src << endl;
        } else { param.remoteHost_src = "localhost"; }

        if (!isValidFile(path)) {
            canSync = false;
            error_msg = "Invalid source path";
        } else {
            src_path = path;
        }
    };

    void addDST(string path) {
        if (path.find(":") != string::npos) { //it is on remote host
            param.remoteHost_dest = path.substr(0, path.find(":")); // assume there is only one remote host
            path = path.substr(path.find(":") + 1);
            cout << "remote host addr:" << param.remoteHost_dest << endl;
        } else { param.remoteHost_dest = "localhost"; }

        if (!isValidFile(path)) {
            canSync = false;
            error_msg = "Invalid destination path";
        } else {
            dest_path = path;
        }
    };

    void parse_path(string path) {

        if (path.find(":") != string::npos) { //it is on remote host
            param.host_name = path.substr(0, path.find(":")); // assume there is only one remote host
            path = path.substr(path.find(":") + 1);
            cout << "remote host addr: " << param.host_name << endl;
        } else { param.host_name = "localhost"; }


        param.file_path = path;


    }

    void cmdCall(vector<string> flags) {
        string dest = "/Users/bowen/Documents/cpisync/SCSync -role=1 "; // dest is 1
        string src = "/Users/bowen/Documents/cpisync/SCSync -role=0 "; // src is 0
        for (string f : flags) {
            dest += f + " ";
            src += f + " ";
        }
        dest += param.remoteHost_dest + ":";
        dest += dest_path;

        src += param.remoteHost_src + ":";
        src += src_path;

        if (param.remoteHost_src != "localhost") src = "ssh " + param.remoteHost_src + " " + src;
        if (param.remoteHost_dest != "localhost") dest = "ssh " + param.remoteHost_dest + " " + dest;
        pid_t pID = fork();
        if (pID == 0) system(dest.c_str());
        else if (pID > 0) {
            int state;
            system(src.c_str());
            waitpid(pID, &state, 0);
        } else Logger::error_and_quit("Failed to sync from fork");

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
    string src_path, dest_path, error_msg;
    Param param;


    map<string, enum option> option_map{
            {"-cpi",      option::U_CPI},
            {"-intercpi", option::U_InterCPI},
            {"-iblt",     option::U_IBLT},
            {"-sc",       option::SetsOfC},
            {"-kshingle", option::KShingle},
            {"-l",        option::lvl},
            {"-par",      option::par},
            {"-p",        option::port},
            {"-s",        option::shingleSize},
            {"-role",     option::role},
            {"-mute",     option::mute},
            {"-h",        option::help},
    };

    static const int NOT_SET = 0;


    // check if file exist
    // check if it is txt file
    bool isValidFile(const string &path) {
        try {
            isPathExist(path);
        } catch (exception &e) {
            return false;
        }
        return true;
    }

};

//TODO: clean up code, this way is bad, but I'm out of time.

// example ags
//  /Users/bowen/Desktop/src.txt /Users/bowen/Desktop/dest.txt
//-role=0 localhost:/Users/bowen/Desktop/dest.txt
//-role=1 localhost:/Users/bowen/Desktop/src.txt

int main(int argc, char *argv[]) {
    // user calls a main command 
    // ---- SCSync [-flags] [remote]:<src> [remote]:<dest>

    // split into two cmds for src and dest 
    // ---- SCSync -role=<src 0 or dest 1> [-flags] [remote]:<src> [remote]:<dest>

    commandline_interface cli;
    if (string(argv[1]).substr(0, 5) == "-role") { // split cmd
        for (int i = 1; i < argc - 1; ++i) { // last one is either src or dest address
            cli.parse_arg(cli.getTag(string(argv[i])));
        }
        cli.parse_path(argv[argc - 1]);
        cli.Sync();
    } else if (string(argv[1]) == "-h") {
        cli.help();

    } else {// main command

        vector<string> flags;
        for (int i = 1; i < argc - 2; ++i) { // last two are src and dest address
            flags.push_back(string(argv[i]));
        }

        cli.addSRC(string(argv[argc - 2]));
        cli.addDST(string(argv[argc - 1]));
        cli.cmdCall(flags);

    };
    return 0;
}

#endif //CPISYNCLIB_SCSYNC_H
