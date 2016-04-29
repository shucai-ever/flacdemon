/***********************************************************************
 * main.cpp : daemon main()
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

#include <iostream>
#include "FlacDemonAll.h"
#include "FlacDemon.h"

#define FDOPTIONS_USE_FLACS_DIR 0

#include "globals.hpp"

FlacDemon::Demon * demon = nullptr;

int main(int argc, const char * argv[])
{

    // insert code here...
    std::cout << "Hello, World!\n";
    
    initGlobals();
    
    demon = new FlacDemon::Demon();
    
    
    demon->run();

    delete demon;
    delete signalHandler;
    
    curl_global_cleanup();
    
    return 0;
}
