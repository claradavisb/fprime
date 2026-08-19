module ComCcsdsSdlsConfig {
    # Base ID for the ComCcsdsSdls Subtopology; the SDLS decryption instances are offsets
    # from this base ID. The packet and transfer frame layers are reused from the ComCcsds
    # subtopology and are configured through ComCcsdsConfig.
    constant BASE_ID = 0x06000000
}

module ComCcsdsSdls {
    @ Decryptor handling the base security association (SdlsSaRouter port 0).
    @ Defined in the configuration module so projects may override the configuration
    @ to select a different decryptor implementation.
    @
    @ NOTE: this configuration selects Svc.Ccsds.AESDecryptor (AES-256-GCM) for uplink,
    @ matching a YAMCS TC link configured with SecurityAssociationAes256Gcm128Factory.
    @
    @ The importing deployment must, because the shared SdlsDecryption topology does not:
    @   1. include the keyManager instance below in its topology
    @   2. connect decryptor.keyGet -> keyManager.keyGet and decryptor.keySet -> keyManager.keySet
    @   3. call decryptor.configure(vcId) with the same VC ID passed to TcDeframer.configure()
    @ Without (1) and (2) the decryptor's keyGet port is unconnected and the first uplinked
    @ frame will assert.
    instance decryptor: Svc.Ccsds.AESDecryptor base id ComCcsdsSdlsConfig.BASE_ID + 0x02000

    @ Default encryptor used by the SDLS framer on the downlink path. Defined in the
    @ configuration module so projects may override the configuration to select a
    @ different encryptor implementation.
    @
    @ WARNING: the default Svc.Ccsds.ClearTextEncryptor provides NO security: no
    @ confidentiality, no integrity, and no authentication.
    @
    @ Downlink is deliberately left in the clear: the YAMCS reference configuration enables
    @ SDLS on the TC link only, and Svc.Ccsds.AESEncryptor does not yet authenticate the TM
    @ frame header (see the warning in AESEncryptor.cpp). Selecting AESEncryptor here would
    @ produce frames YAMCS cannot authenticate.
    instance encryptor: Svc.Ccsds.ClearTextEncryptor base id ComCcsdsSdlsConfig.BASE_ID + 0x04000

    @ Key source for the decryptor. Supplies the AES-256 session key from a file, so that the
    @ same 32-byte key can be installed on both ends before key management exists.
    @
    @ The deployment must call keyManager.configure(<path>, 32) during topology setup; a key
    @ request before configuration asserts.
    instance keyManager: Svc.Ccsds.SdlsFileKeyManager base id ComCcsdsSdlsConfig.BASE_ID + 0x06000
}
