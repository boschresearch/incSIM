# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
#
# Copyright (c) 2021 Robert Bosch GmbH.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

def build(bld):
    module = bld.create_ns3_module('incSIM', ['core','ndnSIM'])
    module.source = [
        'model/inc-orchestration-compute-node.cc',
        'utils/topology/inc-annotated-topology-reader.cpp',
        'utils/dataset_generation/hier-topology-data-generation.cpp',
        'utils/dataset_generation/hier-parser-node-info.cpp',
        'utils/tracers/inc-compute-tracer.cpp',
        'utils/tracers/inc-consumer-tracer.cpp',
        'utils/tracers/inc-orchestrator-tracer.cpp',
        'utils/tracers/inc-orchestrator-tracer-udp.cpp',
        'utils/tracers/inc-orchestrator-tracer-node.cpp',
        'utils/tracers/inc-app-delay-tracer.cpp',
        'utils/tracers/inc-compute-node-tracer.cpp',
        'apps/inc_compute_node_apps/inc_compute_app/inc-consumer-app.cpp',
        'apps/inc_compute_node_apps/inc_compute_app/inc-consumer-base-app.cpp',
        'apps/inc_compute_node_apps/inc_compute_app/inc-data-producer.cpp',
        'apps/inc_compute_node_apps/inc_compute_app/NFN-interest-resolution-engine.cpp',
        'apps/inc_compute_node_apps/inc_compute_app/NFN-producer-app.cpp',
        'apps/inc_functions_template/INC-Computation.cpp',
        'apps/inc_compute_node_apps/node_orchestration_udp_app/udp-orchestration-node-app.cpp',
        'apps/inc_compute_node_apps/node_orchestration_ndn_app/ndn-orchestration-node-app.cpp',
        'apps/orchestration/orchestration_communication_app/udp_orchestration_communication_app/udp-orchestration-helpers.cpp',
        'apps/orchestration/orchestration_communication_app/udp_orchestration_communication_app/udp-orchestration-communication-app.cpp',
        'apps/orchestration/orchestration_communication_app/ndn_orchestration_communication_app/ndn-orchestration-communication-app.cpp',
        'apps/orchestration/orchestration_communication_app/ndn_orchestration_communication_app/ndn-orchestration-helpers.cpp',
        'apps/orchestration/orchestration_communication_app/message_handler.cpp',
        'apps/orchestration/orchestration_management_app/orchestration-management-app.cpp',
        'apps/orchestration/storage/node-info-storage.cpp',
        ]

    module_test = bld.create_ns3_module_test_library('incSIM')
    module_test.source = [
        'test/ndn-inc-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'incSIM'
    headers.source = [
        'model/inc-orchestration-compute-node.h',
        'utils/topology/inc-annotated-topology-reader.hpp',
        'utils/dataset_generation/hier-topology-data-generation.hpp',
        'utils/dataset_generation/hier-parser-node-info.hpp',
        'utils/tracers/inc-compute-tracer.hpp',
        'utils/tracers/inc-consumer-tracer.hpp',
        'utils/tracers/inc-orchestrator-tracer.hpp',
        'utils/tracers/inc-orchestrator-tracer-udp.hpp',
        'utils/tracers/inc-orchestrator-tracer-node.hpp',
        'utils/tracers/inc-app-delay-tracer.hpp',
        'utils/tracers/inc-compute-node-tracer.hpp',
        'apps/inc_compute_node_apps/inc_compute_app/inc-consumer-app.hpp',
        'apps/inc_compute_node_apps/inc_compute_app/inc-consumer-base-app.hpp',
        'apps/inc_compute_node_apps/inc_compute_app/inc-data-producer.hpp',
        'apps/inc_compute_node_apps/inc_compute_app/NFN-interest-resolution-engine.hpp',
        'apps/inc_compute_node_apps/inc_compute_app/NFN-producer-app.hpp',
        'apps/inc_functions_template/INC-Computation.hpp',
        'apps/inc_compute_node_apps/node_orchestration_udp_app/udp-orchestration-node-app.hpp',
        'apps/inc_compute_node_apps/node_orchestration_ndn_app/ndn-orchestration-node-app.hpp',
        'apps/orchestration/orchestration_communication_app/udp_orchestration_communication_app/udp-orchestration-helpers.hpp',
        'apps/orchestration/orchestration_communication_app/udp_orchestration_communication_app/udp-orchestration-communication-app.hpp',
        'apps/orchestration/orchestration_communication_app/ndn_orchestration_communication_app/ndn-orchestration-communication-app.hpp',
        'apps/orchestration/orchestration_communication_app/ndn_orchestration_communication_app/ndn-orchestration-helpers.hpp',
        'apps/orchestration/orchestration_communication_app/message_handler.hpp',
        'apps/orchestration/orchestration_management_app/orchestration-management-app.hpp',
        'apps/orchestration/storage/node-info-storage.hpp'
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()
