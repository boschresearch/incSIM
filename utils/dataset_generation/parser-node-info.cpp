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

#include "parser-node-info.hpp"

#include <fstream>
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/log.h"
#include <boost/algorithm/string.hpp>

NS_LOG_COMPONENT_DEFINE ("inc.parserNodeInfo");

namespace ns3 {
namespace ndn {
namespace inc {

parserNodeInfo::parserNodeInfo (std::string fileToRead)
{
  m_fileToRead = fileToRead;
};
parserNodeInfo::~parserNodeInfo (){};

std::vector<consumer_node_struct>
parserNodeInfo::update_consumer_info (std::vector<Ptr<Node>> consumerNodes)
{

  fstream fin;
  fin.open (m_fileToRead, ios::in);
  std::vector<std::vector<std::string>> file_info;
  std::string line, word;
  while (getline (fin, line))
    {
      if (line == "consumer section")
        break;
      else
        continue;
    }
  while (!fin.eof ())
    {
      string line;
      getline (fin, line);
      if (line == "")
        continue;
      if (line[0] == '#')
        continue; // comments
      if (line == "initial data section")
        break;
      std::vector<std::string> row_info;
      boost::split (row_info, line, boost::is_any_of ("\t"));
      std::vector<struct consumer_node_struct>::iterator it;
      for (it = m_consumer_nodes.begin (); it != m_consumer_nodes.end (); ++it)
        {
          if (it->Name == row_info.at (0))
            {
              std::string function_name = row_info.at(1);
              std::string input_arguments = row_info.at(2);
              it->function_prefix.push_back (
                  generate_func_first_prefix (function_name, input_arguments));
              it->request_frequency.push_back (row_info.at (3));
              it->lifetime.push_back (row_info.at (4));
              it->start_time.push_back (row_info.at (5));
              it->stop_time.push_back (row_info.at (6));
              break;
            }
        }
      if (it == m_consumer_nodes.end ())
        {
          consumer_node_struct new_node;
          new_node.Name = row_info.at (0);
          std::string function_name = row_info.at (1);
          std::string input_arguments = row_info.at (2);
          new_node.function_prefix.push_back (
              generate_func_first_prefix (function_name, input_arguments));
          new_node.request_frequency.push_back (row_info.at (3));
          new_node.lifetime.push_back (row_info.at (4));
          new_node.start_time.push_back (row_info.at (5));
          new_node.stop_time.push_back (row_info.at (6));
          m_consumer_nodes.push_back (new_node);
        }
    };
  while (!fin.eof ())
    {
      string line;
      getline (fin, line);
      if (line == "")
        continue;
      if (line[0] == '#')
        continue; // comments
      std::vector<std::string> row_info;
      boost::split (row_info, line, boost::is_any_of ("\t"));
      std::vector<struct consumer_node_struct>::iterator it;
      for (it = m_consumer_nodes.begin (); it != m_consumer_nodes.end (); ++it)
        {
          if (it->Name == row_info.at (0))
            {
              std::vector<std::string> data_prefix_list = generate_data_prefix (row_info.at (1));
              for (auto itr = data_prefix_list.begin (); itr != data_prefix_list.end (); ++itr)
                it->data_prefix_provided.push_back (*itr);
              it->data_size.push_back (row_info.at (2));
              it->freshness.push_back (row_info.at (3));
              break;
            }
        }
      if (it == m_consumer_nodes.end ())
        {
          consumer_node_struct new_node;
          new_node.Name = row_info.at (0);
          std::vector<std::string> data_prefix_list = generate_data_prefix (row_info.at (1));
          for (auto itr = data_prefix_list.begin (); itr != data_prefix_list.end (); ++itr)
            new_node.data_prefix_provided.push_back (*itr);
          new_node.data_size.push_back (row_info.at (2));
          new_node.freshness.push_back (row_info.at (3));
          m_consumer_nodes.push_back (new_node);
        }
    };
  fin.close ();
  return m_consumer_nodes;
}

std::vector<compute_node_struct>
parserNodeInfo::update_compute_info (std::vector<Ptr<IncOrchestrationComputeNode>> computeNodes)
{
  compute_node_struct orchestrator;
  orchestrator.Name = computeNodes.at (0)->GetName ();
  orchestrator.node_prefix = "/Orchestrator";
  m_compute_nodes.push_back (orchestrator);

  for (uint32_t i = 1; i < computeNodes.size (); i++)
    {
      compute_node_struct new_node;
      std::string name = "/Orchestrator/";
      new_node.Name = computeNodes.at (i)->GetName ();
      new_node.node_prefix = (name.append (computeNodes.at (i)->GetName ()));
      m_compute_nodes.push_back (new_node);
    }

  return m_compute_nodes;
}

std::string
parserNodeInfo::generate_func_first_prefix (std::string func_name, std::string inputs_list)
{
  std::string func_prefix;
  func_prefix.append ("/lambda/Function/" + func_name);
  std::vector<std::string> inputs;
  boost::split (inputs, inputs_list, boost::is_any_of (","));
  for (auto itr = inputs.begin (); itr != inputs.end (); ++itr)
    {
      if (*itr == "null")
        {
          return func_prefix;
        }
      else if (itr == inputs.begin ())
        {
          func_prefix.append ("/Data");
        }
      func_prefix.append ("/" + *itr);
    }
  NS_LOG_INFO("Function prefix is "<<func_prefix);
  return func_prefix;
}

std::string
parserNodeInfo::generate_data_first_prefix (std::string func_name, std::string inputs_list)
{
  std::string data_first_prefix;
  data_first_prefix.append ("/lambda");
  std::vector<std::string> inputs;
  boost::split (inputs, inputs_list, boost::is_any_of (","));
  for (auto itr = inputs.begin (); itr != inputs.end () - 1; ++itr)
    {
      if (*itr == "null")
        {
          data_first_prefix.append ("/Function/" + func_name);
          return data_first_prefix;
        }
      else if (itr == inputs.begin ())
        {
          data_first_prefix.append ("/Data");
        }
      data_first_prefix.append ("/" + *itr);
    }
  data_first_prefix.append ("/Function/" + func_name);
  return data_first_prefix;
}

std::vector<std::string>
parserNodeInfo::generate_data_prefix (std::string data_list)
{
  std::vector<std::string> data_prefix_list;
  std::vector<std::string> input_data;
  boost::split (input_data, data_list, boost::is_any_of (","));
  for (auto itr = input_data.begin (); itr != input_data.end () - 1; ++itr)
    {
      std::string data_prefix = "/Data";
      data_prefix.append ("/" + *itr);
      data_prefix_list.push_back (data_prefix);
    }
  return data_prefix_list;
}
} //namespace inc
} //namespace ndn
} //namespace ns3
