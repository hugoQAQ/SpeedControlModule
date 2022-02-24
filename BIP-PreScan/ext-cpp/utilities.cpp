#include "EthGateway.h"
#include "VideoFromFabricGateway.h"
#include"utilities.hpp"
#include <string> // for string class

using namespace XlTlm;

using namespace EmuTlmApi;
using namespace XlEtherPacketSnooper;


float deepLearn()
{
	stringstream tmp;
	tmp<<"python ./ext-cpp/Inference.py ./ext-cpp/yolox_s.onnx  ./Signs/merged_image.jpg > system.txt";
	// tmp<<"python ./ext-cpp/Inference.py ./ext-cpp/yolox_s.onnx > system.txt";
	string execCmd = tmp.str();
	int a = system(execCmd.c_str());
	FILE *fp;
	float buff;

	fp = fopen("system.txt", "r");
	int result = fscanf(fp, "%f", &buff);
	fclose(fp);

	if(result == EOF)
		return -1;

	return buff;

}

//extern "C" {

// all those varaiables need to be wrapped in some function (they need to be global vars ) (achievable in BIP)
static EthGateway *ethGateway = NULL;
static EmuTlmApiPthreaded *dSession = NULL;
static VideoFromFabricGateway *videoGateway = NULL;

enum { QUANTUM_RATIO = 1 };

const unsigned SOURCE_PORT_ID = 1;
const unsigned  DESTINATION_PORT_ID = 0;

const unsigned  IMAGE_WIDTH = 800;
const unsigned  IMAGE_HEIGHT = 600;
const unsigned  IMAGE_FULL_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT;

static unsigned char macAddresses[][ETH_ADDR_NUM_BYTES] = {
    { 0x33, 0x33, 0x00, 0x00, 0x00, 0x10},   // Port 0
    { 0x33, 0x33, 0x00, 0x00, 0x00, 0x11} }; // Port 1

static unsigned char ipAddresses[][IPV4_ADDR_NUM_BYTES] = {
    {  192, 168, 1, 10 },   // Port 0
    { 192, 168, 1, 11 } }; // Port 1

// Set the RTOS timeout for .1 seconds, expressed in NS, and adjusted by
// the ~QUANTUM_RATIO~ we're using in this simulation.
const unsigned long long RTOS_TIMEOUT_IN_NS
    = .1 * /*ns-per-sec=*/1e9 / QUANTUM_RATIO;

static bool gotRxEthernetPacket = false;
static double vehicleSpeed = 0.0;
static unsigned rxPayloadNumBytes = 0;

static bool gotRxVideoPacket = false;
static unsigned char rxVideoPayload[IMAGE_FULL_SIZE * 4 /*RGBA*/] = {0};
static unsigned rxVideoPayloadNumBytes = 0;

//----------------------------------------------------------------------------
// Topic: Forward transport for RX frames wrapper function
//
// This interrupt handler function is registered directly with the
// interrupt target channel (see below). It, in turn, downcasts the ~context~
// to its owning *class FrontBackTester* object, then calls its
// *::nbTransportFw()* method to do the actual handling of the interrupt.
//----------------------------------------------------------------------------


static void rxEthFrameHandler(
    unsigned char *payload, unsigned numBytes, void *context )
{
    typedef union {
       uint8_t rawData[sizeof(double)];
       double floatRep;
    } PayloadCaster;

    gotRxEthernetPacket = true;
    if (numBytes != sizeof(double))
    {
        cerr << "Error. Expected Ethernet data size: "<< sizeof(double)<<" bytes. Received is: " << numBytes<< " bytes." << endl;
    }
    else
    {
        PayloadCaster *caster = (PayloadCaster *)payload;
        vehicleSpeed = caster->floatRep;
        cout << "Received new speed value from Prescan. Speed: "<< vehicleSpeed<< " m/s." << endl;
    }

    rxPayloadNumBytes = numBytes;
}

static void rxVideoFrameHandler(
    unsigned char *payload, unsigned numBytes, void *context )
{
    gotRxVideoPacket = true;
    if (numBytes != (IMAGE_FULL_SIZE * 4 /*RGBA*/))
    {
        cerr << "Error. Expected video size is: " << (IMAGE_FULL_SIZE * 4) << "bytes. Received is: " << numBytes <<" bytes." << endl;
    }
    else
    {
        cout << "Received new video frame from Prescan" << endl;
        memcpy (rxVideoPayload, payload, numBytes);
    }
    rxVideoPayloadNumBytes = numBytes;

}

void configureVideo()
{
    // Video gateway configuration occurs only in the first iteration
    videoGateway->configure();
}
    
int initializeConnection(const char* serverUrl, unsigned int domain, unsigned int portNum)
{
    // First establish connection remote TLM fabric server ...
    dSession = new EmuTlmApiPthreaded( serverUrl, domain, portNum,
        "remoteSession2" , ":timeServerConduit1", ":resetServerConduit1" );

    ethGateway = new EthGateway( dSession,
        /*txConduitId=*/ ":txEtherFrameConduit01",
        /*rxConduitId=*/ ":rxEtherFrameConduit01",
        /*myMacAddress=*/   macAddresses[SOURCE_PORT_ID],
        /*myIpAddress=*/    ipAddresses[SOURCE_PORT_ID]);

    ethGateway->registerRxCallback( NULL, rxEthFrameHandler );
	
    videoGateway = new VideoFromFabricGateway( dSession, 
        /*rxConduitId=*/ ":videoStream1",
        /*txConduitId=*/ ":videoConfig1",
        /*imageWidth=*/ IMAGE_WIDTH,
        /*imageHeight=*/ IMAGE_HEIGHT );


	videoGateway->registerRxCallback( NULL, rxVideoFrameHandler );

    cout << "+=+ INFO: ... successfully connected to TLM FabricServer !" << endl;
    return 0;
}

// API functions for BIP 

int advanceSimulation(){
	int advanceSuccessful = 1;
    try
    {
        dSession->advanceNs( RTOS_TIMEOUT_IN_NS );
    }
    catch (...)
    {
        advanceSuccessful = 0;
    }

    return advanceSuccessful;
}


void print_out(float x){
cout<<x<<endl;
}


int waitForReset(){
	int resetSuccessful = 1;
		try { dSession->waitForReset(); }
		catch (...) {
			resetSuccessful = 0;
		}
		return resetSuccessful;
}


int connectToServer() {
    char* serverIp = "137.202.65.169 ";  //'localhost'
	
    char* domainStr = "AF_UNIX";
    int   port=50102;
    unsigned int domain;

    if (string(domainStr) == "AF_INET")
        domain = AF_INET;
    else
        domain = AF_UNIX;
    return initializeConnection(serverIp, domain, port);
}

double recvEthernetPacket() 
{
    // Block execution until RX packet received ...
    while( gotRxEthernetPacket == false ) dSession->yield();
    gotRxEthernetPacket = false;

	return vehicleSpeed;
}

void recvVideoPacket()
{
    while (!gotRxVideoPacket) dSession->yield();
    gotRxVideoPacket = false;
    
    // payload is already in BGRA format, which is the same format cv is expecting, 
    // so we construct the image directly.
    cv::Mat image(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC4, rxVideoPayload);
    cv::imwrite("Signs/merged_image.jpg", image);
}

int sendEthernetPacket(float dataToSend) {  
    //unsigned char* data = (unsigned char*) dataToSend.c_str(); // cast from string to unsigned char*
    double data = (double) dataToSend;
    int dataSize = sizeof(data);
    //int val = stoi(dataToSend) ; convert from string to int

     union {
        uint8_t rawData[8];
        double floatRep;
    } caster;

	caster.floatRep = data;
    

    ethGateway->sendEthernetPacket( caster.rawData, sizeof(caster),
        macAddresses[DESTINATION_PORT_ID], ipAddresses[DESTINATION_PORT_ID]);
    printf("Send Data \n");
    return 1;
}

void disconnectFromServer(void) {

    // Ship out a special Ethernet frame telling the Prescan client to exit
    union {
        uint8_t rawData[16];
        uint64_t uintRep[2];
    } caster;

    caster.uintRep[0] = 0xffffffffffffffff;
    caster.uintRep[1] = 0xffffffffffffffff;

    ethGateway->sendEthernetPacket( caster.rawData, 2*sizeof(uint64_t),
        macAddresses[DESTINATION_PORT_ID], ipAddresses[DESTINATION_PORT_ID]);

    delete ethGateway;
    delete dSession;

    cout << "Disconnected from TLM FabricServer ..." << endl;;
}




