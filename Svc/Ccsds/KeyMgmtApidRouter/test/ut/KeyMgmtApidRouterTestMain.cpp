// ======================================================================
// \title  KeyMgmtApidRouterTestMain.cpp
// \brief  cpp file for KeyMgmtApidRouter component test main function
// ======================================================================

#include "KeyMgmtApidRouterTester.hpp"

TEST(KeyMgmtApidRouter, TestRouteKemEstablishment) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testRouteKemEstablishment();
}
TEST(KeyMgmtApidRouter, TestRouteEpPdu) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testRouteEpPdu();
}
TEST(KeyMgmtApidRouter, TestRoutePassThrough) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testRoutePassThrough();
}
TEST(KeyMgmtApidRouter, TestKemBufferContextRoundTrip) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testKemBufferContextRoundTrip();
}
TEST(KeyMgmtApidRouter, TestEpBufferContextRoundTrip) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testEpBufferContextRoundTrip();
}
TEST(KeyMgmtApidRouter, TestPassThroughBufferContextRoundTrip) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testPassThroughBufferContextRoundTrip();
}
TEST(KeyMgmtApidRouter, TestBufferReturnNotFound) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testBufferReturnNotFound();
}
TEST(KeyMgmtApidRouter, TestContextTableFull) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testContextTableFull();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
