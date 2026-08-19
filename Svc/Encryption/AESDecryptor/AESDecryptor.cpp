// ======================================================================
// \title  AESDecryptor.cpp
// \author cadena
// \brief  cpp file for AESDecryptor component implementation class
// ======================================================================

#include "Svc/Encryption/AESDecryptor/AESDecryptor.hpp"
#include <openssl/evp.h>
#include "Svc/Ccsds/Utils/SdlsAuthMask.hpp"

namespace Svc {

namespace Ccsds {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

//! Default virtual channel, matching the ComCfg::FrameContext default
static constexpr U8 DEFAULT_VC_ID = 1;

AESDecryptor ::AESDecryptor(const char* const compName)
    : AESDecryptorComponentBase(compName), m_vcId(DEFAULT_VC_ID) {}

AESDecryptor ::~AESDecryptor() {}

void AESDecryptor ::configure(U8 vcId) {
    this->m_vcId = vcId;
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

//! Length of the AES-GCM initialization vector, in bytes
static constexpr U32 GCM_IV_LEN = 12;
//! Length of the AES-GCM authentication tag (the SDLS MAC), in bytes
static constexpr U32 GCM_TAG_LEN = 16;
//! Length of an AES-256 key, in bytes
static constexpr FwSizeType AES_256_KEY_LEN = 32;

void AESDecryptor ::decryptIn_handler(FwIndexType portNum,
                                      U16 securityAssociationIndex,
                                      Fw::Buffer& data,
                                      const ComCfg::FrameContext& context) {
    // Wire layout of the SDLS frame data field, after CcsdsSdlsDeframer has stripped the
    // 2-byte security association index:
    //
    //     IV (12 B) | ciphertext (N B) | MAC (16 B)
    //
    // Decryption is performed in place. The plaintext is never larger than the ciphertext, so
    // no output buffer is allocated: the incoming buffer is decrypted, narrowed to the
    // plaintext, and passed downstream. Ownership comes back on decryptReturnIn and is
    // returned upstream from there, so every path through this handler ends in exactly one
    // decryptOut_out call and no path returns the buffer twice.

    if (data.getSize() < GCM_IV_LEN + GCM_TAG_LEN) {
        // Too short to hold an IV and a MAC, let alone any ciphertext
        this->decryptOut_out(0, Svc::Ccsds::SdlsStatus::DECRYPTION_FAILURE, data, context);
        return;
    }

    Svc::Ccsds::SdlsKeyBuffer key;
    const Svc::Ccsds::SdlsStatus keyStatus = this->keyGet_out(0, key);
    if ((keyStatus != Svc::Ccsds::SdlsStatus::SUCCESS) || (key.getSize() != AES_256_KEY_LEN)) {
        // A short key would silently decrypt under the wrong key material, so treat a
        // wrong-sized key as a key error rather than attempting the operation
        this->decryptOut_out(0, Svc::Ccsds::SdlsStatus::KEY_ERROR, data, context);
        return;
    }

    U8* const iv = data.getData();
    U8* const ciphertext = data.getData() + GCM_IV_LEN;
    const U32 cipherLen = static_cast<U32>(data.getSize()) - GCM_IV_LEN - GCM_TAG_LEN;
    U8* const tag = ciphertext + cipherLen;

    // The fields that bind this frame to its virtual channel and security association are
    // authenticated but not encrypted. The SA index arrives as an explicit argument, having
    // already selected this component; the VC comes from configure(), because the TC primary
    // header that carried it was stripped upstream and is not recorded on the frame context.
    const Svc::Ccsds::Utils::SdlsTcAuthMask aad(this->m_vcId, securityAssociationIndex);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
        this->decryptOut_out(0, Svc::Ccsds::SdlsStatus::DECRYPTION_FAILURE, data, context);
        return;
    }

    int len = 0;
    int plainLen = 0;
    // Passing a null output pointer to EVP_DecryptUpdate feeds the block as AAD rather than
    // ciphertext. It must precede the payload.
    bool ok = (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1) &&
              (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(GCM_IV_LEN), nullptr) == 1) &&
              (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.getBuffAddr(), iv) == 1) &&
              (EVP_DecryptUpdate(ctx, nullptr, &len, aad.bytes, static_cast<int>(sizeof(aad.bytes))) == 1) &&
              // in == out: the documented in-place form
              (EVP_DecryptUpdate(ctx, ciphertext, &len, ciphertext, static_cast<int>(cipherLen)) == 1);
    if (ok) {
        plainLen = len;
        ok = (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(GCM_TAG_LEN), tag) == 1) &&
             // GCM emits no trailing plaintext; this call exists to verify the MAC
             (EVP_DecryptFinal_ex(ctx, ciphertext + plainLen, &len) == 1);
    }
    if (ok) {
        plainLen += len;
    }

    EVP_CIPHER_CTX_free(ctx);

    if (!ok) {
        // A failed MAC check is deliberately indistinguishable from any other decryption
        // failure: reporting which one leaks information to an attacker
        this->decryptOut_out(0, Svc::Ccsds::SdlsStatus::DECRYPTION_FAILURE, data, context);
        return;
    }

    // Narrow the buffer to the plaintext in place. advance() and setSize() are bounds-checked
    // against the original allocation, and Fw::Buffer carries its allocator context
    // independently of the data pointer, so the buffer remains deallocatable after this.
    data.advance(static_cast<FwSignedSizeType>(GCM_IV_LEN));
    data.setSize(static_cast<Fw::Buffer::SizeType>(plainLen));
    this->decryptOut_out(0, Svc::Ccsds::SdlsStatus::SUCCESS, data, context);
}

void AESDecryptor ::decryptReturnIn_handler(FwIndexType portNum,
                                            Fw::Buffer& data,
                                            const ComCfg::FrameContext& context) {
    // Every buffer sent on decryptOut is the one received on decryptIn, so returning it
    // upstream is always correct
    this->bufferReturnOut_out(0, data, context);
}

}  // namespace Ccsds

}  // namespace Svc
