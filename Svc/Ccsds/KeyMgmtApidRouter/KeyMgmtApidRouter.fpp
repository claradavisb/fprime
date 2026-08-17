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
        guarded input port kemBufferReturnIn: Fw.BufferSend

        @ Port for forwarding Extended Procedures PDU packets (ComCfg.Apid.EP_PDU)
        output port epOut: Svc.ComDataWithContext

        @ Port for receiving back ownership of buffers sent on epOut
        guarded input port epBufferReturnIn: Fw.BufferSend

        @ Port for forwarding all other packets, unchanged, to the standard router
        output port passThroughOut: Svc.ComDataWithContext

        @ Port for receiving back ownership of buffers sent on passThroughOut
        guarded input port passThroughBufferReturnIn: Fw.BufferSend

        @ The buffer-to-context table was full when a buffer was handed off.
        @ The buffer is still forwarded, but its context cannot be restored on
        @ return and will be returned empty.
        event BufferContextTableFull() \
            severity warning high \
            format "Buffer-to-context table full in KeyMgmtApidRouter; context will be lost for this buffer"

        @ A buffer returned on one of the *BufferReturnIn ports was not found
        @ in the buffer-to-context table. The buffer is still returned, but
        @ with an empty context.
        event BufferContextNotFound() \
            severity warning high \
            format "Returned buffer not found in context table; returning with empty context"

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
