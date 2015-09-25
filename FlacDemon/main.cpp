//
//  main.cpp
//  FlacDemon
//
//  Created by merryclarke on 15/06/2015.
//  Copyright (c) 2015 c4software. All rights reserved.
//

#include <iostream>
#include "FlacDemonNameSpace.h"
#include "FlacDemon.h"

const SignalHandler * signalHandler = new SignalHandler();

int main(int argc, const char * argv[])
{

    // insert code here...
    std::cout << "Hello, World!\n";
    
    chdir("/Users/merryclarke/Documents/Xcode Projects/FlacDemon/");
    
    FlacDemon::Demon * demon = new FlacDemon::Demon();
    
    demon->run();

    delete demon;
    delete signalHandler;
    
    
    return 0;
}

