# ======================================================================
# SdlsSaRouterCfg.fpp
# Compile-time configuration for the SdlsSaRouter component
# ======================================================================

module SdlsCfg {

    @ Number of downstream crypto component ports on the SdlsSaRouter
    constant SaRouterPortCount = 2

    @ Number of entries in the SA-to-port routing map
    constant SaRouterMapEntryCount = 2

    @ Maximum number of processed data buffers outstanding (sent downstream, not yet returned)
    constant SaRouterMaxOutstandingBuffers = 4

    @ Enumeration of the downstream crypto component ports on the SdlsSaRouter
    enum SaRouterPorts : FwIndexType {
        UNCONNECTED = 0
        PLAINTEXT = 1
    }

    @ Compile-time map from security association index to downstream port index. Projects
    @ may define sparse or non-linear SA ranges that map down to a compact, linear port
    @ array. Port indices must be in [0, SaRouterPortCount).
    @
    @ This single table is shared by BOTH SdlsSaRouter instances -- decryptionSaRouter
    @ (uplink) and encryptionSaRouter (downlink) -- it is not per-instance. SA 0 is the
    @ table's only routed entry, matching CcsdsSdlsFramer's SA_INDEX parameter default (0):
    @ any outbound frame whose context does not specify an SA (i.e. essentially all normal
    @ telemetry/event downlink) falls back to SA 0, so SA 0 must stay routed to a connected
    @ port or every downlink frame fails encryption with UNKNOWN_PORT. SA 1 is left
    @ unrouted; the ground's SDLS SPI must be configured to match SA 0, not SA 1.
    @
    @ NOTE: the port is still named PLAINTEXT for compatibility with the upstream framework,
    @ but on the uplink side it hosts whichever component ComCcsdsSdlsConfig selects --
    @ Svc.Ccsds.AESDecryptor in this configuration, not a pass-through. Restoring a genuine
    @ clear-mode SA means adding a second crypto instance on the other port rather than
    @ pointing an SA at this one.
    array SaMap = [SaRouterMapEntryCount] Svc.Ccsds.SaMapEntry default [
        { securityAssociationIndex = 0, portIndex = SaRouterPorts.PLAINTEXT },
        { securityAssociationIndex = 1, portIndex = SaRouterPorts.UNCONNECTED }
    ]

}
