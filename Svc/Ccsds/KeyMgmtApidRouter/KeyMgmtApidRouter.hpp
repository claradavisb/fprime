// ======================================================================
// \title  KeyMgmtApidRouter.hpp
// \brief  hpp file for KeyMgmtApidRouter component implementation class
// ======================================================================

#ifndef Svc_Ccsds_KeyMgmtApidRouter_HPP
#define Svc_Ccsds_KeyMgmtApidRouter_HPP

#include "Fw/Types/ActiveEnumAc.hpp"
#include "Fw/Types/SuccessEnumAc.hpp"
#include "Svc/Ccsds/KeyMgmtApidRouter/KeyMgmtApidRouterComponentAc.hpp"
#include "Svc/Ccsds/KeyMgmtApidRouter/config/KeyMgmtApidRouterConfig/FppConstantsAc.hpp"

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
    void kemBufferReturnIn_handler(FwIndexType portNum,  //!< The port number
                                   Fw::Buffer& fwBuffer  //!< The buffer
                                   ) override;

    //! Handler implementation for epBufferReturnIn
    void epBufferReturnIn_handler(FwIndexType portNum,  //!< The port number
                                  Fw::Buffer& fwBuffer  //!< The buffer
                                  ) override;

    //! Handler implementation for passThroughBufferReturnIn
    void passThroughBufferReturnIn_handler(FwIndexType portNum,  //!< The port number
                                           Fw::Buffer& fwBuffer  //!< The buffer
                                           ) override;

  private:
    // ----------------------------------------------------------------------
    // Buffer-to-context association table
    // ----------------------------------------------------------------------

    //! One buffer-to-context association
    struct BufferContextEntry {
        Fw::Active state;              //!< ACTIVE if this slot holds an outstanding association, else INACTIVE
        const U8* key;                 //!< Data pointer of the outstanding buffer
        ComCfg::FrameContext context;  //!< Context to restore when the buffer returns
    };

    //! Record the context for a buffer being handed off.
    //! \return SUCCESS, or FAILURE if the table is full
    Fw::Success insertContext(const Fw::Buffer& buffer, const ComCfg::FrameContext& context);

    //! Look up and remove the context for a returned buffer.
    //! \return SUCCESS, or FAILURE if the buffer was not found
    Fw::Success takeContext(const Fw::Buffer& buffer, ComCfg::FrameContext& context);

    //! Common handling for the three *BufferReturnIn ports: restore context and
    //! forward to dataReturnOut
    void handleBufferReturn(Fw::Buffer& fwBuffer);

    //! Fixed-size buffer-to-context table (all entries initialized unused)
    BufferContextEntry m_bufferContextTable[Svc::KeyMgmtApidRouterCfg::BufferContextTableSize];
};

}  // namespace Ccsds

}  // namespace Svc

#endif
