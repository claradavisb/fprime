// ======================================================================
// \title  KeyMgmtApidRouterTester.cpp
// \brief  cpp file for KeyMgmtApidRouter component test harness implementation class
// ======================================================================

#include "KeyMgmtApidRouterTester.hpp"

namespace Svc {

namespace Ccsds {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

KeyMgmtApidRouterTester ::KeyMgmtApidRouterTester()
    : KeyMgmtApidRouterGTestBase("KeyMgmtApidRouterTester", KeyMgmtApidRouterTester::MAX_HISTORY_SIZE),
      component("KeyMgmtApidRouter") {
    this->initComponents();
    this->connectPorts();
}

KeyMgmtApidRouterTester ::~KeyMgmtApidRouterTester() {}

// ----------------------------------------------------------------------
// Test Cases
// ----------------------------------------------------------------------

void KeyMgmtApidRouterTester ::testRouteKemEstablishment() {
    this->mockReceiveApid(ComCfg::Apid::KEM_ESTABLISHMENT);
    ASSERT_from_kemOut_SIZE(1);
    ASSERT_from_epOut_SIZE(0);
    ASSERT_from_passThroughOut_SIZE(0);
    ASSERT_from_dataReturnOut_SIZE(0);  // not returned yet (will come back on kemBufferReturnIn)
}

void KeyMgmtApidRouterTester ::testRouteEpPdu() {
    this->mockReceiveApid(ComCfg::Apid::EP_PDU);
    ASSERT_from_kemOut_SIZE(0);
    ASSERT_from_epOut_SIZE(1);
    ASSERT_from_passThroughOut_SIZE(0);
    ASSERT_from_dataReturnOut_SIZE(0);  // not returned yet (will come back on epBufferReturnIn)
}

void KeyMgmtApidRouterTester ::testRoutePassThrough() {
    this->mockReceiveApid(ComCfg::Apid::FW_PACKET_COMMAND);
    ASSERT_from_kemOut_SIZE(0);
    ASSERT_from_epOut_SIZE(0);
    ASSERT_from_passThroughOut_SIZE(1);
    ASSERT_from_dataReturnOut_SIZE(0);  // not returned yet (will come back on passThroughBufferReturnIn)
}

void KeyMgmtApidRouterTester ::testKemBufferContextRoundTrip() {
    Fw::Buffer sent = this->mockReceiveApid(ComCfg::Apid::KEM_ESTABLISHMENT);
    ASSERT_from_kemOut_SIZE(1);

    Fw::Buffer returned = this->fromPortHistory_kemOut->at(0).data;
    this->invoke_to_kemBufferReturnIn(0, returned);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), ComCfg::Apid::KEM_ESTABLISHMENT);
    ASSERT_EVENTS_SIZE(0);  // no degrade events
}

void KeyMgmtApidRouterTester ::testEpBufferContextRoundTrip() {
    Fw::Buffer sent = this->mockReceiveApid(ComCfg::Apid::EP_PDU);
    ASSERT_from_epOut_SIZE(1);

    Fw::Buffer returned = this->fromPortHistory_epOut->at(0).data;
    this->invoke_to_epBufferReturnIn(0, returned);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), ComCfg::Apid::EP_PDU);
    ASSERT_EVENTS_SIZE(0);
}

void KeyMgmtApidRouterTester ::testPassThroughBufferContextRoundTrip() {
    Fw::Buffer sent = this->mockReceiveApid(ComCfg::Apid::FW_PACKET_COMMAND);
    ASSERT_from_passThroughOut_SIZE(1);

    Fw::Buffer returned = this->fromPortHistory_passThroughOut->at(0).data;
    this->invoke_to_passThroughBufferReturnIn(0, returned);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), ComCfg::Apid::FW_PACKET_COMMAND);
    ASSERT_EVENTS_SIZE(0);
}

void KeyMgmtApidRouterTester ::testBufferReturnNotFound() {
    U8 data[1];
    Fw::Buffer buffer(data, sizeof(data));
    this->invoke_to_kemBufferReturnIn(0, buffer);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EVENTS_BufferContextNotFound_SIZE(1);
    ComCfg::FrameContext defaultCtx;
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), defaultCtx.get_apid());
}

void KeyMgmtApidRouterTester ::testContextTableFull() {
    const FwSizeType tableSize = Svc::KeyMgmtApidRouterCfg::BufferContextTableSize;

    // Use a heap array of buffers so their data pointers stay distinct and alive
    U8* blocks = new U8[(tableSize + 1) * sizeof(FwPacketDescriptorType)];
    Fw::Buffer* buffers = new Fw::Buffer[tableSize + 1];
    for (FwSizeType i = 0; i < tableSize + 1; i++) {
        buffers[i] = Fw::Buffer(blocks + (i * sizeof(FwPacketDescriptorType)), sizeof(FwPacketDescriptorType));
        ComCfg::FrameContext ctx;
        ctx.set_apid(ComCfg::Apid::KEM_ESTABLISHMENT);
        this->invoke_to_dataIn(0, buffers[i], ctx);
    }
    // Exactly one overflow event for the (tableSize+1)th send
    ASSERT_EVENTS_BufferContextTableFull_SIZE(1);

    // The overflow buffer (last) returns with an empty/default context
    this->invoke_to_kemBufferReturnIn(0, buffers[tableSize]);
    const U8 lastReturnIdx = static_cast<U8>(this->fromPortHistory_dataReturnOut->size() - 1);
    ComCfg::FrameContext defaultCtx;
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(lastReturnIdx).context.get_apid(), defaultCtx.get_apid());
    ASSERT_EVENTS_BufferContextNotFound_SIZE(1);  // overflow buffer was never in the table

    delete[] buffers;
    delete[] blocks;
}

// ----------------------------------------------------------------------
// Test Helper
// ----------------------------------------------------------------------

Fw::Buffer KeyMgmtApidRouterTester::mockReceiveApid(ComCfg::Apid::T apid) {
    static U8 data[sizeof(FwPacketDescriptorType)];
    Fw::Buffer buffer(data, sizeof(data));
    ComCfg::FrameContext context;
    context.set_apid(apid);
    this->invoke_to_dataIn(0, buffer, context);
    return buffer;
}

}  // namespace Ccsds

}  // namespace Svc
