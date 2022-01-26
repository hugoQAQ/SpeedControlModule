#ifndef _Utilities
#define _Utilities

#include <stdio.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <limits> 
#include <fstream>
#include <cstring>
#include <unistd.h>

#define GET_FILE_NUM_CMD "ls Signs/*.png | wc -l"
using namespace std;

int const_getFileNum();
int deepLearn(int i);
int get_directory();
int input();
#endif
