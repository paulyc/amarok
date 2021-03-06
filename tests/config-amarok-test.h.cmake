/* config-amarok-test.h. Generated by cmake from config-amarok-test.h.cmake */

/* The root of the test data in Amarok's source tree. Use this if you have to access uninstalled test data */
#define AMAROK_TEST_DIR "${AMAROK_TEST_TREE}"

/*The number of tracks that are used for the SqlCollection performance tests/stress tests.
  It should be a multiple of 1000, check the unit tests to see why*/
  #define AMAROK_SQLCOLLECTION_STRESS_TEST_TRACK_COUNT ${STRESS_TEST_TRACK_COUNT}

/*The location of the built utilities in the tree */
#define AMAROK_OVERRIDE_UTILITIES_PATH "${AMAROK_UTILITIES_DIR}"