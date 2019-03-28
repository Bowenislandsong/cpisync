//
// Created by Bowen Song on 3/28/19.
//
// A shell for "SetsofContent" to handle folder and file
// Using heuristics compare digest of two files that has the same name, size.
// if same, skip reconciling them.
// If files are too small, (under certain limit), if diff, just full sync


#ifndef CPISYNCLIB_RCDS_H
#define CPISYNCLIB_RCDS_H

class RCDS : public SyncMethod {
public:
    RCDS();

    ~RCDS() = default;
};

#endif //CPISYNCLIB_RCDS_H
