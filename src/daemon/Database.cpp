/***********************************************************************
 * Database.cpp : SQLite Database management
 * part of FlacDemon
 ************************************************************************
 *  Copyright (c) 2016 Meriadoc Clarke.
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#include "Database.h"

struct SQLStatements{
    char * fields = nullptr;
    const char * createTableTracksFormat = "create table if not exists `tracks` (id INTEGER PRIMARY KEY AUTOINCREMENT, %s)";
    const char * addTrackFormat = "insert into tracks (%s) values(%s)";
    const char * createTableFiles = "create table if not exists `associate_files` (id INTEGER PRIMARY KEY AUTOINCREMENT, filepath varchar(255), albumuuid varchar(255), flags INTEGER, errorflags INTERGER)";
    const char * addFileFormat = "insert into associate_files (filepath, albumuuid, flags, errorflags) values(%s)";
    const char * setValueFormat = "update tracks set %s=`%s` where id=%lu";
    const char * getValueFormat = "select %s from `tracks` where id=%lu";
    const char * getJSONFormat = "select * from `tracks` where id=%lu";
    const char * getAll = "select * from `tracks`";
} sql_statements;



FlacDemon::Database::Database(){
//    this->sqlSelectStatment = nullptr;

    this->allkeys = new std::vector<std::string>;

    this->metakeys = new std::vector<std::string>{
        "track",
        "title",
        "albumartist",
        "artist",
        "album",
        "genre",
        "composer",
        "disc",
        "year"
    };
    this->trackinfokeys = new std::vector<std::string>{
        "tracktime",
        "playcount",
        "dateadded",
        "filepath",
        "albumuuid",
        "verified",
        "errorflags"
    };
    this->initDB();
    this->initSignals();
    
    char buf[PATH_MAX];
    if(getcwd(buf, PATH_MAX) == nullptr){
        cout << "Error getting current working directory: " << strerror(errno) << endl;
        return;
    }
    this->currentWorkingDirectory = new std::string(buf);
}
FlacDemon::Database::~Database(){
    
}

void FlacDemon::Database::initSignals(){
    auto f = boost::bind(&FlacDemon::Database::signalReceiver, this, _1, _2);
    signalHandler->signals("addAlbumDirectory")->connect(f);
    signalHandler->signals("runSQL")->connect(f);
    
}
void FlacDemon::Database::signalReceiver(const char * signalName, void * arg){
    cout << "signal received: "<< signalName << endl;
    if(strcmp(signalName, "addAlbumDirectory")==0){
        this->addAlbumDirectory(static_cast<FlacDemon::File *>(arg));
    } else if(strcmp(signalName, "runSQL")==0){
        this->runSQL(static_cast<std::string *>(arg));
    }
}
void FlacDemon::Database::addAlbumDirectory(FlacDemon::File *albumDirectory){
    std::cout << "add album directory: " << *albumDirectory->filepath << endl;
    
    albumDirectory->standardisePath();
    
    std::string uuid = this->albumDirectoryUUIDForPath(albumDirectory->filepath);
    albumDirectory->setAlbumDirectoryUUID(&uuid);
    
    albumDirectory->verifyAlbum();
    
    this->add(albumDirectory);
    
    std::vector<FlacDemon::File * > * albumFiles = albumDirectory->getAllFiles();
    
    for(std::vector<FlacDemon::File*>::iterator it = albumFiles->begin(); it != albumFiles->end(); it++){
        this->add(*it);
    }
}
void FlacDemon::Database::add(FlacDemon::File * file){
    if(!file)
        return;
    file->standardisePath();
    if(file->isMediaFile()){
        this->add(file->track);
        return;
    }
    if(!file->albumuuid)
        return;
    
    if(this->hasEntryForFile(file->filepath, "associate_files")){
        cout << "not adding file '" << *file->filepath << "' already in database" << endl;
        return;
    }
    
    cout << "adding album file " << *file->filepath << endl;
    
    std::string values = "'";
    values.append(fd_sqlescape(*(file->filepath)));
    values.append("','");
    values.append(*file->albumuuid);
    values.append("','");
    values.append(std::to_string(file->flags));
    values.append("',");
    values.append(std::to_string(file->errorFlags));

    std::string sql = sql_statements.addFileFormat;
    size_t pos = sql.find("%s");
    sql.erase(pos, 2);
    sql.insert(pos, values);
    this->runSQL(&sql);
}
void FlacDemon::Database::add(FlacDemon::Track *track){
    cout << "adding track: " << *track->file->filepath << endl;
    
    if(this->hasEntryForFile(track->file->filepath, "tracks")){
        cout << "not adding file " << *track->file->filepath << ". already in database" << endl;
        return;
    }
    
    track->initInfo();
    
    std::string sql="'";
    std::string value;
    for(std::vector<std::string>::iterator it = this->allkeys->begin(); it != this->allkeys->end(); it++){
        value = fd_sqlescape( track->valueForKey(&(*it) ) );

        sql.append(value);
        sql.append("','");
    }
    sql.pop_back();
    sql.pop_back();
    
    std::string sql2 = sql_statements.addTrackFormat;
    size_t pos = sql2.find("%s");
    sql2.erase(pos, 2);
    sql2.insert(pos, sql);
    
//    cout << sql2.c_str() << endl;
    this->runSQL(sql2.c_str());
}
sqlite3 * FlacDemon::Database::openDB(){
    sqlite3 * db = nullptr;
    if(sqlite3_open_v2("flacdemon.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK){
        cout << "error opening sqlite3 database, err code: " << sqlite3_errcode(db) << ". err msg: " << sqlite3_errmsg(db);
    }
    return db;
}
void FlacDemon::Database::closeDB(sqlite3* db){
    sqlite3_close_v2(db);
}
void FlacDemon::Database::runSQL(std::string * sql, int (*callback)(void*,int,char**,char**), void * arg){
    this->runSQL(sql->c_str(), callback, arg);
}
void FlacDemon::Database::runSQL(const char *sql,int (*callback)(void*,int,char**,char**), void * arg){
    sqlite3 * db = this->openDB();
    if(!db)
        return;
    char * err = nullptr;

//    cout << "running sql " << sql << endl;
    
    sqlite3_exec(db, sql, callback, arg, &err);
    
    if(err){
        cout << err << endl;
        free(err);
    }
    this->closeDB(db);
}
fd_keymap_vector * FlacDemon::Database::sqlSelect(std::string *isql){
    const char * sql = isql->c_str();
    sqlite3_stmt * stmt;
    if(this->sqlSelectStatment == nullptr){
        if(sql == nullptr)
            return nullptr;
        sqlite3 * db = this->openDB();
        if(!db)
            return nullptr;
        if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK){
            //error
            cout << "Error occured while preparing sql statement: " << sql << endl;
            return nullptr;
        }
        this->sqlSelectStatment = stmt;
    } else {
        stmt = this->sqlSelectStatment;
    }
    fd_keymap_vector * results = new fd_keymap_vector;
    int stepCode, columns = 0, i;
    const unsigned char * value = nullptr;
    const char * key = nullptr;
    while ((stepCode = sqlite3_step(stmt)) == SQLITE_ROW) {
        fd_keymap * result = new fd_keymap;
        columns = sqlite3_column_count(stmt);
        for (i = 0; i < columns; i++) {
            if((value = sqlite3_column_text(stmt, i)) && (key = sqlite3_column_name(stmt, i))){
                result->insert( fd_keypair( std::string(key), std::string(value, value + sqlite3_column_bytes(stmt, i)))); //will need to free the key
            }
            
        }
        results->push_back(result);
    }
    if(stepCode != SQLITE_DONE){
        //error occurred, check result code
        cout << "Error stepping through sql select" << endl;
    }
    if(columns == 0)
        this->clearSelect();

    return results;
}
fd_keymap * FlacDemon::Database::sqlSelectRow(std::string *sql){
    fd_keymap_vector * results = sqlSelect(sql);
    if(results && results->size())
        return results->back();
    else
        return nullptr;
}
std::string FlacDemon::Database::sqlSelectOne(std::string * isql){
    fd_keymap * results = this->sqlSelectRow(isql);
    this->clearSelect();
    if(results && results->size() == 1){
        return results->begin()->second;
    }
    return std::string("");
}
int FlacDemon::Database::sqlCount(std::string *sql){
    return this->sqlCount(sql->c_str());
}
int FlacDemon::Database::sqlCount(const char *sql){
    sqlite3_stmt * stmt;
    
    // create functions to avoid repeat code between this function and sqlSelect

    sqlite3 * db = this->openDB();
    if(!db)
        return 0;
    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK){
        //error
        cout << "Error occured while preparing sql statement: " << sql << endl;
        return 0;
    }
    
    int stepCode, columns, results=0;
    if ((stepCode = sqlite3_step(stmt)) == SQLITE_ROW) {
        columns = sqlite3_column_count(stmt);
        if(columns == 1){
            results = sqlite3_column_int(stmt, 0);
        }
    } else {
        if(stepCode != SQLITE_DONE){
            //error occurred, check result code
            cout << "Error stepping through sql select" << endl;
        }
    }
    sqlite3_finalize(stmt);
    this->closeDB(db);
    return results;
}
void FlacDemon::Database::clearSelect(){
    if(this->sqlSelectStatment != nullptr){
        sqlite3 * db = sqlite3_db_handle(this->sqlSelectStatment);
        sqlite3_finalize(this->sqlSelectStatment);
        this->closeDB(db);
        this->sqlSelectStatment = nullptr;
    }
}
void FlacDemon::Database::initDB(){
//    this->runSQL("drop table if exists `tracks`");
    
    std::string sql="";
    std::string fields="";
    for(std::vector<std::string>::iterator it = this->metakeys->begin(); it != this->metakeys->end(); it++){
        sql.append((*it));
        sql.append(" varchar(255),");
        
        fields.append((*it));
        fields.append(",");
        
        this->allkeys->push_back(*new std::string((*it)));

    }
    std::string value;
    for(std::vector<std::string>::iterator it = this->trackinfokeys->begin(); it != this->trackinfokeys->end(); it++){
        sql.append((*it));
//        sql.append(" unsigned int,");
        sql.append(" varchar(255),");

        fields.append((*it));
        fields.append(",");
        
        this->allkeys->push_back(*(new std::string((*it)))); //messy, use vector of string pointers intstead
    }
    
    sql.pop_back();
    fields.pop_back();
    
    std::string sql2 = sql_statements.createTableTracksFormat;
    size_t pos = sql2.find("%s");
    sql2.erase(pos, 2);
    sql2.insert(pos, sql);
    
    this->runSQL(&sql2, nullptr);
    
    std::string addTrackSQL = sql_statements.addTrackFormat;
    pos = addTrackSQL.find("%s");
    addTrackSQL.erase(pos, 2);
    addTrackSQL.insert(pos, fields);
    sql_statements.addTrackFormat = new char [addTrackSQL.length() + 1];
    std::strcpy((char *)sql_statements.addTrackFormat, addTrackSQL.c_str());
    
    sql_statements.fields = new char [fields.length() + 1];
    std::strcpy(sql_statements.fields, fields.c_str());
    
    this->runSQL(sql_statements.createTableFiles);
}

FlacDemon::Track * FlacDemon::Database::trackForID(unsigned long ID){
    for(std::vector<FlacDemon::Track *>::iterator it = this->openTracks->begin(); it != this->openTracks->end(); it++){
        std::string value = "";
        std::string strID = std::to_string(ID);
        if((value = (*it)->valueForKey("id")).length() && value == strID ){
            return (*it);
        }
    }
    std::string sql = "select * from tracks where id=";
    sql.append(std::to_string(ID));
    fd_keymap * results = this->sqlSelectRow(&sql);
    this->clearSelect();
    if(results->count("filepath")){
        cout << "found track for id " << ID << endl;
        return this->trackWithKeyMap(results);
    }
    return nullptr;
}
FlacDemon::Track * FlacDemon::Database::trackWithKeyMap(fd_keymap *keyMap){
    FlacDemon::Track * track = new FlacDemon::Track(keyMap);
    this->openTracks->push_back(track);
    return track;
}
FlacDemon::Album * FlacDemon::Database::albumForID( unsigned long ID ) {
    //check already open tracks / albums
    
    std::string sql = "select * ";
    return nullptr;
}
std::string FlacDemon::Database::getUUID(){
    uuid_t uuid;
    uuid_generate_random(uuid);
    char s[37];
    uuid_unparse(uuid, s);
    return std::string(s);
}
int FlacDemon::Database::hasEntryForFile(std::string * filepath, const char * table){
    std::string sql = "select count(*) from ";
    sql.append(table);
    sql.append(" where filepath='");
    sql.append(fd_sqlescape(*filepath));
    sql.append("'");
    int results = this->sqlCount(&sql);
    if(results > 0){
        return 1;
    }
    return 0;
}
std::string FlacDemon::Database::albumDirectoryUUIDForPath(std::string * path){
    std::string sql = "select (albumuuid) from associate_files where filepath='";
    sql.append(*path);
    sql.append("'");
    std::string value = this->sqlSelectOne(&sql);
    if( ! value.length() )
        value = this->getUUID();
    return value;
}
int FlacDemon::Database::setValue(unsigned long ID, std::string * key, std::string * value){
    std::string sql = sql_statements.setValueFormat;
    size_t pos = sql.find("%s");
    sql.erase(pos, 2);
    sql.insert(pos, *key);
    
    pos = sql.find("%s");
    sql.erase(pos, 2);
    sql.insert(pos, *value);
    
    pos = sql.find("%lu");
    sql.erase(pos, 3);
    sql.insert(pos, std::to_string(ID));
    this->runSQL(&sql);
    
    return 0;
}
std::string FlacDemon::Database::getValue(unsigned long ID, std::string * key){
    std::string sql = sql_statements.getValueFormat;
    size_t pos = sql.find("%s");
    sql.erase(pos, 2);
    sql.insert(pos, *key);
    
    pos = sql.find("%lu");
    sql.erase(pos, 3);
    sql.insert(pos, std::to_string(ID));
    return this->sqlSelectOne(&sql);
}
std::string FlacDemon::Database::getJSONForID(int uid){
    std::string sql = sql_statements.getJSONFormat;
    std::string search = "%lu";
    std::string replace = std::to_string(uid);
    fd_strreplace(&sql, &search, &replace);
    fd_keymap * result = this->sqlSelectRow(&sql);
    return fd_keymaptojson(result);
}
std::string FlacDemon::Database::getAll(){
    std::string sql = sql_statements.getAll;
    fd_keymap_vector * results = this->sqlSelect(&sql);
    return fd_keymap_vectortojson(results);
}

void FlacDemon::Database::fillDatabase(int entries){
    std::string sql = "insert into tracks (track,title,albumartist,artist,album,genre,composer,disc,tracktime,playcount,dateadded,filepath,albumuuid,verified) values('3','Loin Des Villes','Yann Tiersen','Yann Tiersen','Les Retrouvailles','Other','-1','0','199866','0','1449148992','/mnt/Backup/Storage/FLACS/Yann Tiersen/Les Retrouvailles/03 Yann Tiersen - Loin Des Villes.flac','729489FE-9124-49B2-927C-8D1E246F2D6C','1')";
    for(int i = 0; i < entries; i++){
        this->runSQL(&sql);
    }
}

