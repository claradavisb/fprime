module Svc {
module Ccsds {
    @ Splits SDLS key-management traffic (KEM Establishment and Extended
    @ Procedures PDU packets) off the deframed space-packet stream by APID,
    @ forwarding everything else unchanged. Sits between a packet deframer
    @ (e.g. Svc.Ccsds.SpacePacketDeframer) and the standard Svc.Router so that
    @ ComCfg.Apid.KEM_ESTABLISHMENT and ComCfg.Apid.EP_PDU traffic reaches the
    @ key-management components instead of falling into the router's
    @ unrecognized-packet path.
    passive component KeyMgmtApidRouter {

        @ Port to receive deframed space packets to be split by APID
        guarded input port dataIn: Svc.ComDataWithContext

        @ Port for returning ownership of buffers received on dataIn
        output port dataReturnOut: Svc.ComDataWithContext

        @ Port for forwarding KEM Establishment packets (ComCfg.Apid.KEM_ESTABLISHMENT)
        output port kemOut: Svc.ComDataWithContext

        @ Port for receiving back ownership of buffers sent on kemOut
        guarded input port kemBufferReturnIn: Svc.ComDataWithContext

        @ Port for forwarding Extended Procedures PDU packets (ComCfg.Apid.EP_PDU)
        output port epOut: Svc.ComDataWithContext

        @ Port for receiving back ownership of buffers sent on epOut
        guarded input port epBufferReturnIn: Svc.ComDataWithContext

        @ Port for forwarding all other packets, unchanged, to the standard router.
        @ This port carries all normal uplink traffic and is required: like
        @ Svc.FprimeRouter's commandOut, leaving it unconnected is a topology error.
        output port passThroughOut: Svc.ComDataWithContext

        @ Port for receiving back ownership of buffers sent on passThroughOut
        guarded input port passThroughBufferReturnIn: Svc.ComDataWithContext

        @ A key-management packet arrived but its output port is not connected,
        @ which is expected during bring-up before the KEM reassembler and EP PDU
        @ handler exist. The packet is returned undelivered rather than dropped.
        event KeyMgmtPortNotConnected(
                apid: ComCfg.Apid @< The APID whose handler is not connected
            ) \
            severity warning high \
            format "No handler connected for key-management APID {}; packet returned undelivered"

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

    }
}
}
