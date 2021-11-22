/*
 * ******************************************************************************
 * Copyright (c) 2021 Robert Bosch GmbH.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU GENERAL PUBLIC LICENSE v3
 * which accompanies this distribution, and is available at
 * https://www.gnu.org/licenses/gpl-3.0.de.html
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 * *****************************************************************************
 */

#ifndef SRC_INC_HIERARCHICAL_NETWORK_INFO_GENERATION_HPP_
#define SRC_INC_HIERARCHICAL_NETWORK_INFO_GENERATION_HPP_

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/incSIM-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ptr.h"
#include <ns3/node-container.h>
#include <vector>
#include <fstream>

namespace ns3 {
    namespace ndn {
        namespace inc {

struct funcProperties{
    std::string name;
    std::string ram_mb;
    std::string rom_gb;
    std::string num_instructions;
    std::string cores;
    std::string exec_time;
    std::string lifetime;
    std::string func_size_KB;
    std::string result_size_KB;
    int num_inputs;
    std::string input_signature;
};
struct nodeProperties{
    std::string name;
    std::string lati;
    std::string longi;
    std::string ram_mb;
    std::string rom_gb;
    std::string clock_speed_mips;
    std::string cores;
    std::string queue;
};

struct dataProperties{
    std::string name;
    std::string data_size_KB;
    std::string freshness_s;
};

          class DataGen
          {
          public:
          DataGen(uint32_t seed, std::string fileToWrite);
          ~DataGen();

          void
          generate_data(uint32_t num_consumer_nodes, uint32_t t1_nodes, uint32_t t2_nodes, uint32_t t3_nodes, uint32_t cloud_scale ,uint32_t num_func, uint32_t num_data ,std::pair<int,int> input_range, std::pair<int, int>frequency_range, uint32_t sim_time, bool zipf, bool nfn_load_distribution_scenario, bool homogeneous_nodes);
          void
          generate_func_list(uint32_t num_func, std::pair<int, int> input_range);
          std::string
          generate_input_list(int num_inputs, uint32_t num_data);
          //void
          //generate_freq_list(uint32_t num_func, std::pair<int, int>frequency_range);
          void
          generate_provided_data(uint32_t num_data);

          void
          generate_start_time(uint32_t num_consumer_nodes, uint32_t sim_time);

          void
          generate_stop_time(uint32_t num_consumer_nodes, uint32_t sim_time);

          void
          generate_int_lifetime(uint32_t num_func, std::pair<int, int>frequency_range);

          private:

          std::vector<double>
          createZipfMandelbrotDist(uint32_t f_N, double f_q, double f_s);
          uint32_t
          getNextZipfMandelbrotRandomNumber(std::vector<double> dist_vector, uint32_t basis);

          std::string m_fileToWrite;
          std::vector<struct funcProperties> m_func_list;
          std::vector<struct nodeProperties> m_node_list;
          std::vector<std::string> m_freq;
          std::vector<struct dataProperties> m_provided_data;
          std::vector<std::pair<int,vector<std::string>>> m_consumer_provided_data;
          std::vector<std::string> m_startTime;
          std::vector<std::string> m_stopTime;
          uint32_t m_seed;
          Ptr<UniformRandomVariable> m_seqRNG;
          };
        }
    }
}

#endif
