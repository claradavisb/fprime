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

    // Maximum size of histories storing events, telemetry, and port outputs.
    // Must exceed the buffer-context table size so the table-full test can
    // send more buffers than the table holds without overflowing port history.
    static const FwSizeType MAX_HISTORY_SIZE = Svc::KeyMgmtApidRouterCfg::BufferContextTableSize + 5;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

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

    //! A buffer returned on kemBufferReturnIn restores its saved context on dataReturnOut
    void testKemBufferContextRoundTrip();

    //! A buffer returned on epBufferReturnIn restores its saved context on dataReturnOut
    void testEpBufferContextRoundTrip();

    //! A buffer returned on passThroughBufferReturnIn restores its saved context on dataReturnOut
    void testPassThroughBufferContextRoundTrip();

    //! A buffer that was never handed off still returns, but with an empty context and a
    //! BufferContextNotFound event
    void testBufferReturnNotFound();

    //! When the context table is full, a handed-off buffer emits the table-full event and its
    //! context degrades to empty on return
    void testContextTableFull();

  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

    //! Send a minimal packet buffer with the given APID on dataIn
    //! \return the buffer that was sent (the router forwards the same handle downstream)
    Fw::Buffer mockReceiveApid(ComCfg::Apid::T apid);

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    KeyMgmtApidRouter component;
};

}  // namespace Ccsds

}  // namespace Svc

#endif
