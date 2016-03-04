//
//  FlacDemonUtils.h
//  FlacDemon
//
//  Created by merryclarke on 24/11/2015.
//  Copyright (c) 2015 c4software. All rights reserved.
//

#ifndef __FlacDemon__FlacDemonUtils__
#define __FlacDemon__FlacDemonUtils__

#include "includes.h"
#include "typedefs.h"
#include "netincludes.h"
#include "globals.h"

extern std::vector< std::string > * fd_numbers;

void initGlobals();
int fd_stringtoint(std::string * str, int * value);
float fd_comparetags(std::string * tag1, std::string * tag2);
const void * characterAtIndex(const void * str, int index, void * context);
int compareCharacters(const void * c1, const void * c2, void * context);

void fd_tolowercase(std::string * str);
int fd_strreplace(std::string * str, std::string * search, std::string * replace , bool global = false );
int fd_strreplace(std::string * str, const char * search, const char * replace , bool global = false );
int fd_strnumbercompare(std::string * str1, std::string * str2);

std::string fd_sqlescape(std::string isql);

std::string fd_keymap_vectortojson(fd_keymap_vector* ikeymap_vector);
std::string fd_keymaptojson(fd_keymap * ikeymap);
fd_keymap_vector * fd_jsontokeymap_vector(std::string * json);
std::vector< std::string > fd_splitjsondescriptor(std::string * json);
json_t * fd_decodeJSON( std::string & jsonString );
void waitfor0(int * value);
void waitfor0(bool * value);

int isMainThread();

std::string fd_standardiseKey(std::string * key);
std::string fd_secondstoformattime(int seconds);

std::vector < std::string > fd_splitstring(std::string & str, const char * delim);
std::vector < std::string > fd_splitstring(std::string & str, std::string delim);
fd_keymap fd_parsekeyvaluepairs( std::string & str , const char * pairSeparator , const char * keyValueSeparator );
bool searchPredicate(char a, char b);

#endif /* defined(__FlacDemon__FlacDemonUtils__) */
