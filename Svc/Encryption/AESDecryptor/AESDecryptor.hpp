// ======================================================================
// \title  AESDecryptor.hpp
// \author cadena
// \brief  hpp file for AESDecryptor component implementation class
// ======================================================================

#ifndef Svc_Ccsds_AESDecryptor_HPP
#define Svc_Ccsds_AESDecryptor_HPP

#include "Svc/Encryption/AESDecryptor/AESDecryptorComponentAc.hpp"

namespace Svc {

namespace Ccsds {

class AESDecryptor final : public AESDecryptorComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct AESDecryptor object
    AESDecryptor(const char* const compName  //!< The component name
    );

    //! Destroy AESDecryptor object
    ~AESDecryptor();

    //! Set the virtual channel this component decrypts for.
    //!
    //! SDLS authenticates the frame's virtual channel ID, but the TC primary header carrying
    //! it has already been stripped by the time a buffer reaches this component, and
    //! ComCfg::FrameContext is not populated with it on the TC uplink path. The VC is
    //! therefore supplied here instead, and MUST match the value the deployment passes to
    //! Svc::Ccsds::TcDeframer::configure(); a mismatch makes every frame fail its MAC check.
    //!
    //! Defaults to 1 if never called, matching the ComCfg::FrameContext default.
    //!
    //! NOTE: this limits the component to a single virtual channel. Supporting several would
    //! require the VCID to travel on the frame context.
    void configure(U8 vcId  //!< Virtual channel ID (6 bits)
    );

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for decryptIn
    //!
    //! Port to receive the security association index and iv/data buffer to decrypt
    void decryptIn_handler(FwIndexType portNum,  //!< The port number
                           U16 securityAssociationIndex,
                           Fw::Buffer& data,
                           const ComCfg::FrameContext& context) override;

    //! Handler implementation for decryptReturnIn
    //!
    //! Port for receiving back ownership of buffers sent on decryptOut
    void decryptReturnIn_handler(FwIndexType portNum,  //!< The port number
                                 Fw::Buffer& data,
                                 const ComCfg::FrameContext& context) override;

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! Virtual channel ID authenticated in the SDLS additional authenticated data
    U8 m_vcId;
};

}  // namespace Ccsds

}  // namespace Svc

#endif
