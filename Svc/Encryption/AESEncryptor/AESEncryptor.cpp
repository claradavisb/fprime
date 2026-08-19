// ======================================================================
// \title  AESEncryptor.cpp
// \author cadena
// \brief  cpp file for AESEncryptor component implementation class
// ======================================================================

#include "Svc/Encryption/AESEncryptor/AESEncryptor.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>

namespace Svc {

namespace Ccsds {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

AESEncryptor ::AESEncryptor(const char* const compName) : AESEncryptorComponentBase(compName), m_outBuf() {}

AESEncryptor ::~AESEncryptor() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

//! Length of the AES-GCM initialization vector, in bytes
static constexpr U32 GCM_IV_LEN = 12;
//! Length of the AES-GCM authentication tag (the SDLS MAC), in bytes
static constexpr U32 GCM_TAG_LEN = 16;
//! Length of an AES-256 key, in bytes
static constexpr FwSizeType AES_256_KEY_LEN = 32;

// ======================================================================
// WARNING: this component does not yet supply additional authenticated data.
//
// The uplink counterpart (AESDecryptor) authenticates the masked frame header and security
// association index, per Svc/Ccsds/Utils/SdlsAuthMask.hpp, because that is what the YAMCS
// ground software does for TC frames. The equivalent TM mask is a different layout and has
// not been confirmed against yamcs-core, so it is deliberately not guessed at here.
//
// Consequently frames produced by this component will NOT authenticate against a YAMCS TM
// link configured for SDLS. Downlink SDLS must stay disabled -- ComCcsdsSdlsConfig selects
// ClearTextEncryptor for exactly this reason -- until the TM mask is confirmed and applied.
// ======================================================================

void AESEncryptor ::encryptIn_handler(FwIndexType portNum,
                                      U16 securityAssociationIndex,
                                      Fw::Buffer& data,
                                      const ComCfg::FrameContext& context) {
    // Output layout, assembled in m_outBuf:
    //
    //     IV (12 B) | ciphertext (N B) | MAC (16 B)
    //
    // On success the incoming plaintext buffer is returned on bufferReturnOut as soon as it
    // has been consumed, and m_outBuf is passed downstream on encryptOut. On failure the
    // incoming buffer itself is passed on encryptOut carrying the error status, so the caller
    // always learns the outcome, and is returned when it comes back on encryptReturnIn.
    // Either way each buffer is handed onward exactly once.

    const FwSizeType requiredSize = static_cast<FwSizeType>(data.getSize()) + GCM_IV_LEN + GCM_TAG_LEN;
    if (requiredSize > MAX_OUTPUT_SIZE) {
        this->encryptOut_out(0, Svc::Ccsds::SdlsStatus::ENCRYPTION_FAILURE, data, context);
        return;
    }

    Svc::Ccsds::SdlsKeyBuffer key;
    const Svc::Ccsds::SdlsStatus keyStatus = this->keyGet_out(0, key);
    if ((keyStatus != Svc::Ccsds::SdlsStatus::SUCCESS) || (key.getSize() != AES_256_KEY_LEN)) {
        this->encryptOut_out(0, Svc::Ccsds::SdlsStatus::KEY_ERROR, data, context);
        return;
    }

    U8* const iv = this->m_outBuf;
    U8* const ciphertext = this->m_outBuf + GCM_IV_LEN;

    // A repeated IV under the same key is catastrophic for GCM, so a failure of the CSPRNG
    // must abort the operation rather than proceed with whatever is in the buffer
    if (RAND_bytes(iv, static_cast<int>(GCM_IV_LEN)) != 1) {
        this->encryptOut_out(0, Svc::Ccsds::SdlsStatus::ENCRYPTION_FAILURE, data, context);
        return;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
        this->encryptOut_out(0, Svc::Ccsds::SdlsStatus::ENCRYPTION_FAILURE, data, context);
        return;
    }

    int len = 0;
    int cipherLen = 0;
    U8 tag[GCM_TAG_LEN];
    bool ok = (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1) &&
              (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(GCM_IV_LEN), nullptr) == 1) &&
              (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.getBuffAddr(), iv) == 1) &&
              (EVP_EncryptUpdate(ctx, ciphertext, &len, data.getData(), static_cast<int>(data.getSize())) == 1);
    if (ok) {
        cipherLen = len;
        ok = (EVP_EncryptFinal_ex(ctx, ciphertext + cipherLen, &len) == 1);
    }
    if (ok) {
        cipherLen += len;
        ok = (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, static_cast<int>(GCM_TAG_LEN), tag) == 1);
    }

    EVP_CIPHER_CTX_free(ctx);

    if (!ok) {
        this->encryptOut_out(0, Svc::Ccsds::SdlsStatus::ENCRYPTION_FAILURE, data, context);
        return;
    }

    (void)::memcpy(ciphertext + cipherLen, tag, GCM_TAG_LEN);
    const U32 outLen = GCM_IV_LEN + static_cast<U32>(cipherLen) + GCM_TAG_LEN;

    // The plaintext has been consumed; hand it back before passing the ciphertext downstream
    this->bufferReturnOut_out(0, data, context);

    Fw::Buffer cipherBuf(this->m_outBuf, outLen);
    this->encryptOut_out(0, Svc::Ccsds::SdlsStatus::SUCCESS, cipherBuf, context);
}

void AESEncryptor ::encryptReturnIn_handler(FwIndexType portNum,
                                            Fw::Buffer& data,
                                            const ComCfg::FrameContext& context) {
    // Two kinds of buffer come back here: m_outBuf on the success path, which is component
    // storage and must not be handed to an allocator, and the caller's own buffer on the
    // failure path, which must be returned upstream.
    if (data.getData() == this->m_outBuf) {
        return;
    }
    this->bufferReturnOut_out(0, data, context);
}

}  // namespace Ccsds

}  // namespace Svc
