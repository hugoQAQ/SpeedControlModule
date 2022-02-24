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

// Open CV dependencies
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace std;

float deepLearn();
void print_out(float x);
int sendEthernetPacket(float dataToSend);
double recvEthernetPacket();
void recvVideoPacket();
int connectToServer();
void disconnectFromServer();
int waitForReset();
void configureVideo();
int advanceSimulation();

#endif
