// ======================================================================
// \title  KeyMgmtApidRouter.cpp
// \brief  cpp file for KeyMgmtApidRouter component implementation class
// ======================================================================

#include "Svc/Ccsds/KeyMgmtApidRouter/KeyMgmtApidRouter.hpp"
#include "Fw/FPrimeBasicTypes.hpp"

namespace Svc {

namespace Ccsds {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

KeyMgmtApidRouter ::KeyMgmtApidRouter(const char* const compName) : KeyMgmtApidRouterComponentBase(compName) {
    // Mark every table entry unused
    for (FwSizeType i = 0; i < FW_NUM_ARRAY_ELEMENTS(this->m_bufferContextTable); i++) {
        this->m_bufferContextTable[i].state = Fw::Active::INACTIVE;
    }
}

KeyMgmtApidRouter ::~KeyMgmtApidRouter() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void KeyMgmtApidRouter ::dataIn_handler(FwIndexType portNum,
                                        Fw::Buffer& packetBuffer,
                                        const ComCfg::FrameContext& context) {
    ComCfg::Apid::T apid = context.get_apid();

    switch (apid) {
        case ComCfg::Apid::KEM_ESTABLISHMENT: {
            if (this->insertContext(packetBuffer, context) == Fw::Success::FAILURE) {
                this->log_WARNING_HI_BufferContextTableFull();
            }
            this->kemOut_out(0, packetBuffer, context);
            break;
        }
        case ComCfg::Apid::EP_PDU: {
            if (this->insertContext(packetBuffer, context) == Fw::Success::FAILURE) {
                this->log_WARNING_HI_BufferContextTableFull();
            }
            this->epOut_out(0, packetBuffer, context);
            break;
        }
        default: {
            if (this->insertContext(packetBuffer, context) == Fw::Success::FAILURE) {
                this->log_WARNING_HI_BufferContextTableFull();
            }
            this->passThroughOut_out(0, packetBuffer, context);
            break;
        }
    }
}

void KeyMgmtApidRouter ::kemBufferReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    this->handleBufferReturn(fwBuffer);
}

void KeyMgmtApidRouter ::epBufferReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    this->handleBufferReturn(fwBuffer);
}

void KeyMgmtApidRouter ::passThroughBufferReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    this->handleBufferReturn(fwBuffer);
}

// ----------------------------------------------------------------------
// Private helper methods
// ----------------------------------------------------------------------

void KeyMgmtApidRouter ::handleBufferReturn(Fw::Buffer& fwBuffer) {
    ComCfg::FrameContext context;
    if (this->takeContext(fwBuffer, context) == Fw::Success::FAILURE) {
        // Buffer not found in the table: return with an empty context (already default)
        this->log_WARNING_HI_BufferContextNotFound();
    }
    this->dataReturnOut_out(0, fwBuffer, context);
}

// ----------------------------------------------------------------------
// Buffer-to-context association table helpers
// ----------------------------------------------------------------------

Fw::Success KeyMgmtApidRouter ::insertContext(const Fw::Buffer& buffer, const ComCfg::FrameContext& context) {
    const U8* key = buffer.getData();
    for (FwSizeType i = 0; i < FW_NUM_ARRAY_ELEMENTS(this->m_bufferContextTable); i++) {
        if (this->m_bufferContextTable[i].state == Fw::Active::INACTIVE) {
            this->m_bufferContextTable[i].state = Fw::Active::ACTIVE;
            this->m_bufferContextTable[i].key = key;
            this->m_bufferContextTable[i].context = context;
            return Fw::Success::SUCCESS;
        }
    }
    // Table full
    return Fw::Success::FAILURE;
}

Fw::Success KeyMgmtApidRouter ::takeContext(const Fw::Buffer& buffer, ComCfg::FrameContext& context) {
    const U8* key = buffer.getData();
    for (FwSizeType i = 0; i < FW_NUM_ARRAY_ELEMENTS(this->m_bufferContextTable); i++) {
        if (this->m_bufferContextTable[i].state == Fw::Active::ACTIVE && this->m_bufferContextTable[i].key == key) {
            context = this->m_bufferContextTable[i].context;
            this->m_bufferContextTable[i].state = Fw::Active::INACTIVE;
            return Fw::Success::SUCCESS;
        }
    }
    // Not found
    return Fw::Success::FAILURE;
}

}  // namespace Ccsds

}  // namespace Svc
