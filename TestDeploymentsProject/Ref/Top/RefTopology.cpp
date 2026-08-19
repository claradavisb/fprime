// ======================================================================
// \title  Topology.cpp
// \author mstarch
// \brief cpp file containing the topology instantiation code
//
// \copyright
// Copyright 2009-2022, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ======================================================================

// Provides access to autocoded functions
#include <Ref/Top/RefTopologyAc.hpp>

// Necessary project-specified types
#include <Fw/Types/MallocAllocator.hpp>

// For the SDLS key file path environment override
#include <cstdlib>

// Allows easy reference to objects in FPP/autocoder required namespaces
using namespace Ref;

// Instantiate a malloc allocator for cmdSeq buffer allocation
Fw::MallocAllocator mallocator;

// The reference topology divides the incoming clock signal (1Hz) into sub-signals: 1Hz, 1/2Hz, and 1/4Hz and
// zero offset for all the dividers
Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{1, 0}, {2, 0}, {4, 0}}};

// Rate groups may supply a context token to each of the attached children whose purpose is set by the project. The
// reference topology sets each token to zero as these contexts are unused in this project.
Svc::ActiveRateGroup::ContextArray rateGroup1Context(0);
Svc::ActiveRateGroup::ContextArray rateGroup2Context(0);
Svc::ActiveRateGroup::ContextArray rateGroup3Context(0);

enum TopologyConstants {
    COMM_PRIORITY = 34,
    // YAMCS UDP frame link endpoints (yamcs.fprime-project.yaml in fprime-yamcs):
    // UdpTmFrameLink listens for TM datagrams on 50000; UdpTcFrameLink sends TC
    // datagrams to <FSW>:50001.
    YAMCS_TM_PORT = 50000,
    TC_RECV_PORT = 50001,
};

// SDLS session key: 32 bytes of AES-256 key material, pre-shared with the ground for
// milestone-1 bring-up (the same file the YAMCS SDLS link reads, per the fprime-yamcs
// integration test README). Path overridable via the SDLS_KEY_FILE environment variable.
static constexpr const char* SDLS_KEY_FILE_DEFAULT = "sdls-key.bin";
static constexpr FwSizeType SDLS_KEY_SIZE = 32;

// Virtual channel carrying SDLS-protected TC uplink. Must match the vcId in the YAMCS
// UdpTcFrameLink virtualChannels configuration; it is authenticated in the SDLS AAD, so a
// mismatch fails every frame's MAC check.
static constexpr U8 SDLS_TC_VCID = 1;

/**
 * \brief configure/setup components in project-specific way
 *
 * This is a *helper* function which configures/sets up each component requiring project specific input. This includes
 * allocating resources, passing-in arguments, etc. This function may be inlined into the topology setup function if
 * desired, but is extracted here for clarity.
 */
void configureTopology() {
    // Rate group driver needs a divisor list
    rateGroupDriverComp.configure(rateGroupDivisorsSet);

    // Rate groups require context arrays. Empty for Reference example.
    rateGroup1Comp.configure(rateGroup1Context);
    rateGroup2Comp.configure(rateGroup2Context);
    rateGroup3Comp.configure(rateGroup3Context);

    // Command sequencer needs to allocate memory to hold contents of command sequences
    cmdSeq.allocateBuffer(0, mallocator, 5 * 1024);

    // Restrict uplinked files to a sandbox directory to prevent path-traversal writes
    FileHandling::fileUplink.configure("/tmp/uplink/");

    // PrmDb file name must be supplied by the using topology
    FileHandling::prmDb.configure("PrmDb.dat");

    // SDLS: point the file-backed key manager at the pre-shared session key. Must happen
    // before the first uplinked frame; an unconfigured key manager asserts on keyGet.
    const char* keyFile = getenv("SDLS_KEY_FILE");
    ComCcsdsSdls::keyManager.configure((keyFile != nullptr) ? keyFile : SDLS_KEY_FILE_DEFAULT, SDLS_KEY_SIZE);

    // SDLS: the decryptor authenticates the virtual channel ID in the AAD, but the TC
    // primary header carrying it is stripped upstream, so the VC is supplied here
    ComCcsdsSdls::decryptor.configure(SDLS_TC_VCID);
}

// Public functions for use in main program are namespaced with deployment name Ref
namespace Ref {
void setupTopology(const TopologyState& state) {
    // Autocoded initialization. Function provided by autocoder.
    initComponents(state);
    // Autocoded id setup. Function provided by autocoder.
    setBaseIds();
    // Autocoded connection wiring. Function provided by autocoder.
    connectComponents();
    // Autocoded configuration. Function provided by autocoder.
    configComponents(state);
    if (state.hostname != nullptr) {
        // TM downlink: one frame per datagram to the YAMCS UdpTmFrameLink intake
        (void)comDriver.configureSend(state.hostname, (state.port != 0) ? state.port : static_cast<U16>(YAMCS_TM_PORT));
        // TC uplink: bind locally for datagrams from the YAMCS UdpTcFrameLink outlet
        (void)comDriver.configureRecv("0.0.0.0", TC_RECV_PORT);
    }
    // Project-specific component configuration. Function provided above. May be inlined, if desired.
    configureTopology();
    // Autocoded command registration. Function provided by autocoder.
    regCommands();
    // Autocoded parameter loading. Function provided by autocoder.
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);
    // Initialize socket communication if and only if there is a valid specification
    if (state.hostname != nullptr) {
        Os::TaskString name("ReceiveTask");
        comDriver.start(name, COMM_PRIORITY, Default::STACK_SIZE);
    }
}

void startRateGroups(const Fw::TimeInterval& interval) {
    // This timer drives the fundamental tick rate of the system.
    // Svc::RateGroupDriver will divide this down to the slower rate groups.
    // This call will block until the stopRateGroups() call is made.
    // For this Linux demo, that call is made from a signal handler.
    linuxTimer.startTimer(interval);
}

void stopRateGroups() {
    linuxTimer.quit();
}

void teardownTopology(const TopologyState& state) {
    // Autocoded (active component) task clean-up. Functions provided by topology autocoder.
    stopTasks(state);
    freeThreads(state);

    // Stop the comDriver component, free thread
    comDriver.stop();
    (void)comDriver.join();

    // Resource deallocation
    cmdSeq.deallocateBuffer(mallocator);
    tearDownComponents(state);
    deinitComponents(state);
}
}  // namespace Ref
