// ======================================================================
// \title  KeyMgmtApidRouterTester.cpp
// \brief  cpp file for KeyMgmtApidRouter component test harness implementation class
// ======================================================================

#include "KeyMgmtApidRouterTester.hpp"
#include "Fw/Types/Assert.hpp"

namespace Svc {

namespace Ccsds {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

KeyMgmtApidRouterTester ::KeyMgmtApidRouterTester()
    : KeyMgmtApidRouterGTestBase("KeyMgmtApidRouterTester", KeyMgmtApidRouterTester::MAX_HISTORY_SIZE),
      component("KeyMgmtApidRouter"),
      m_packetStorage(),
      m_nextPacket(0) {
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
    ASSERT_EQ(this->fromPortHistory_kemOut->at(0).context.get_apid(), ComCfg::Apid::KEM_ESTABLISHMENT);
    ASSERT_EVENTS_SIZE(0);
}

void KeyMgmtApidRouterTester ::testRouteEpPdu() {
    this->mockReceiveApid(ComCfg::Apid::EP_PDU);
    ASSERT_from_kemOut_SIZE(0);
    ASSERT_from_epOut_SIZE(1);
    ASSERT_from_passThroughOut_SIZE(0);
    ASSERT_from_dataReturnOut_SIZE(0);  // not returned yet (will come back on epBufferReturnIn)
    ASSERT_EQ(this->fromPortHistory_epOut->at(0).context.get_apid(), ComCfg::Apid::EP_PDU);
    ASSERT_EVENTS_SIZE(0);
}

void KeyMgmtApidRouterTester ::testRoutePassThrough() {
    this->mockReceiveApid(ComCfg::Apid::FW_PACKET_COMMAND);
    ASSERT_from_kemOut_SIZE(0);
    ASSERT_from_epOut_SIZE(0);
    ASSERT_from_passThroughOut_SIZE(1);
    ASSERT_from_dataReturnOut_SIZE(0);  // not returned yet (will come back on passThroughBufferReturnIn)
    ASSERT_EQ(this->fromPortHistory_passThroughOut->at(0).context.get_apid(), ComCfg::Apid::FW_PACKET_COMMAND);
    ASSERT_EVENTS_SIZE(0);
}

void KeyMgmtApidRouterTester ::testKemBufferReturn() {
    this->mockReceiveApid(ComCfg::Apid::KEM_ESTABLISHMENT);
    ASSERT_from_kemOut_SIZE(1);

    Fw::Buffer buffer = this->fromPortHistory_kemOut->at(0).data;
    ComCfg::FrameContext context = this->fromPortHistory_kemOut->at(0).context;
    this->invoke_to_kemBufferReturnIn(0, buffer, context);

    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), ComCfg::Apid::KEM_ESTABLISHMENT);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), buffer.getData());
    ASSERT_EVENTS_SIZE(0);
}

void KeyMgmtApidRouterTester ::testEpBufferReturn() {
    this->mockReceiveApid(ComCfg::Apid::EP_PDU);
    ASSERT_from_epOut_SIZE(1);

    Fw::Buffer buffer = this->fromPortHistory_epOut->at(0).data;
    ComCfg::FrameContext context = this->fromPortHistory_epOut->at(0).context;
    this->invoke_to_epBufferReturnIn(0, buffer, context);

    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), ComCfg::Apid::EP_PDU);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), buffer.getData());
    ASSERT_EVENTS_SIZE(0);
}

void KeyMgmtApidRouterTester ::testPassThroughBufferReturn() {
    this->mockReceiveApid(ComCfg::Apid::FW_PACKET_COMMAND);
    ASSERT_from_passThroughOut_SIZE(1);

    Fw::Buffer buffer = this->fromPortHistory_passThroughOut->at(0).data;
    ComCfg::FrameContext context = this->fromPortHistory_passThroughOut->at(0).context;
    this->invoke_to_passThroughBufferReturnIn(0, buffer, context);

    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), ComCfg::Apid::FW_PACKET_COMMAND);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), buffer.getData());
    ASSERT_EVENTS_SIZE(0);
}

void KeyMgmtApidRouterTester ::testMultiplePacketsInFlight() {
    this->mockReceiveApid(ComCfg::Apid::KEM_ESTABLISHMENT);
    this->mockReceiveApid(ComCfg::Apid::EP_PDU);
    this->mockReceiveApid(ComCfg::Apid::FW_PACKET_COMMAND);
    ASSERT_from_kemOut_SIZE(1);
    ASSERT_from_epOut_SIZE(1);
    ASSERT_from_passThroughOut_SIZE(1);
    ASSERT_from_dataReturnOut_SIZE(0);

    // Return them in a different order than they were sent: each buffer carries its own
    // context, so the order cannot matter
    Fw::Buffer ptBuffer = this->fromPortHistory_passThroughOut->at(0).data;
    ComCfg::FrameContext ptContext = this->fromPortHistory_passThroughOut->at(0).context;
    this->invoke_to_passThroughBufferReturnIn(0, ptBuffer, ptContext);

    Fw::Buffer kemBuffer = this->fromPortHistory_kemOut->at(0).data;
    ComCfg::FrameContext kemContext = this->fromPortHistory_kemOut->at(0).context;
    this->invoke_to_kemBufferReturnIn(0, kemBuffer, kemContext);

    Fw::Buffer epBuffer = this->fromPortHistory_epOut->at(0).data;
    ComCfg::FrameContext epContext = this->fromPortHistory_epOut->at(0).context;
    this->invoke_to_epBufferReturnIn(0, epBuffer, epContext);

    ASSERT_from_dataReturnOut_SIZE(3);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), ComCfg::Apid::FW_PACKET_COMMAND);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(1).context.get_apid(), ComCfg::Apid::KEM_ESTABLISHMENT);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(2).context.get_apid(), ComCfg::Apid::EP_PDU);

    // Each returned buffer is the one that was handed out, not a mix-up
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), ptBuffer.getData());
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(1).data.getData(), kemBuffer.getData());
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(2).data.getData(), epBuffer.getData());
    ASSERT_EVENTS_SIZE(0);
}

// ----------------------------------------------------------------------
// Test Helper
// ----------------------------------------------------------------------

Fw::Buffer KeyMgmtApidRouterTester::mockReceiveApid(ComCfg::Apid::T apid) {
    FW_ASSERT(this->m_nextPacket < NUM_TEST_PACKETS, static_cast<FwAssertArgType>(this->m_nextPacket));
    Fw::Buffer buffer(this->m_packetStorage[this->m_nextPacket], TEST_PACKET_SIZE);
    this->m_nextPacket++;

    ComCfg::FrameContext context;
    context.set_apid(apid);
    this->invoke_to_dataIn(0, buffer, context);
    return buffer;
}

}  // namespace Ccsds

}  // namespace Svc
