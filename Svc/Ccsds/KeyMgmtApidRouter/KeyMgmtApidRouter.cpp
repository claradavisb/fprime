// ======================================================================
// \title  KeyMgmtApidRouter.cpp
// \brief  cpp file for KeyMgmtApidRouter component implementation class
// ======================================================================

#include "Svc/Ccsds/KeyMgmtApidRouter/KeyMgmtApidRouter.hpp"

namespace Svc {

namespace Ccsds {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

KeyMgmtApidRouter ::KeyMgmtApidRouter(const char* const compName) : KeyMgmtApidRouterComponentBase(compName) {}

KeyMgmtApidRouter ::~KeyMgmtApidRouter() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void KeyMgmtApidRouter ::dataIn_handler(FwIndexType portNum,
                                        Fw::Buffer& packetBuffer,
                                        const ComCfg::FrameContext& context) {
    const ComCfg::Apid::T apid = context.get_apid();

    switch (apid) {
        // The two key-management ports are optional: during bring-up the KEM reassembler and
        // EP PDU handler may not exist yet. Returning the buffer is preferable to asserting
        // on an unconnected output port, which would take the FSW down on the first uplinked
        // key-management packet.
        case ComCfg::Apid::KEM_ESTABLISHMENT: {
            if (this->isConnected_kemOut_OutputPort(0)) {
                this->kemOut_out(0, packetBuffer, context);
            } else {
                this->log_WARNING_HI_KeyMgmtPortNotConnected(apid);
                this->dataReturnOut_out(0, packetBuffer, context);
            }
            break;
        }
        case ComCfg::Apid::EP_PDU: {
            if (this->isConnected_epOut_OutputPort(0)) {
                this->epOut_out(0, packetBuffer, context);
            } else {
                this->log_WARNING_HI_KeyMgmtPortNotConnected(apid);
                this->dataReturnOut_out(0, packetBuffer, context);
            }
            break;
        }
        // The pass-through path carries all normal uplink traffic. Like Svc.FprimeRouter's
        // commandOut, it is considered an error for it to be unconnected, so it is not guarded.
        default: {
            this->passThroughOut_out(0, packetBuffer, context);
            break;
        }
    }
}

// The three return ports carry the frame context back alongside the buffer, so ownership is
// simply forwarded upstream. No buffer-to-context bookkeeping is needed.

void KeyMgmtApidRouter ::kemBufferReturnIn_handler(FwIndexType portNum,
                                                   Fw::Buffer& data,
                                                   const ComCfg::FrameContext& context) {
    this->dataReturnOut_out(0, data, context);
}

void KeyMgmtApidRouter ::epBufferReturnIn_handler(FwIndexType portNum,
                                                  Fw::Buffer& data,
                                                  const ComCfg::FrameContext& context) {
    this->dataReturnOut_out(0, data, context);
}

void KeyMgmtApidRouter ::passThroughBufferReturnIn_handler(FwIndexType portNum,
                                                           Fw::Buffer& data,
                                                           const ComCfg::FrameContext& context) {
    this->dataReturnOut_out(0, data, context);
}

}  // namespace Ccsds

}  // namespace Svc
