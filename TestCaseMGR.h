
#ifndef _TESTCASEMGR_h
#define _TESTCASEMGR_h
#include <systemc.h>
#include "svdpi.h" 
#include "tlm.h" 
#include "VsiEthGateway.h"
#include <sstream>
#include "ConvertVpiSimTime.h"

typedef struct 
{
	uint32_t id;
	float data[2];
} DataPacket;


// ---------------------------------------------------------------------------
// Class TestCaseMGR : Inner Level Class
// ---------------------------------------------------------------------------
class TestCaseMGR : public sc_module {

	public:
		VsiEthGateway * ethGateway;
		enum { QUANTUM_RATIO = 1 };
		DataPacket receivedUDP;
		DataPacket transUDP;
		bool initialize;
		bool bInfusionRecieved;
		bool bBIS;

	private:
		sc_event dGotRtosInterruptEvent;
		sc_event dGotBISEvent;
		SC_HAS_PROCESS(TestCaseMGR);

	public:
		TestCaseMGR(sc_module_name name );

		~TestCaseMGR();

	bool getTestCase(float* Data,uint32_t currentCase);

        void rtosInterrupt() 
        { 
            dGotRtosInterruptEvent.notify(); 
        }

	void BISInput() 
        { 
            dGotBISEvent.notify(); 

        }

	uint32_t stoi(string s);
	string to_string(uint32_t number);
	

		// Callback function that's called upon receiving an ethernet frame
		void handleReceivedPacket( unsigned char *payload, unsigned numBytes );

		// Main thread for processing and sending ethernet frames
		void mainThread();
};
#endif
