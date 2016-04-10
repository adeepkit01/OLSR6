## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('olsr6', ['internet'])
    module.includes = '.'
    module.source = [
        'model/olsr6-header.cc',
        'model/olsr6-state.cc',
        'model/olsr6-routing-protocol.cc',
        'helper/olsr6-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('olsr6')
    module_test.source = [
        'test/hello-regression-test.cc',
        'test/olsr6-header-test-suite.cc',
        'test/regression-test-suite.cc',
        'test/olsr6-routing-protocol-test-suite.cc',
        'test/tc-regression-test.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'olsr6'
    headers.source = [
        'model/olsr6-routing-protocol.h',
        'model/olsr6-header.h',
        'model/olsr6-state.h',
        'model/olsr6-repositories.h',
        'helper/olsr6-helper.h',
        ]


    if bld.env['ENABLE_EXAMPLES']:
        bld.recurse('examples')

    #bld.ns3_python_bindings()
