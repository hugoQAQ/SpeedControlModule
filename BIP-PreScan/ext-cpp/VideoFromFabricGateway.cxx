//----------------------------------------------------------------------------//
// Unpublished work. Copyright 2021 Siemens                                   //
//                                                                            //
// This material contains trade secrets or otherwise confidential             //
// information owned by Siemens Industry Software Inc. or its affiliates      //
// (collectively, "SISW"), or its licensors. Access to and use of this        //
// information is strictly limited as set forth in the Customer's applicable  //
// agreements with SISW.                                                      //
//----------------------------------------------------------------------------//

#include "VideoFromFabricGateway.h"

using namespace tlm;
using namespace XlVip;
using namespace EmuTlmApi;

//----------------------------------------------------------------------------
// Topic: Forward transport for RX frames wrapper function
//
// This RX video frame handler function is registered directly with the
// RX target channel (see below). It, in turn, downcasts the ~context~
// to its owning *class VideoFromFabricGateway* object, then calls its
// *::nb_transport_fw_rx_video()* method to do the actual handling of the
// video frame. It can also handle static configuration updates.
//----------------------------------------------------------------------------

// (begin inline source)

static tlm_sync_enum rxFrameHandler(
    tlm_generic_payload &trans, // Input/output: TLM GP transaction
    tlm_phase &phase,           // Input/output: Transaction phase.
    void *context )             // Input: Context pointer for callback function.
{
    VideoFromFabricGateway *me =
        reinterpret_cast<VideoFromFabricGateway *>( context );
    return me->nb_transport_fw_rx_video( trans, phase );
}
// (end inline source)

//---------------------------------------------------------
//  Constructor/Destructor()
//---------------------------------------------------------

VideoFromFabricGateway::VideoFromFabricGateway(
    EmuTlmApiPthreaded *session,
    const char *rxConduitId,
    const char *txConduitId,
    unsigned imageWidth, unsigned imageHeight )
{
    // Connect the RX direction ...
    rxSocket = new EmuTlmPtTargetSocket( session );
    rxSocket->connect( rxConduitId );

    // Register the target socket callback function for the ethernet
    // RX socket interface
    rxSocket->registerNbTransportFwCallback( this, rxFrameHandler );

    // Connect the TX (config) direction ...
    configSocket = new EmuTlmPtInitiatorSocket( session );
    configSocket->connect( txConduitId );
    
    // Set initial configuration
    dConfig.setDefaults();
    dConfig.setImageWidth( imageWidth );
    dConfig.setImageHeight( imageHeight );
    dConfig.setPixelFormat( VideoStreamConfig::BGRA32 );

}

VideoFromFabricGateway::~VideoFromFabricGateway(){
}

//---------------------------------------------------------
// Method: ::configure()
//
// This method send the video stream configuration to the XlTlmVideoToEthGateway
// module in form of a TLM packet.
// 
//---------------------------------------------------------
    
void VideoFromFabricGateway::configure() {

    // Let's send the initial static configuration to our TLM-2.0 target for
    // the video stream channel ...
    tlm_phase phase = BEGIN_REQ;
    tlm_sync_enum tlmSyncStatus =
        configSocket->nbTransportFw( dConfig.update_trans(), phase );
    if( tlmSyncStatus != TLM_COMPLETED
        || dConfig.update_trans().get_response_status()
            != TLM_OK_RESPONSE )
        errorOnTransport( "::VideoFromFabricGateway()", __LINE__, __FILE__ );
}   

//---------------------------------------------------------
// Method: ::nb_transport_fw_rx_video()
//
// This is the standard TLM transport callback function for the RX target
// socket.
//---------------------------------------------------------

// (begin inline source)
tlm_sync_enum VideoFromFabricGateway::nb_transport_fw_rx_video(
    tlm_generic_payload &trans, tlm_phase &phase )
{
    tlm_sync_enum ret = TLM_COMPLETED;

    // Innocent until proven guilty.
    trans.set_response_status( TLM_OK_RESPONSE );

    if( phase != BEGIN_REQ ){
        errorOnTransportPhase(
            "VideoFromFabricGateway::nb_transport_fw_rx_video()",
            BEGIN_REQ, phase, __LINE__, __FILE__ );
        trans.set_response_status( TLM_GENERIC_ERROR_RESPONSE );
    }

    //---------------------------------------------------------
    // Topic: Handling of the "static configuration" extension
    //
    // If the *::transportFw()* function sees that a TLM configuration
    // extension, it means the intiator is sending a *class VideoStreamConfig*
    // object intended for *class VideoReducer* configuration.

    // (begin inline source)

    VideoStreamConfig *configExtension;
    trans.get_extension( configExtension );

    if( configExtension != NULL && trans.get_data_length() != 0 ) {
        errorStaticConfigsOnly(
            "VideoReducer::nb_transport_fw()", __LINE__, __FILE__ );
        trans.set_response_status( tlm::TLM_COMMAND_ERROR_RESPONSE );
    }

    if( configExtension != NULL ){

        // if( this is a "query" op )
        //     We're just querying the static configuration. We can just
        //     transfer it from the local config object.
        if( trans.is_read() )
            configExtension->copy_from( dConfig );

        // Else  this is a "config" op ) We transfer it to the local config
        // object.
        else
            dConfig.copy_from( *configExtension );
        return TLM_COMPLETED;
    }

    if( trans.is_write() ) // This is an RX packet ...
        // Simply transfer packet contents from reusable UDP frame
        // container payload to recive callback function ...
        dRecvVideoFrame(
            trans.get_data_ptr(), trans.get_data_length(),
            dRecvVideoFrameContext );
    else {
        errorUnexpectedRxPacket(
            "VideoFromFabricGateway::nb_transport_fw_rx_video()" );
        trans.set_response_status( TLM_GENERIC_ERROR_RESPONSE );
    }

    // Indicate "request complete" as per TLM-2.0 LRM
    // section 8.2.4 "permitted phase transitions" table line 4.
    phase = END_RESP;
    return TLM_COMPLETED;
}
// (end inline source)
