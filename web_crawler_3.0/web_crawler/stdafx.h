/*
Name: Ismael Rodriguez
Class: CSCE 463-500
Semester: Fall 2019
*/
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//memory leakage debugging

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <Windows.h>

#include "HTMLParserBase.h"


// TODO: reference additional headers your program requires here
#include <iostream>
#include <string.h>

#include "URLParser.h"
#include "WebCrawler.h"

#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#include <chrono>
#include <fstream>
#include <unordered_set>
#include <string>
#include <vector>
#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)
using namespace std;