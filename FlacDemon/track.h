//
//  Track.h
//  FlacDemon
//
//  Created by merryclarke on 15/06/2015.
//  Copyright (c) 2015 c4software. All rights reserved.
//

#ifndef __FlacDemon__Track__
#define __FlacDemon__Track__

#include <iostream>
#include <time.h>
#include "FlacDemonNameSpace.h"
#include "File.h"

class FlacDemon::Track {
private:
    void init();
    
protected:
    time_t dateAdded;
    unsigned int playCount;
    time_t trackTime;
    
    std::string * filepath = NULL;
//    std::string * albumuuid = NULL;
    
    std::map<std::string, long> * trackinfo = NULL;
    fd_keymap * keymap = NULL;
    
public:
    friend FlacDemon::File;
    
    string * uuid = NULL;
    FlacDemon::File* file = NULL;
    struct MediaStreamInfo * mediaStreamInfo = NULL;

    Track (FlacDemon::File* file = NULL);
    Track (fd_keymap * keymap);
    ~Track();
    
    void incrementPlaycount ();
    void decrementPlaycount ();
    
    string * valueForKey (const char * key);
    string * valueForKey (std::string * key);
    void setValueForKey(std::string * value, std::string* key);
//    void setValueForKey(const unsigned char * value, const std::string *key);
    
    std::string * standardiseMetaValue(std::string * value, std::string * key);

    long getTrackInfoForKey(const char * key);
    long getTrackInfoForKey(std::string * key);
    
    void setFile(FlacDemon::File*);
    void setValuesFromAudioContext(AVCodecContext*);
    
    void initInfo();
    int openFilePath();
    
//    void addGroup(&Group group);
//    void removeGroup(&Group group);
    
};

#endif /* defined(__FlacDemon__Track__) */
