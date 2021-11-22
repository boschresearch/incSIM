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

#ifndef SRC_INC_PARSER_NODE_INFO_HPP_
#define SRC_INC_PARSER_NODE_INFO_HPP_

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
            struct consumer_node_struct {
                std::string Name;
                std::vector<std::string> function_prefix;
                std::vector<std::string> data_prefix_provided;
                std::vector<std::string> request_frequency;
                std::vector<std::string> start_time;
                std::vector<std::string> stop_time;
                std::vector<std::string> lifetime;
                std::vector<std::string> data_size;
                std::vector<std::string> freshness;
            };
             struct compute_node_struct{
                std::string Name;
                std::string node_prefix;
                std::vector<std::string> function_prefix;
                std::string func_size;
                std::string result_size;
            };

class parserNodeInfo
{
public:
parserNodeInfo(std::string fileToRead);
~parserNodeInfo();

std::vector<consumer_node_struct>
update_consumer_info(std::vector<Ptr<Node>> consumerNodes);

std::vector<compute_node_struct>
update_compute_info(std::vector<Ptr<ns3::ndn::inc::IncOrchestrationComputeNode>> computeNodes);

std::string
generate_func_first_prefix(std::string, std::string);

std::string
generate_data_first_prefix(std::string,std::string);

std::vector<std::string>
generate_data_prefix(std::string);

private:
std::vector<struct consumer_node_struct> m_consumer_nodes;
std::vector<struct compute_node_struct> m_compute_nodes;
std::string m_fileToRead;
};
        }
    }
}

#endif
