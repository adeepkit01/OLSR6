# Steps to test OLSR model for IPv6

Step 1: Install ns-3.25 (Clone it from: http://code.nsnam.org/ns-3.25)

Step 2: Copy "olsr6" directory to src directory

Step 3: Recompile ns-3

Step 4: Run programs given in the example directory to reproduce the results.

Step 5: Run "./test.py" to run unit tests

Details about the module is as follows:

.
|-- examples
|   |-- olsr6-hna.cc
|   |-- simple-point-to-point-olsr6.cc
|   `-- wscript
|-- helper
|   |-- olsr6-helper.cc
|   `-- olsr6-helper.h
|-- model
|   |-- olsr6-header.cc
|   |-- olsr6-header.h
|   |-- olsr6-repositories.h
|   |-- olsr6-routing-protocol.cc
|   |-- olsr6-routing-protocol.h
|   |-- olsr6-state.cc
|   `-- olsr6-state.h
|-- test
|   |-- examples-to-run.py
|   |-- hello-regression-test.cc
|   |-- hello-regression-test.h
|   |-- olsr6-header-test-suite.cc
|   |-- olsr6-routing-protocol-test-suite.cc
|   |-- olsr6-test-suite.cc
|   |-- regression-test-suite.cc
|   |-- tc-regression-test.cc
|   `-- tc-regression-test.h
`-- wscript


4 directories, 22 files
