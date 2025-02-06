# CMake generated Testfile for 
# Source directory: /home/kilynho/src/cpu6502
# Build directory: /home/kilynho/src/cpu6502/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(runTests "/home/kilynho/src/cpu6502/build/runTests")
set_tests_properties(runTests PROPERTIES  _BACKTRACE_TRIPLES "/home/kilynho/src/cpu6502/CMakeLists.txt;33;add_test;/home/kilynho/src/cpu6502/CMakeLists.txt;0;")
subdirs("lib/googletest")
