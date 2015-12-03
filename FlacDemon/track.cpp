//
//  Track.cpp
//  FlacDemon
//
//  Created by merryclarke on 15/06/2015.
//  Copyright (c) 2015 c4software. All rights reserved.
//

#include "TrackFile.h"

FlacDemon::Track::Track(FlacDemon::File* file){
    this->init();
    if(file)
        this->setFile(file);
}
FlacDemon::Track::Track(fd_keymap * ikeymap){
    Track();
    this->keymap = ikeymap;
    this->filepath = this->keymap->at("filepath");
}
FlacDemon::Track::~Track(){

}
void FlacDemon::Track::init(){
    this->playCount = 0;
    this->dateAdded = 0;
    this->trackTime = 0;
    
    this->trackinfo = new std::map<std::string, long>{
        {"playcount", 0},
        {"dateadded", 0},
        {"tracktime", 0},
        {"verified", 0}
    };
}
void FlacDemon::Track::setFile(FlacDemon::File * file){
    this->file = file;
    this->trackinfo->at("tracktime") = (long) this->file->mediaStreamInfo->duration / (0.001 * this->file->mediaStreamInfo->sampleRate);
    
    this->filepath = new std::string(*file->filepath);
}

std::string * FlacDemon::Track::valueForKey (const char * key){
    std::string tkey = key;
    return this->valueForKey(&tkey);
}
std::string * FlacDemon::Track::valueForKey(std::string* key){
    std::string * value = nullptr;
    if(key->compare("filepath")==0){
        return this->filepath;
    }
    if(this->keymap)
        value = this->keymap->at(*key);
    else if(this->file){
        if(key->compare("albumuuid")==0){
            return this->file->albumuuid;
        } else if(key->compare("errorflags")==0){
            return new std::string(std::to_string(this->file->errorFlags)); //this will need to be released
        }
        if((value = this->file->getMetaDataEntry(key)) != nullptr){
            value = this->standardiseMetaValue(value, key);
        }
    }
    //query db if no file?
    if(value == nullptr){
        value = new std::string(std::to_string(this->getTrackInfoForKey(key)));
    }
    return value;
}
std::string * FlacDemon::Track::standardiseMetaValue(std::string *value, std::string *key){
    if(key->compare("disc") == 0){
        int ivalue = std::stoi(*value);
        value = new std::string(std::to_string(ivalue));
    }
    
    return value;
}
//template <class KValue>
void FlacDemon::Track::setValueForKey(std::string * value, std::string *key){
    
}
//void FlacDemon::Track::setValueForKey(const unsigned char * value, const std::string *key){
//    
//}
long FlacDemon::Track::getTrackInfoForKey(const char * key){
    std::string tkey = key;
    return this->getTrackInfoForKey(&tkey);
}
long FlacDemon::Track::getTrackInfoForKey(std::string * key){
    if(this->trackinfo->count(*key)){
        return this->trackinfo->at(*key);
    }
    return -1;
}
void FlacDemon::Track::setTrackInfoForKey(const char * key, long value){
    std::string tkey = key;
    this->setTrackInfoForKey(&tkey, value);
}
void FlacDemon::Track::setTrackInfoForKey(std::string * key, long value){
    cout << this->trackinfo->at(*key) << endl;
    this->trackinfo->at(*key) = value;
}

void FlacDemon::Track::initInfo(){
    cout << time(nullptr) << endl;
    this->trackinfo->at("dateadded") = time(nullptr);
}
int FlacDemon::Track::openFilePath(){
    //will need to do more checks, path exists, relative pathname, file opened succesfully etc
    if(this->filepath){
        if(!this->file){
            this->file = new FlacDemon::File(this->filepath, false);
        } else {
            this->file->openFormatContext(true); //reset
        }
    }
    
    return this->file==nullptr ? 0 : 1;
}