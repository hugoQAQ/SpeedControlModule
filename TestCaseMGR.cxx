//----------------------------------------------------------------------------//
// Unpublished work. Copyright 2022 Siemens                                   //
//                                                                            //
// This material contains trade secrets or otherwise confidential             //
// information owned by Siemens Industry Software Inc. or its affiliates      //
// (collectively, "SISW"), or its licensors. Access to and use of this        //
// information is strictly limited as set forth in the Customer's applicable  //
// agreements with SISW.                                                      //
//----------------------------------------------------------------------------//


#include "TestCaseMGR.h"

#define NUM_CLIENTS 3
#define SRC_INDEX 0

static unsigned char macAddresses[][XlEtherPacketSnooper::ETH_ADDR_NUM_BYTES] = {
	 {0x12, 0x34, 0x56, 0x78, 0x9a,0xbc},
	 {0x12, 0x34, 0x56, 0x78, 0x9a,0xbd},
	 {0x12, 0x34, 0x56, 0x78, 0x9a,0xbe}};

static unsigned char ipAddresses[][XlEtherPacketSnooper::IPV4_ADDR_NUM_BYTES] = {
	 {192, 168, 1,10},
	 {192, 168, 1,11},
	 {192, 168, 1,12}};

float receivedBIS = 0 ; // Global variable to receive the BISout from patient fmu
float receivedOASS = 0;
float newInfusionRate = 0 ; // Global variable to receive the new infusion from BIP model 
// ---------------------------------------------------------------------------
// Method: rxFrameHandler
// This is a callback method which will be called upon receiving new packets.
// it casts the context to the TestCaseMGR object that will handle
// the incoming packet
//----------------------------------------------------------------------------
static void rxFrameHandler (
    unsigned char *payload, unsigned numBytes, void *context )
{
	TestCaseMGR *me
		= reinterpret_cast<TestCaseMGR*>(context);
	return me->handleReceivedPacket( payload, numBytes );
}

// ---------------------------------------------------------------------------
// Method:: TestCaseMGR() constructor
//
// The constructor contains the setup of the Gateway object which will 
// be used in communication with the server, it also contains the
// registration of the appropriate transport method.
//---------------------------------------------------------------------------
TestCaseMGR::TestCaseMGR (sc_module_name name )
    : sc_module( name ),initialize(true), bInfusionRecieved(false), bBIS(false)
{
	transUDP.id = 1;
	transUDP.data[0] = 0;
	transUDP.data[1] = 0;

	ethGateway = new VsiEthGateway( "ethGateway",
		/*txConduitId =*/  ":txEtherFrameConduit0",
		/*rxConduitId =*/  ":rxEtherFrameConduit0",
		/*myMacAddress=*/  macAddresses[SRC_INDEX] ,
		/*myIpAddress=*/  ipAddresses[SRC_INDEX] );

	ethGateway->registerRxCallback( this, rxFrameHandler );
	SC_THREAD( mainThread );
}

// ---------------------------------------------------------------------------
// Destructor 
// ---------------------------------------------------------------------------
TestCaseMGR::~TestCaseMGR() 
{
	delete ethGateway;
}


string TestCaseMGR::to_string(uint32_t number)
{

	std::ostringstream ostr; //output string stream
	ostr << number; //use the string stream just like cout,
	//except the stream prints not to stdout but to a string.
	return ostr.str(); //the str() function of the stream 
}

// ---------------------------------------------------------------------------
// TestCaseMGR::stoi to convert string into int
// ---------------------------------------------------------------------------
uint32_t TestCaseMGR::stoi(string s)
{
	uint32_t i;
	i = 0;
	uint32_t count = 0;
	while(s[count] >= '0' && s[count] <= '9')
	{
		i = i * 10 + (s[count] - '0');
		count++;
	}
	return i;
}
// ---------------------------------------------------------------------------
// TestCaseMGR::getTestCase to fetch test case if exists 
// ---------------------------------------------------------------------------
bool TestCaseMGR::getTestCase(float* Data,uint32_t currentCase)
{
	ifstream myfile ("./TestCases.txt");
	if(!myfile.is_open())
	{	
		cout << "Error opening TestCases.txt\n";
		return 0;
	}

	string line;
	uint32_t testCaseNo = 0;
	do
	{
		std::getline(myfile, line);
		if (line=="")
		{
			cout<<"TEST CASE "<<currentCase<<" NOT FOUND! We have reached the end of file"<<"\n";
			return 0;
		}

		size_t pos = line.find("[");
		if (pos != string::npos)
		{
			size_t pos2 = line.find("]");
			testCaseNo = stoi( line.substr(pos+1,(pos2-pos+1)));
		}

	}while ( testCaseNo != currentCase);
	std::getline(myfile, line);
	cout<<"FOUND TEST CASE NO."<<currentCase<<endl;
	for(int i= 0; i<10;i++)
	{
		size_t pos = line.find(":");
		size_t pos2 = line.find(",");
		string temp =line.substr(pos+1,pos2-pos-1);
		Data[i] = atof(temp.c_str()); // convert the number from string to float

		line = line.substr(pos2+1); // advance the index we start searching from

	}
	Data[10] = 0;
	return true;
}

//----------------------------------------------------------------------------
// Method:: mainThread()
// The main thread is called every simulation step (RTOS interrupt),
// it is where all the needed operations are executed 
//----------------------------------------------------------------------------
void TestCaseMGR::mainThread()
{
	float vPatientData[11];
	int numBytesPatient = sizeof(vPatientData) / sizeof(char); // Patient Client 2
	int numBytesDataPacket = sizeof(transUDP) / sizeof(char);
	uint32_t currentCase = 1; // test case no. we start with

	bool available = getTestCase(vPatientData,currentCase); // fetch a test case to run
	
	if(!available)
	{	
		cout<<"No more available test cases.."<<endl;
		return ;
	}
	receivedBIS = vPatientData[1]; //initial value of BIS
	transUDP.data[0] = vPatientData[8]; // initial infusion rate
	newInfusionRate = vPatientData[8];
	
	while(1) {
		

		wait(dGotRtosInterruptEvent);
		if (initialize)
		{
			initialize = false;
			ethGateway -> sendEthernetPacket((unsigned char*)vPatientData,numBytesPatient, macAddresses[2], 
				ipAddresses[2]);
			ethGateway -> sendEthernetPacket((unsigned char*)&transUDP,numBytesDataPacket, macAddresses[1], 
				ipAddresses[1]);
			vPatientData[10] = 1;
			continue;

		}
		

		while(!bBIS);
		bBIS=false;

		cout<<"Inserting stats into the report..."<<endl;

		// open a report file for the testcase we are currently use
		string path = "./testReports/testCase"+to_string(currentCase)+".txt";
		cout<<path<<endl;
		std::ofstream myfile;
		myfile.open (path.c_str(),std::ios_base::app);
		myfile 	<< "Age:"<<vPatientData[0]<<" - BISo:"<<vPatientData[1]<<" - BISmin:"<<vPatientData[2]<<" - Drug Conc:"<<vPatientData[3]<<" - E50:"<<vPatientData[4]
			<<" - Gamma:"<<vPatientData[5]<<" - Gender:"<<vPatientData[6]<<" - Height:"<<vPatientData[7]<<" - Infusion rate:"<<vPatientData[8]
			<<" - Weight:"<<vPatientData[9]<<" - BISout:"<<receivedBIS<<" - OASS:"<<receivedOASS<<" - Time[ns]: "<< convert.timeInNs() <<" \n";
		myfile.close();

		cout <<"OASS value is: "<<receivedOASS<<endl;
		if  (receivedOASS >= 5)
		{


			receivedOASS = 0;
			currentCase++; 

			bool available = getTestCase(vPatientData,currentCase); // fetch a test case to run

			if(!available)
			{

				transUDP.id = 3 ;
				ethGateway -> sendEthernetPacket((unsigned char*)&transUDP,numBytesDataPacket, macAddresses[1], 
				ipAddresses[1]);

				return ;
			}
			receivedBIS = vPatientData[1];
			transUDP.data[0] = vPatientData[8];
			newInfusionRate = vPatientData[8]; 

			ethGateway -> sendEthernetPacket((unsigned char*)vPatientData,numBytesPatient, macAddresses[2], 
				ipAddresses[2]);
			ethGateway -> sendEthernetPacket((unsigned char*)&transUDP,numBytesDataPacket, macAddresses[1], 
				ipAddresses[1]);
			vPatientData[10] = 1;
			continue;

		}	
	

		vPatientData[8] = newInfusionRate;		
		cout<<"Send new infusion rate from BIP: "<<newInfusionRate<<endl;	
		// To send any packet to any client use sendEtherentPacket (vPatientData, number of bytes, 
		// destination MAC address, destination IP address)
		ethGateway -> sendEthernetPacket((unsigned char*)vPatientData,numBytesPatient, macAddresses[2], 
		ipAddresses[2]);


	}
}

// ---------------------------------------------------------------------------
// Method:: handleReceivedPacket()
// This method does the actual handling of the received packets from other 
// clients.
//----------------------------------------------------------------------------
void TestCaseMGR::handleReceivedPacket(unsigned char *payload, unsigned numBytes)
{
	//-------------------------------------------------------------------------
	// This is where we receive data packets from other clients
	// The user can manipulate the received data here; read it, store its value,
	// or even fire some event based on the data received.
	// The following is an example of reading and printing the received packet:
	//-------------------------------------------------------------------------

	memcpy((unsigned char*)&receivedUDP,payload,sizeof(receivedUDP)/sizeof(char));
	
	if ( receivedUDP.id == 0 )
	{
		receivedBIS = receivedUDP.data[0];
		cout<<"Received BIS from Patient: "<<receivedBIS<<endl;	
		bBIS = true;
	}
	else if(receivedUDP.id == 1)
	{	bInfusionRecieved = true;
		newInfusionRate = receivedUDP.data[0];
		receivedOASS =receivedUDP.data[1];
		cout<<"Received OASS value: "<<receivedOASS<<endl;	
		
	}	
}
