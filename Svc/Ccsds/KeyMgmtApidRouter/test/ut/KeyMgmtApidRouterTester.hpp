// ======================================================================
// \title  KeyMgmtApidRouterTester.hpp
// \brief  hpp file for KeyMgmtApidRouter component test harness implementation class
// ======================================================================

#ifndef Svc_Ccsds_KeyMgmtApidRouterTester_HPP
#define Svc_Ccsds_KeyMgmtApidRouterTester_HPP

#include "Svc/Ccsds/KeyMgmtApidRouter/KeyMgmtApidRouter.hpp"
#include "Svc/Ccsds/KeyMgmtApidRouter/KeyMgmtApidRouterGTestBase.hpp"

namespace Svc {

namespace Ccsds {

class KeyMgmtApidRouterTester final : public KeyMgmtApidRouterGTestBase {
  public:
    // ----------------------------------------------------------------------
    // Constants
    // ----------------------------------------------------------------------

    // Maximum size of histories storing events, telemetry, and port outputs
    static const FwSizeType MAX_HISTORY_SIZE = 10;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

    // Number of distinct packet buffers the test harness can hand to the component.
    // Each mockReceiveApid() call consumes one, so that concurrently outstanding
    // buffers always have distinct data pointers.
    static const FwSizeType NUM_TEST_PACKETS = 4;

    // Size of each test packet buffer
    static const FwSizeType TEST_PACKET_SIZE = 8;

  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    //! Construct object KeyMgmtApidRouterTester
    KeyMgmtApidRouterTester();

    //! Destroy object KeyMgmtApidRouterTester
    ~KeyMgmtApidRouterTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! A packet with APID KEM_ESTABLISHMENT is forwarded on kemOut only
    void testRouteKemEstablishment();

    //! A packet with APID EP_PDU is forwarded on epOut only
    void testRouteEpPdu();

    //! A packet with any other APID is forwarded on passThroughOut only
    void testRoutePassThrough();

    //! A buffer returned on kemBufferReturnIn is forwarded on dataReturnOut with its context
    void testKemBufferReturn();

    //! A buffer returned on epBufferReturnIn is forwarded on dataReturnOut with its context
    void testEpBufferReturn();

    //! A buffer returned on passThroughBufferReturnIn is forwarded on dataReturnOut with its context
    void testPassThroughBufferReturn();

    //! Several packets may be outstanding at once, and each carries its own context back
    //! regardless of the order in which they are returned
    void testMultiplePacketsInFlight();

  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

    //! Send a packet buffer with the given APID on dataIn. Each call uses a distinct
    //! backing block so that several packets can be outstanding at once.
    //! \return the buffer that was sent (the router forwards the same handle downstream)
    Fw::Buffer mockReceiveApid(ComCfg::Apid::T apid);

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    KeyMgmtApidRouter component;

    //! Backing storage for test packets, one distinct block per mockReceiveApid() call
    U8 m_packetStorage[NUM_TEST_PACKETS][TEST_PACKET_SIZE];

    //! Index of the next unused block in m_packetStorage
    FwSizeType m_nextPacket;
};

}  // namespace Ccsds

}  // namespace Svc

#endif
