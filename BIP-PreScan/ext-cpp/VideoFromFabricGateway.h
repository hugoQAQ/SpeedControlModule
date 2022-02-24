//----------------------------------------------------------------------------//
// Unpublished work. Copyright 2021 Siemens                                   //
//                                                                            //
// This material contains trade secrets or otherwise confidential             //
// information owned by Siemens Industry Software Inc. or its affiliates      //
// (collectively, "SISW"), or its licensors. Access to and use of this        //
// information is strictly limited as set forth in the Customer's applicable  //
// agreements with SISW.                                                      //
//----------------------------------------------------------------------------//

#ifndef _VideoFromFabricGateway_h
#define _VideoFromFabricGateway_h

#include "svdpi.h"
#include "tlm.h"

#include "EmuTlmApi.h"
#include "EmuTlmApiPthreaded.h"
#include "ConvertVpiSimTime.h"

#include "VideoStreamConfig.h"

typedef void (*VideoFromFabricGatewayRxCallbackType)(
    unsigned char *payload, unsigned numBytes, void *context );

//______________________________                                ______________
// class VideoFromFabricGateway \______________________________/ johnS 6-11-21
//----------------------------------------------------------------------------

class VideoFromFabricGateway {

  public:
    // TLM sockets for Ethernet interface, to be connected to TLM Ethernet switch
    EmuTlmApi::EmuTlmPtTargetSocket *rxSocket;
    EmuTlmApi::EmuTlmPtInitiatorSocket *configSocket;

    // This object is for receiving an initial static configuration on the TLM
    // channel for video streaming.
    XlVip::VideoStreamConfig dConfig;

  private:
    VideoFromFabricGatewayRxCallbackType dRecvVideoFrame;
    void *dRecvVideoFrameContext;

  public:
    //---------------------------------------------------------
    // Constructors/Destructors

    VideoFromFabricGateway(
        EmuTlmApi::EmuTlmApiPthreaded *session,
        const char *rxConduitId,
        const char *txConduitId,
        unsigned imageWidth, unsigned imageHeight );
    ~VideoFromFabricGateway();

    // -------------------------------------------------------------
    // Topic: TLM-2.0 target required transport virtual functions
    // -------------------------------------------------------------
    //
    // | ::nb_transport_***()
    //
    // This is the TLM-2.0 compliant ~transport~ function required to be
    // registered with TLM-2.0 target socket, ~rxEtherSocket~.
    //
    // It is the callback which processes RX frames being broadcast to the
    // TLM bus.
    //---------------------------------------------------------

    // (begin inline source)
    tlm::tlm_sync_enum nb_transport_fw_rx_video(
        tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase );
    // (end inline source)

  public:
    void registerRxCallback( void *context, VideoFromFabricGatewayRxCallbackType callback ){
        dRecvVideoFrameContext = context;
        dRecvVideoFrame = callback;
    }
    void configure();

    //---------------------------------------------------------
    // Accessors
    //---------------------------------------------------------
    unsigned getImageWidth() { return dConfig.getImageWidth(); }
    unsigned getImageHeight() { return dConfig.getImageHeight(); }

    //---------------------------------------------------------
    // Error Handlers
    //---------------------------------------------------------

    void errorOnTransport( const char *functionName,
        int line, const char *file )
    {   char messageBuffer[1024];
        sprintf( messageBuffer,
            "%s: Error on transport socket [line #%d of '%s'].\n",
            functionName, line, file );
        SC_REPORT_FATAL( "VIDEO-FATAL", messageBuffer ); }

    void errorOnTransportPhase( const char *functionName,
        tlm::tlm_phase expectedPhase,
        tlm::tlm_phase actualPhase,
        int line, const char *file )
    {   char messageBuffer[1024];
        const char *expectedPhaseStr;
        const char *actualPhaseStr;
        switch( expectedPhase ){
            case tlm::UNINITIALIZED_PHASE:
                expectedPhaseStr = "tlm::UNINITIALIZED_PHASE"; break;
            case tlm::BEGIN_REQ:  expectedPhaseStr = "tlm::BEGIN_REQ";  break;
            case tlm::END_REQ:    expectedPhaseStr = "tlm::END_REQ";    break;
            case tlm::BEGIN_RESP: expectedPhaseStr = "tlm::BEGIN_RESP"; break;
            case tlm::END_RESP:   expectedPhaseStr = "tlm::END_RESP";   break;
            default: expectedPhaseStr = "tlm::UNINITIALIZED_PHASE"; break;
        }
        switch( actualPhase ){
            case tlm::UNINITIALIZED_PHASE:
                actualPhaseStr = "tlm::UNINITIALIZED_PHASE"; break;
            case tlm::BEGIN_REQ:  actualPhaseStr = "tlm::BEGIN_REQ";  break;
            case tlm::END_REQ:    actualPhaseStr = "tlm::END_REQ";    break;
            case tlm::BEGIN_RESP: actualPhaseStr = "tlm::BEGIN_RESP"; break;
            case tlm::END_RESP:   actualPhaseStr = "tlm::END_RESP";   break;
            default: actualPhaseStr = "tlm::UNINITIALIZED_PHASE"; break;
        }
        sprintf( messageBuffer,
            "%s: phase error on transport socket "
            "expectedPhase=%s actualPhase=%s [line #%d of '%s'].\n",
            functionName,
            expectedPhaseStr, actualPhaseStr, line, file );
        SC_REPORT_FATAL( "VIDEO-FATAL", messageBuffer ); }

    void errorStaticConfigsOnly( const char *functionName,
        int line, const char *file ) const
    {   char messageBuffer[1024];
        sprintf( messageBuffer,
            "Error in %s: Only static configuration ops (config, query) are "
            "allowed on the ::nb_transport_fw() calls of the 'configSocket' "
            "port of the VIDEO slave driver [line #%d of '%s'].\n",
            functionName, line, file );
        SC_REPORT_FATAL( "VIDEO-FATAL", messageBuffer ); }

    void errorUnexpectedRxPacket( const char *functionName )
    {   char messageBuffer[1024];
        sprintf( messageBuffer,
            "%s: Received an unexpected RX packet at a time when the "
            "'expected RX packet' queue was empty.",
            functionName );
        SC_REPORT_FATAL( "VIDEO-FATAL", messageBuffer ); }
};
#endif // _VideoFromFabricGateway_h
