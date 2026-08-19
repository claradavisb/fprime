// ======================================================================
// \title  KeyMgmtApidRouter.hpp
// \brief  hpp file for KeyMgmtApidRouter component implementation class
// ======================================================================

#ifndef Svc_Ccsds_KeyMgmtApidRouter_HPP
#define Svc_Ccsds_KeyMgmtApidRouter_HPP

#include "Svc/Ccsds/KeyMgmtApidRouter/KeyMgmtApidRouterComponentAc.hpp"

namespace Svc {

namespace Ccsds {

class KeyMgmtApidRouter final : public KeyMgmtApidRouterComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct KeyMgmtApidRouter object
    KeyMgmtApidRouter(const char* const compName  //!< The component name
    );

    //! Destroy KeyMgmtApidRouter object
    ~KeyMgmtApidRouter();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for dataIn
    //!
    //! Receives deframed space packets to be split by APID
    void dataIn_handler(FwIndexType portNum,                 //!< The port number
                        Fw::Buffer& packetBuffer,            //!< The packet buffer
                        const ComCfg::FrameContext& context  //!< The context object
                        ) override;

    //! Handler implementation for kemBufferReturnIn
    void kemBufferReturnIn_handler(FwIndexType portNum,                 //!< The port number
                                   Fw::Buffer& data,                    //!< The buffer
                                   const ComCfg::FrameContext& context  //!< The context object
                                   ) override;

    //! Handler implementation for epBufferReturnIn
    void epBufferReturnIn_handler(FwIndexType portNum,                 //!< The port number
                                  Fw::Buffer& data,                    //!< The buffer
                                  const ComCfg::FrameContext& context  //!< The context object
                                  ) override;

    //! Handler implementation for passThroughBufferReturnIn
    void passThroughBufferReturnIn_handler(FwIndexType portNum,                 //!< The port number
                                           Fw::Buffer& data,                    //!< The buffer
                                           const ComCfg::FrameContext& context  //!< The context object
                                           ) override;
};

}  // namespace Ccsds

}  // namespace Svc

#endif
