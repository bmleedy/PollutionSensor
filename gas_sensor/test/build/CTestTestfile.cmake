# CMake generated Testfile for 
# Source directory: /Users/bmleedy/PollutionSensor/gas_sensor/test
# Build directory: /Users/bmleedy/PollutionSensor/gas_sensor/test/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TestAll "test-all")
set_tests_properties(TestAll PROPERTIES  _BACKTRACE_TRIPLES "/Users/bmleedy/PollutionSensor/gas_sensor/test/CMakeLists.txt;28;add_test;/Users/bmleedy/PollutionSensor/gas_sensor/test/CMakeLists.txt;0;")
subdirs("arduino_mock")
