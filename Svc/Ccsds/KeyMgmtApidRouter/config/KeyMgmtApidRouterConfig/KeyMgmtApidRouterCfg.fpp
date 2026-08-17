# ======================================================================
# KeyMgmtApidRouterCfg.fpp
# Compile-time configuration for the KeyMgmtApidRouter component
# ======================================================================

module Svc {

  module KeyMgmtApidRouterCfg {

    @ Capacity of KeyMgmtApidRouter's buffer->FrameContext table, bounding how
    @ many buffers handed off on kemOut/epOut/passThroughOut can await return
    @ at once. Size it to the buffer pool that feeds dataIn, the hard upper
    @ bound on outstanding buffers.
    constant BufferContextTableSize = 50

  }

}
