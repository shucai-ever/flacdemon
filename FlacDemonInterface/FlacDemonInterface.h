//
//  FlacDemonInterface.h
//  FlacDemon
//
//  Created by merryclarke on 02/01/2016.
//  Copyright (c) 2016 c4software. All rights reserved.
//

#ifndef __FlacDemon__FlacDemonInterface__
#define __FlacDemon__FlacDemonInterface__

#include "includes.h"

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "ncurses.h"
#include "panel.h"
#include <condition_variable>

#include "FlacDemonUtils.h"
#include "TrackListing.h"
#include "Library.h"
#include "CommandParser.h"
#include "LibraryTitles.h"


#define fd_interface_printlibrary   1 << 0
#define fd_interface_libraryupdate  1 << 1
#define fd_interface_printcommand   1 << 2
#define fd_interface_playing        1 << 3
#define fd_interface_printplaying   1 << 4
#define fd_interface_playbackprogress 1 << 5
#define fd_interface_printprogress  1 << 6

using std::cout;
using std::endl;

class FlacDemonInterface{
private:
    int socketFileDescriptor;
    std::thread * readThread;
    std::thread * retryConnectThread;
//    std::mutex socketMutex;
    std::mutex eventMutex;    
    std::condition_variable eventCV;
    
    WINDOW * browserWindow;
    WINDOW * browserHeaderWindow;
    WINDOW * commandWindow;
    WINDOW * playbackWindow;
    
    WINDOW * verifyWindow;
    WINDOW * albumViewWindow;
    
    PANEL * browserPanel;
    PANEL * verifyPanel;
    
    FlacDemon::Library library;
    
    unsigned long flags;
    
    int maxColumns;
    int maxRows;
    int browserRows;
    int currentBrowserRow;
    size_t browserOffset;
    std::map < std::string , size_t > columnWidths;
    
    bool printAlbums;
    bool disableColors;
    bool colorsOn;
    
    size_t commandCursorPosition;
    size_t commandCursorDefault;
    
    FlacDemon::CommandParser commandParser;
    std::string commandPrompt;
    std::string searchPrompt;
    std::string userCommand;
    
    bool fetchedLibrary;
    bool killResponseThread;
    bool isSearch;
    bool typeSearch;
    
    
    FlacDemon::TrackListing * nowPlaying;
    float progress;
    
protected:
    
public:
    FlacDemonInterface();
    ~FlacDemonInterface();
    void initialize();
    WINDOW * nextwin( size_t rowSize , size_t * row = nullptr , int reset = -1);
    void connect();
    void retryConnect();
    void onConnect();
    void run();
    void printFlags();
    void event(unsigned long rlflags = 0);
    void setRunLoopFlags(unsigned long flags);
    void userInputLoop();
    void trySearch();
    void clearSearch();
    void escapeHandler();
    fd_stringvector& libraryTitles();
    void printLibrary(int offset);
    void printLibraryHeaders();
    void printLibraryLine( WINDOW * window , std::vector< std:: string > * values);
    void callCommand( const char * signal, void * args );
    void sendCommand(const char * command);
    void readResponse();
    void parseResponse(std::string response);
    std::string parseCommandFromResponse(std::string * response);
    std::string removeCommandFromResponse(std::string * response);
    const char * formatValue(std::string value, int max);
    void parseLibraryUpdate(std::string * response);
    void changeOffset(int diff, bool absolute = false);
    void setColumnWidths();
    void setNowPlaying(std::string ID);
    void printNowPlaying();
    void printCommand();
    void setCommandCursor();
    void printProgress();
    void waitForSearch();
    void setColor( WINDOW * window , int attr , bool onoff );
    void printAlbum( WINDOW * window , FlacDemon::Album * album );
    
};

#endif /* defined(__FlacDemon__FlacDemonInterface__) */
