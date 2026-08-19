// ======================================================================
// \title  AESEncryptor.hpp
// \author cadena
// \brief  hpp file for AESEncryptor component implementation class
// ======================================================================

#ifndef Svc_Ccsds_AESEncryptor_HPP
#define Svc_Ccsds_AESEncryptor_HPP

#include "Svc/Encryption/AESEncryptor/AESEncryptorComponentAc.hpp"

namespace Svc {

namespace Ccsds {

class AESEncryptor final : public AESEncryptorComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct AESEncryptor object
    AESEncryptor(const char* const compName  //!< The component name
    );

    //! Destroy AESEncryptor object
    ~AESEncryptor();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for encryptIn
    //!
    //! Port to receive the security association index and iv/data buffer to encrypt
    void encryptIn_handler(FwIndexType portNum,  //!< The port number
                           U16 securityAssociationIndex,
                           Fw::Buffer& data,
                           const ComCfg::FrameContext& context) override;

    //! Handler implementation for encryptReturnIn
    //!
    //! Port for receiving back ownership of buffers sent on encryptOut
    void encryptReturnIn_handler(FwIndexType portNum,  //!< The port number
                                 Fw::Buffer& data,
                                 const ComCfg::FrameContext& context) override;

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! Largest output this component can produce, i.e. the largest plaintext it accepts plus
    //! the 12-byte IV and 16-byte MAC.
    //!
    //! TODO: this belongs in a config module, as SdlsCfg does for key sizes, so projects can
    //! size it against their frame length rather than editing this header.
    static constexpr FwSizeType MAX_OUTPUT_SIZE = 1024;

    //! Scratch storage for the assembled IV | ciphertext | MAC output.
    //!
    //! WARNING: a single buffer per component instance, which is only safe while the
    //! encryptOut round trip completes before the next encryptIn call. That holds for the
    //! passive, synchronous SDLS pipeline this component was written for, and breaks if any
    //! component between here and the framer becomes active or queued. Allocating from a
    //! Svc.BufferManager through a bufferAllocate port is the correct fix and is required
    //! before this component is used on a downlink that matters.
    U8 m_outBuf[MAX_OUTPUT_SIZE];
};

}  // namespace Ccsds

}  // namespace Svc

#endif
