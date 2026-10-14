# CMake generated Testfile for 
# Source directory: /Users/ernest.molczan/CLionProjects/Calculator
# Build directory: /Users/ernest.molczan/CLionProjects/Calculator/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[arithmetic]=] "/Users/ernest.molczan/CLionProjects/Calculator/build/calculator_tests")
set_tests_properties([=[arithmetic]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/ernest.molczan/CLionProjects/Calculator/CMakeLists.txt;25;add_test;/Users/ernest.molczan/CLionProjects/Calculator/CMakeLists.txt;0;")
add_test([=[file_io]=] "/opt/homebrew/bin/cmake" "-DCALCULATOR=/Users/ernest.molczan/CLionProjects/Calculator/build/Calculator" "-DTEST_DIRECTORY=/Users/ernest.molczan/CLionProjects/Calculator/build/file-tests" "-P" "/Users/ernest.molczan/CLionProjects/Calculator/tests/test_files.cmake")
set_tests_properties([=[file_io]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/ernest.molczan/CLionProjects/Calculator/CMakeLists.txt;26;add_test;/Users/ernest.molczan/CLionProjects/Calculator/CMakeLists.txt;0;")
