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
TEST(KeyMgmtApidRouter, TestKemBufferReturn) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testKemBufferReturn();
}
TEST(KeyMgmtApidRouter, TestEpBufferReturn) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testEpBufferReturn();
}
TEST(KeyMgmtApidRouter, TestPassThroughBufferReturn) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testPassThroughBufferReturn();
}
TEST(KeyMgmtApidRouter, TestMultiplePacketsInFlight) {
    Svc::Ccsds::KeyMgmtApidRouterTester tester;
    tester.testMultiplePacketsInFlight();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
