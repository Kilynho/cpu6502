# CMake generated Testfile for 
# Source directory: /home/runner/work/cpu6502/cpu6502/tests
# Build directory: /home/runner/work/cpu6502/cpu6502/_codeql_build_dir/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(cpu6502_tests "/home/runner/work/cpu6502/cpu6502/_codeql_build_dir/runTests")
set_tests_properties(cpu6502_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/cpu6502/cpu6502/tests/CMakeLists.txt;41;add_test;/home/runner/work/cpu6502/cpu6502/tests/CMakeLists.txt;0;")
subdirs("../googletest")
