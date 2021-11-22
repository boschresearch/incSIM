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
 * Based on the code by regents of ndnSIM
 *     https://github.com/named-data-ndnSIM/ndnSIM/blob/master/utils/topology/annotated-topology-reader.cpp
 *     Based on the code by Hajime Tazaki <tazaki@sfc.wide.ad.jp>
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 *          Liming Liu <fixed-term.liming.liu@de.bosch.com>
 * *****************************************************************************
 */

#include "ns3/nstime.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/names.h"
#include "ns3/net-device-container.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-address.h"
#include "ns3/error-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/double.h"

#include "ns3/ndnSIM-module.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include "ns3/incSIM-module.h"
#include <set>

#ifdef NS3_MPI
#include <ns3/mpi-interface.h>
#endif

using namespace std;
namespace ns3 {
namespace ndn {
namespace inc {
NS_LOG_COMPONENT_DEFINE ("IncNdnAnnotatedTopologyReader");

IncNdnAnnotatedTopologyReader::IncNdnAnnotatedTopologyReader (const std::string &path,
                                                              double scale /*=1.0*/)
    : m_path (path),
      m_randX (CreateObject<UniformRandomVariable> ()),
      m_randY (CreateObject<UniformRandomVariable> ()),
      m_scale (scale),
      m_requiredPartitions (1)
{
  NS_LOG_FUNCTION (this);

  m_randX->SetAttribute ("Min", DoubleValue (0));
  m_randX->SetAttribute ("Max", DoubleValue (100.0));

  m_randY->SetAttribute ("Min", DoubleValue (0));
  m_randY->SetAttribute ("Max", DoubleValue (100.0));

  SetMobilityModel ("ns3::ConstantPositionMobilityModel");
}

void
IncNdnAnnotatedTopologyReader::SetBoundingBox (double ulx, double uly, double lrx, double lry)
{
  NS_LOG_FUNCTION (this << ulx << uly << lrx << lry);

  m_randX->SetAttribute ("Min", DoubleValue (ulx));
  m_randX->SetAttribute ("Max", DoubleValue (lrx));

  m_randY->SetAttribute ("Min", DoubleValue (uly));
  m_randY->SetAttribute ("Max", DoubleValue (lry));
}

void
IncNdnAnnotatedTopologyReader::SetMobilityModel (const std::string &model)
{
  NS_LOG_FUNCTION (this << model);
  m_mobilityFactory.SetTypeId (model);
}

IncNdnAnnotatedTopologyReader::~IncNdnAnnotatedTopologyReader ()
{
  NS_LOG_FUNCTION (this);
}

void
IncNdnAnnotatedTopologyReader::AddComputeNode (Ptr<IncOrchestrationComputeNode> value)
{
  m_inc_compute_nodes.push_back (value);
}

void
IncNdnAnnotatedTopologyReader::AddConsumerNode (Ptr<Node> value)
{
  m_inc_consumer_nodes.push_back (value);
}

std::vector<Ptr<INC_Computation>>
IncNdnAnnotatedTopologyReader::GetFunctionList ()
{
  return this->m_function_list;
}

std::vector<Ptr<IncOrchestrationComputeNode>>
IncNdnAnnotatedTopologyReader::GetComputeNodes ()
{
  return m_inc_compute_nodes;
}

std::vector<Ptr<Node>>
IncNdnAnnotatedTopologyReader::GetConsumerNodes ()
{
  return m_inc_consumer_nodes;
}

NodeContainer
IncNdnAnnotatedTopologyReader::GetComputeNodeContainer () const
{
  return m_compute_nodecontainer;
}

NodeContainer
IncNdnAnnotatedTopologyReader::GetConsumerNodeContainer () const
{
  return m_consumer_nodecontainer;
}

Ptr<IncOrchestrationComputeNode>
IncNdnAnnotatedTopologyReader::CreateIncComputeNode (const std::string name, uint32_t systemId)
{
  NS_LOG_DEBUG (this << name);
  m_requiredPartitions = std::max (m_requiredPartitions, systemId + 1);

  IncOrchestrationComputeNode node = IncOrchestrationComputeNode (systemId);

  Ptr<IncOrchestrationComputeNode> nodePtr = CreateObject<IncOrchestrationComputeNode> (node);

  Names::Add (m_path, name, node.GetNode ());
  m_compute_nodecontainer.Add (node.GetNode ());
  AddComputeNode (nodePtr);

  return nodePtr;
}

Ptr<IncOrchestrationComputeNode>
IncNdnAnnotatedTopologyReader::CreateIncComputeNode (const std::string name, double posX,
                                                     double posY, uint32_t systemId)
{
  NS_LOG_DEBUG (this << name << posX << posY);
  m_requiredPartitions = std::max (m_requiredPartitions, systemId + 1);

  IncOrchestrationComputeNode node = IncOrchestrationComputeNode (systemId);
  Ptr<MobilityModel> loc = DynamicCast<MobilityModel> (m_mobilityFactory.Create ());
  node.GetNode ()->AggregateObject (loc);

  loc->SetPosition (Vector (posX, posY, 0));

  Ptr<IncOrchestrationComputeNode> nodePtr = CreateObject<IncOrchestrationComputeNode> (node);

  Names::Add (m_path, name, node.GetNode ());
  m_compute_nodecontainer.Add (node.GetNode ());
  AddComputeNode (nodePtr);

  return nodePtr;
}

Ptr<Node>
IncNdnAnnotatedTopologyReader::CreateNode (const std::string name, uint32_t systemId)
{
  NS_LOG_FUNCTION (this << name);
  m_requiredPartitions = std::max (m_requiredPartitions, systemId + 1);

  Ptr<Node> node = CreateObject<Node> (systemId);

  Names::Add (m_path, name, node);
  m_consumer_nodecontainer.Add (node);

  return node;
}

Ptr<Node>
IncNdnAnnotatedTopologyReader::CreateNode (const std::string name, double posX, double posY,
                                           uint32_t systemId)
{
  NS_LOG_FUNCTION (this << name << posX << posY);
  m_requiredPartitions = std::max (m_requiredPartitions, systemId + 1);

  Ptr<Node> node = CreateObject<Node> (systemId);
  Ptr<MobilityModel> loc = DynamicCast<MobilityModel> (m_mobilityFactory.Create ());
  node->AggregateObject (loc);

  loc->SetPosition (Vector (posX, posY, 0));

  Names::Add (m_path, name, node);
  m_consumer_nodecontainer.Add (node);

  return node;
}

const std::list<TopologyReader::Link> &
IncNdnAnnotatedTopologyReader::GetLinks () const
{
  return m_linksList;
}

std::unordered_map<std::string, std::vector<std::string>>
IncNdnAnnotatedTopologyReader::GetInitialFunctionStatusMap ()
{
  return m_initial_func_status;
}

std::pair<NodeContainer, NodeContainer>
IncNdnAnnotatedTopologyReader::ReadTopology (void)
{

  ifstream topgen;
  topgen.open (GetFileName ().c_str ());
  string curSection;

  if (!topgen.is_open () || !topgen.good ())
    {
      NS_FATAL_ERROR ("Cannot open file " << GetFileName () << " for reading");
      return std::make_pair (m_compute_nodecontainer, m_consumer_nodecontainer);
    }

  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);

      if (line == "router")
        {
          curSection = "router";
          break;
        }
    }

  if (topgen.eof ())
    {
      NS_FATAL_ERROR ("Topology file " << GetFileName () << " does not have \"router\" section");
      return std::make_pair (m_compute_nodecontainer, m_consumer_nodecontainer);
    }

  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);
      if (line[0] == '#')
        continue; // comments
      if (line == "links")
        {
          curSection = "link";
          break; // stop reading nodes
        }

      istringstream lineBuffer (line);
      std::string name;
      double latitude = 0, longitude = 0;
      uint32_t systemId = 0;
      uint32_t processor_core = 0, RAM = 0, ROM = 0;
      uint32_t processor_speed = 0;
      uint32_t queue = 0;
      string runtimes = "", links = "";

      lineBuffer >> name >> latitude >> longitude >> processor_core >> processor_speed >> RAM >>
          ROM >> queue >> links >> runtimes;

      if (name.empty ())
        continue;

      if (boost::contains (name, "orchestrator") || boost::contains (name, "compute"))
        {
          Ptr<IncOrchestrationComputeNode> node;
          if (abs (latitude) > 0.001 && abs (latitude) > 0.001)
            node = CreateIncComputeNode (name, m_scale * longitude, -m_scale * latitude, systemId);
          else
            {
              Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
              node = CreateIncComputeNode (name, var->GetValue (0, 200), var->GetValue (0, 200),
                                           systemId);
            }

          node->SetName (name);
          node->SetProcessorCore (processor_core);
          node->SetProcessorClockSpeed (processor_speed);
          node->SetRam (RAM);
          node->SetRom (ROM);
          node->SetLinks (links);
          node->SetSupportedRuntimes (runtimes);
          node->SetQueueSize(queue);
        }
      else if (boost::contains (name, "consumer"))
        {
          Ptr<Node> node;
          //std::cout<<"Node name is "<<name<<std::endl;
          if (abs (latitude) > 0.001 && abs (latitude) > 0.001)
            node = CreateNode (name, m_scale * longitude, -m_scale * latitude, systemId);
          else
            {
              Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
              node = CreateNode (name, var->GetValue (0, 200), var->GetValue (0, 200), systemId);
            }
        }
    }

  map<string, set<string>> processedLinks; // to eliminate duplications

  if (topgen.eof ())
    {
      NS_LOG_ERROR ("Topology file " << GetFileName () << " does not have \"link\" section");
      return std::make_pair (m_compute_nodecontainer, m_consumer_nodecontainer);
    }

  // SeekToSection ("link");
  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);
      if (line == "")
        continue;
      if (line[0] == '#')
        continue; // comments
      if (line == "functions" || line == "initial function status" || line == "consumer section")
        {
          curSection = line;
          break; // stop reading nodes
        }

      // NS_LOG_DEBUG ("Input: [" << line << "]");
      istringstream lineBuffer (line);
      string from, to, capacity, metric, delay, maxPackets, lossRate;

      lineBuffer >> from >> to >> capacity >> metric >> delay >> maxPackets >> lossRate;

      if (processedLinks[to].size () != 0 &&
          processedLinks[to].find (from) != processedLinks[to].end ())
        {
          continue; // duplicated link
        }
      processedLinks[from].insert (to);

      Ptr<Node> fromNode = Names::Find<Node> (m_path, from);
      NS_ASSERT_MSG (fromNode != 0, from << " node not found");
      Ptr<Node> toNode = Names::Find<Node> (m_path, to);
      NS_ASSERT_MSG (toNode != 0, to << " node not found");

      Link link (fromNode, from, toNode, to);

      link.SetAttribute ("DataRate", capacity);
      link.SetAttribute ("OSPF", metric);

      if (!delay.empty ())
        link.SetAttribute ("Delay", delay);
      if (!maxPackets.empty ())
        link.SetAttribute ("MaxPackets", maxPackets);

      // Saran Added lossRate
      if (!lossRate.empty ())
        link.SetAttribute ("LossRate", lossRate);

      AddLink (link);
      NS_LOG_DEBUG ("New link " << from << " <==> " << to << " / " << capacity << " with " << metric
                                << " metric (" << delay << ", " << maxPackets << ", " << lossRate
                                << ")");
    }

  if (topgen.eof ())
    {
      NS_LOG_ERROR ("Topology file " << GetFileName ()
                                     << " does not have \"initial function status\" section");
      return std::make_pair (m_compute_nodecontainer, m_consumer_nodecontainer);
    }

  // SeekToSection ("functions");
  while (!topgen.eof ())
    {
      if (curSection == "initial function status")
        {
          break;
        }
      string line;
      getline (topgen, line);
      if (line == "")
        continue;
      if (line[0] == '#')
        continue; // comments
      if (line == "initial function status")
        {
          curSection == "initial function status";
          break; // stop reading functions
        }

      // NS_LOG_DEBUG ("Input: [" << line << "]");

      istringstream lineBuffer (line);
      string name, inputListString, rumtimesRequiredString;
      uint32_t ram, rom, cpu, funcSize, resultSize, num_inputs, NumInstructions;
      double execTime;

      lineBuffer >> name >> inputListString >> rumtimesRequiredString >> ram >> rom >> cpu >> execTime >>
          NumInstructions >> funcSize >> resultSize;

      std::vector<std::string> rumtimesRequired;
      std::string delim = ";";
      auto start = 0U;
      auto end = rumtimesRequiredString.find (delim);

      while (end != std::string::npos)
        {
          rumtimesRequired.push_back (rumtimesRequiredString.substr (start, end - start));
          start = end + delim.length ();
          end = rumtimesRequiredString.find (delim, start);
        }

      if(!(inputListString.compare("null")==0))
      {
        std::vector<std::string> signature;
        boost::split(signature, inputListString, boost::is_any_of(","));
        num_inputs = signature.size();
      }
      else
      {
        num_inputs = 0;
      }


      Ptr<INC_Computation> t_func = CreateObject<INC_Computation> ();
      t_func->setName (name);
      t_func->SetCpu (cpu);
      t_func->SetRam (ram);
      t_func->SetRom (rom);
      t_func->SetExecTime (execTime);
      t_func->SetNumInstructions (NumInstructions);
      t_func->SetFuncSize (funcSize);
      t_func->SetResultSize (resultSize);
      t_func->SetRuntimeEnvironment (rumtimesRequired);
      t_func->SetParamNumber(num_inputs);
      m_function_list.push_back (t_func);
      NS_LOG_DEBUG ("New function "
                    << name << " cpu=" << cpu << " ram=" << ram << " rom=" << rom
                    << " execution_time=" << execTime << "input_params=" << inputListString
                    << "function size=" << funcSize << "result size=" << resultSize
                    << " runtime_environment=" << rumtimesRequiredString << std::endl);
    }

  if (topgen.eof ())
    {
      NS_LOG_ERROR ("Topology file " << GetFileName ()
                                     << " does not have \"initial function status\" section");
      return std::make_pair (m_compute_nodecontainer, m_consumer_nodecontainer);
    }

  // SeekToSection ("initial function status");
  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);
      if (line == "")
        continue;
      if (line[0] == '#')
        continue; // comments
      if (line == "consumer section")
        {
          curSection = "consumer section";
          break;
        }

      istringstream lineBuffer (line);
      string node_name, func_name;

      lineBuffer >> node_name >> func_name;

      if (m_initial_func_status.find (node_name) == m_initial_func_status.end ())
        {
          vector<string> t_vector;
          t_vector.push_back (func_name);
          m_initial_func_status.emplace (node_name, t_vector);
        }
      else
        {
          m_initial_func_status[node_name].push_back (func_name);
        }
      }

  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);
      if (line == "")
        continue;
      if (line[0] == '#')
        continue; // comments
      if (line == "initial data section")
        {
          curSection = "initial data section";
          break;
        }

      istringstream lineBuffer (line);
      string node_name, func_name, InputParamList;
      lineBuffer >> node_name >> func_name >> InputParamList;

      NS_LOG_DEBUG("InputParamList is "<<InputParamList);
      std::vector<std::string> inputs;
      if (!(InputParamList.compare ("null") == 0))
        {
          boost::split (inputs, InputParamList, boost::is_any_of (","));
        }
        else
        {
          inputs.clear();
        }

      for (auto itr = m_function_list.begin (); itr != m_function_list.end (); itr++)
        {
          if ((*itr)->getName ().compare(func_name)==0)
            {
              if (!(InputParamList.compare ("null") == 0))
                {
                  NS_LOG_DEBUG("inputs.size() is "<<inputs.size());
                  for (int i = 0; i < inputs.size (); i++)
                    (*itr)->AddToInputList(inputs.at (i));
                }
            }
        }
    }
        //   add this new function to compute nodes
    for (uint32_t i = 1; i < m_inc_compute_nodes.size (); i++)
        for (auto itr = m_function_list.begin (); itr != m_function_list.end (); ++itr)
          {
            m_inc_compute_nodes.at (i)->AddNewFunction (((*itr)->getName ()).toUri (), *itr);
          }


  //do log
  NS_LOG_INFO ("Annotated topology created with "
               << (m_compute_nodecontainer.GetN () + m_consumer_nodecontainer.GetN ())
               << " nodes and " << LinksSize () << " links and " << m_initial_func_status.size ()
               << " function initial status setting");
  topgen.close ();
  ApplySettings ();
  return std::make_pair (m_compute_nodecontainer, m_consumer_nodecontainer);
}

void
IncNdnAnnotatedTopologyReader::AssignIpv4Addresses (Ipv4Address base)
{
  Ipv4AddressHelper address (base, Ipv4Mask ("/24"));

  BOOST_FOREACH (const Link &link, m_linksList)
    {
      address.Assign (NetDeviceContainer (link.GetFromNetDevice (), link.GetToNetDevice ()));

      base = Ipv4Address (base.Get () + 256);
      address.SetBase (base, Ipv4Mask ("/24"));
    }
}

NodeContainer
IncNdnAnnotatedTopologyReader::Read (void)
{
  ifstream topgen;
  topgen.open (GetFileName ().c_str ());
  string curSection;

  if (!topgen.is_open () || !topgen.good ())
    {
      NS_FATAL_ERROR ("Cannot open file " << GetFileName () << " for reading");
      return m_nodes;
    }

  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);

      if (line == "router")
        {
          curSection = "router";
          break;
        }
    }

  if (topgen.eof ())
    {
      NS_FATAL_ERROR ("Topology file " << GetFileName () << " does not have \"router\" section");
      return m_nodes;
    }

  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);
      if (line[0] == '#')
        continue; // comments
      if (line == "link")
        {
          curSection = "link";
          break; // stop reading nodes
        }

      istringstream lineBuffer (line);
      string name, city;
      double latitude = 0, longitude = 0;
      uint32_t systemId = 0;
      uint32_t processor_speed = 0;
      uint32_t processor_core = 0, RAM = 0, ROM = 0, UUID = 0;
      string links = "", runtimes = "";

      lineBuffer >> name >> city >> latitude >> longitude >> processor_core >> processor_speed >>
          RAM >> ROM >> links >> UUID >> runtimes;

      if (name.empty ())
        continue;

      Ptr<IncOrchestrationComputeNode> node;

      if (abs (latitude) > 0.001 && abs (latitude) > 0.001)
        node = CreateIncComputeNode (name, m_scale * longitude, -m_scale * latitude, systemId);
      else
        {
          Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
          node =
              CreateIncComputeNode (name, var->GetValue (0, 200), var->GetValue (0, 200), systemId);
        }

      node->SetName (name);
      node->SetProcessorCore (processor_core);
      node->SetProcessorClockSpeed (processor_speed);
      node->SetRam (RAM);
      node->SetRom (ROM);
      node->SetUUID (UUID);
      node->SetLinks (links);
      node->SetSupportedRuntimes (runtimes);
    }

  map<string, set<string>> processedLinks; // to eliminate duplications

  if (topgen.eof ())
    {
      NS_LOG_ERROR ("Topology file " << GetFileName () << " does not have \"link\" section");
      return m_nodes;
    }

  // SeekToSection ("link");
  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);
      if (line == "")
        continue;
      if (line[0] == '#')
        continue; // comments
      if (line == "functions" || line == "initial function status")
        {
          curSection = line;
          break; // stop reading nodes
        }

      // NS_LOG_DEBUG ("Input: [" << line << "]");

      istringstream lineBuffer (line);
      string from, to, capacity, metric, delay, maxPackets, lossRate;

      lineBuffer >> from >> to >> capacity >> metric >> delay >> maxPackets >> lossRate;

      if (processedLinks[to].size () != 0 &&
          processedLinks[to].find (from) != processedLinks[to].end ())
        {
          continue; // duplicated link
        }
      processedLinks[from].insert (to);

      Ptr<Node> fromNode = Names::Find<Node> (m_path, from);
      NS_ASSERT_MSG (fromNode != 0, from << " node not found");
      Ptr<Node> toNode = Names::Find<Node> (m_path, to);
      NS_ASSERT_MSG (toNode != 0, to << " node not found");

      Link link (fromNode, from, toNode, to);

      link.SetAttribute ("DataRate", capacity);
      link.SetAttribute ("OSPF", metric);

      if (!delay.empty ())
        link.SetAttribute ("Delay", delay);
      if (!maxPackets.empty ())
        link.SetAttribute ("MaxPackets", maxPackets);

      // Saran Added lossRate
      if (!lossRate.empty ())
        link.SetAttribute ("LossRate", lossRate);

      AddLink (link);
      NS_LOG_DEBUG ("New link " << from << " <==> " << to << " / " << capacity << " with " << metric
                                << " metric (" << delay << ", " << maxPackets << ", " << lossRate
                                << ")");
    }

  if (topgen.eof ())
    {
      NS_LOG_ERROR ("Topology file " << GetFileName ()
                                     << " does not have \"initial function status\" section");
      return m_nodes;
    }

  // SeekToSection ("functions");
  while (!topgen.eof ())
    {
      if (curSection == "initial function status")
        {
          break;
        }
      string line;
      getline (topgen, line);
      if (line == "")
        continue;
      if (line[0] == '#')
        continue; // comments
      if (line == "initial function status")
        {
          curSection == "initial function status";
          break; // stop reading functions
        }

      // NS_LOG_DEBUG ("Input: [" << line << "]");

      istringstream lineBuffer (line);
      string name, inputListString, rumtimesRequiredString;
      u_int32_t ram, rom, cpu;
      double execTime;

      lineBuffer >> name >> inputListString >> rumtimesRequiredString >> ram >> rom >> cpu >>
          execTime;

      std::vector<std::string> rumtimesRequired;

      std::string delim = ";";

      auto start = 0U;
      auto end = rumtimesRequiredString.find (delim);

      while (end != std::string::npos)
        {
          rumtimesRequired.push_back (rumtimesRequiredString.substr (start, end - start));
          start = end + delim.length ();
          end = rumtimesRequiredString.find (delim, start);
        }
      /*
    std::vector<std::string> inputParamList;

    if(inputListString!="null"){
    	start = 0U;
		end = inputListString.find(delim);

		while (end != std::string::npos)
		{
			inputParamList.push_back(inputListString.substr(start, end - start));
			start = end + delim.length();
			end = inputListString.find(delim, start);
		}
    } */

      Ptr<INC_Computation> t_func = CreateObject<INC_Computation> ();
      t_func->setName (name);
      t_func->SetCpu (cpu);
      t_func->SetRam (ram);
      t_func->SetRom (rom);
      t_func->SetExecTime (execTime);
      t_func->SetRuntimeEnvironment (rumtimesRequired);
      //t_func->SetInputList(inputParamList);
      m_function_list.push_back (t_func);
      NS_LOG_DEBUG ("New function "
                    << name << " cpu=" << cpu << " ram=" << ram << " rom=" << rom
                    << " execution_time=" << rom << "input_params=" << inputListString
                    << " runtime_environment=" << rumtimesRequiredString << std::endl);
      //   add this new function to compute nodes
      for (int i = 0; i < m_inc_compute_nodes.size (); i++)
        {
          m_inc_compute_nodes.at (i)->AddNewFunction (name, t_func);
          NS_LOG_DEBUG ("Add function " << name << " to " << m_inc_compute_nodes.at (i)->GetName ()
                                        << std::endl);
        }
    }

  if (topgen.eof ())
    {
      NS_LOG_ERROR ("Topology file " << GetFileName ()
                                     << " does not have \"initial function status\" section");
      return m_nodes;
    }

  // SeekToSection ("initial function status");
  while (!topgen.eof ())
    {
      string line;
      getline (topgen, line);
      if (line == "")
        continue;
      if (line[0] == '#')
        continue; // comments

      istringstream lineBuffer (line);
      string node_name, func_name;

      lineBuffer >> node_name >> func_name;

      if (m_initial_func_status.find (node_name) == m_initial_func_status.end ())
        {
          vector<string> t_vector;
          t_vector.push_back (func_name);
          m_initial_func_status.emplace (node_name, t_vector);
        }
      else
        {
          m_initial_func_status[node_name].push_back (func_name);
        }

      NS_LOG_INFO ("New initial function status on node " << node_name << " of " << func_name);
    }

  //do log
  NS_LOG_INFO ("Annotated topology created with "
               << m_nodes.GetN () << " nodes and " << LinksSize () << " links and "
               << m_initial_func_status.size () << " function initial status setting");
  topgen.close ();
  ApplySettings ();
  return m_nodes;
}

void
IncNdnAnnotatedTopologyReader::ApplyOspfMetric ()
{
  BOOST_FOREACH (const Link &link, m_linksList)
    {
      NS_LOG_DEBUG ("OSPF: " << link.GetAttribute ("OSPF"));
      uint16_t metric = boost::lexical_cast<uint16_t> (link.GetAttribute ("OSPF"));

      {
        Ptr<Ipv4> ipv4 = link.GetFromNode ()->GetObject<Ipv4> ();
        if (ipv4 != 0)
          {
            int32_t interfaceId = ipv4->GetInterfaceForDevice (link.GetFromNetDevice ());
            NS_ASSERT (interfaceId >= 0);

            ipv4->SetMetric (interfaceId, metric);
          }

        Ptr<ndn::L3Protocol> ndn = link.GetFromNode ()->GetObject<ndn::L3Protocol> ();
        if (ndn != 0)
          {
            shared_ptr<ndn::Face> face = ndn->getFaceByNetDevice (link.GetFromNetDevice ());
            NS_ASSERT (face != 0);

            face->setMetric (metric);
          }
      }

      {
        Ptr<Ipv4> ipv4 = link.GetToNode ()->GetObject<Ipv4> ();
        if (ipv4 != 0)
          {
            int32_t interfaceId = ipv4->GetInterfaceForDevice (link.GetToNetDevice ());
            NS_ASSERT (interfaceId >= 0);

            ipv4->SetMetric (interfaceId, metric);
          }

        Ptr<ndn::L3Protocol> ndn = link.GetToNode ()->GetObject<ndn::L3Protocol> ();
        if (ndn != 0)
          {
            shared_ptr<ndn::Face> face = ndn->getFaceByNetDevice (link.GetToNetDevice ());
            NS_ASSERT (face != 0);

            face->setMetric (metric);
          }
      }
    }
}

void
IncNdnAnnotatedTopologyReader::ApplySettings ()
{
#ifdef NS3_MPI
  if (MpiInterface::IsEnabled () && MpiInterface::GetSize () != m_requiredPartitions)
    {
      std::cerr << "MPI interface is enabled, but number of partitions ("
                << MpiInterface::GetSize ()
                << ") is not equal to number of partitions in the topology ("
                << m_requiredPartitions << ")";
      exit (-1);
    }
#endif

  PointToPointHelper p2p;

  BOOST_FOREACH (Link &link, m_linksList)
    {
      // cout << "Link: " << Findlink.GetFromNode () << ", " << link.GetToNode () << endl;
      string tmp;

      ////////////////////////////////////////////////
      if (link.GetAttributeFailSafe ("MaxPackets", tmp))
        {
          NS_LOG_INFO ("MaxPackets = " + link.GetAttribute ("MaxPackets"));

          try
            {
              std::string maxPackets = link.GetAttribute ("MaxPackets");

              // compatibility mode. Only DropTailQueue is supported
              p2p.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize",
                            StringValue (maxPackets + "p"));
          } catch (...)
            {
              typedef boost::tokenizer<boost::escaped_list_separator<char>> tokenizer;
              std::string value = link.GetAttribute ("MaxPackets");
              tokenizer tok (value);

              tokenizer::iterator token = tok.begin ();
              p2p.SetQueue (*token);

              for (token++; token != tok.end (); token++)
                {
                  boost::escaped_list_separator<char> separator ('\\', '=', '\"');
                  tokenizer attributeTok (*token, separator);

                  tokenizer::iterator attributeToken = attributeTok.begin ();

                  string attribute = *attributeToken;
                  attributeToken++;

                  if (attributeToken == attributeTok.end ())
                    {
                      NS_LOG_ERROR ("Queue attribute ["
                                    << *token << "] should be in form <Attribute>=<Value>");
                      continue;
                    }

                  string value = *attributeToken;

                  p2p.SetQueueAttribute (attribute, StringValue (value));
                }
          }
        }

      if (link.GetAttributeFailSafe ("DataRate", tmp))
        {
          NS_LOG_INFO ("DataRate = " + link.GetAttribute ("DataRate"));
          p2p.SetDeviceAttribute ("DataRate", StringValue (link.GetAttribute ("DataRate")));
        }

      if (link.GetAttributeFailSafe ("Delay", tmp))
        {
          NS_LOG_INFO ("Delay = " + link.GetAttribute ("Delay"));
          p2p.SetChannelAttribute ("Delay", StringValue (link.GetAttribute ("Delay")));
        }

      NetDeviceContainer nd = p2p.Install (link.GetFromNode (), link.GetToNode ());
      link.SetNetDevices (nd.Get (0), nd.Get (1));

      ////////////////////////////////////////////////
      if (link.GetAttributeFailSafe ("LossRate", tmp))
        {
          NS_LOG_INFO ("LinkError = " + link.GetAttribute ("LossRate"));

          typedef boost::tokenizer<boost::escaped_list_separator<char>> tokenizer;
          std::string value = link.GetAttribute ("LossRate");
          tokenizer tok (value);

          tokenizer::iterator token = tok.begin ();
          ObjectFactory factory (*token);

          for (token++; token != tok.end (); token++)
            {
              boost::escaped_list_separator<char> separator ('\\', '=', '\"');
              tokenizer attributeTok (*token, separator);

              tokenizer::iterator attributeToken = attributeTok.begin ();

              string attribute = *attributeToken;
              attributeToken++;

              if (attributeToken == attributeTok.end ())
                {
                  NS_LOG_ERROR ("ErrorModel attribute ["
                                << *token << "] should be in form <Attribute>=<Value>");
                  continue;
                }

              string value = *attributeToken;

              factory.Set (attribute, StringValue (value));
            }

          nd.Get (0)->SetAttribute ("ReceiveErrorModel",
                                    PointerValue (factory.Create<ErrorModel> ()));
          nd.Get (1)->SetAttribute ("ReceiveErrorModel",
                                    PointerValue (factory.Create<ErrorModel> ()));
        }
    }
}

void
IncNdnAnnotatedTopologyReader::SaveTopology (const std::string &file)
{
  ofstream os (file.c_str (), ios::trunc);
  os << "# any empty lines and lines starting with '#' symbol is ignored\n"
     << "\n"
     << "# The file should contain exactly two sections: router and link, each starting with the "
        "corresponding keyword\n"
     << "\n"
     << "# router section defines topology nodes and their relative positions (e.g., to use in "
        "visualizer)\n"
     << "router\n"
     << "\n"
     << "# each line in this section represents one router and should have the following data\n"
     << "# node  comment     yPos    xPos\n";

  for (NodeContainer::Iterator node = m_compute_nodecontainer.Begin ();
       node != m_compute_nodecontainer.End (); node++)
    {
      std::string name = Names::FindName (*node);
      Ptr<MobilityModel> mobility = (*node)->GetObject<MobilityModel> ();
      Vector position = mobility->GetPosition ();

      os << name << "\t"
         << "NA"
         << "\t" << -position.y << "\t" << position.x << "\n";
    }

  os << "# link section defines point-to-point links between nodes and characteristics of these "
        "links\n"
     << "\n"
     << "link\n"
     << "\n"
     << "# Each line should be in the following format (only first two are required, the rest can "
        "be omitted)\n"
     << "# srcNode   dstNode     bandwidth   metric  delay   queue\n"
     << "# bandwidth: link bandwidth\n"
     << "# metric: routing metric\n"
     << "# delay:  link delay\n"
     << "# queue:  MaxPackets for transmission queue on the link (both directions)\n"
     << "# error:  comma-separated list, specifying class for ErrorModel and necessary "
        "attributes\n";

  for (std::list<Link>::const_iterator link = m_linksList.begin (); link != m_linksList.end ();
       link++)
    {
      os << Names::FindName (link->GetFromNode ()) << "\t";
      os << Names::FindName (link->GetToNode ()) << "\t";

      string tmp;
      if (link->GetAttributeFailSafe ("DataRate", tmp))
        os << link->GetAttribute ("DataRate") << "\t";
      else
        NS_FATAL_ERROR ("DataRate must be specified for the link");

      if (link->GetAttributeFailSafe ("OSPF", tmp))
        os << link->GetAttribute ("OSPF") << "\t";
      else
        os << "1\t";

      if (link->GetAttributeFailSafe ("Delay", tmp))
        {
          os << link->GetAttribute ("Delay") << "\t";

          if (link->GetAttributeFailSafe ("MaxPackets", tmp))
            {
              os << link->GetAttribute ("MaxPackets") << "\t";

              if (link->GetAttributeFailSafe ("LossRate", tmp))
                {
                  os << link->GetAttribute ("LossRate") << "\t";
                }
            }
        }
      os << "\n";
    }
}

/// @cond include_hidden

template <class Names>
class name_writer
{
public:
  name_writer (Names _names) : names (_names)
  {
  }

  template <class VertexOrEdge>
  void
  operator() (std::ostream &out, const VertexOrEdge &v) const
  {
    // out << "[label=\"" << names[v] << "\",style=filled,fillcolor=\"" << colors[v] << "\"]";
    out << "[shape=\"circle\",width=0.1,label=\"\",style=filled,fillcolor=\"green\"]";
  }

private:
  Names names;
};

template <class Names>
inline name_writer<Names>
make_name_writer (Names n)
{
  return name_writer<Names> (n);
}

/// @endcond

void
IncNdnAnnotatedTopologyReader::SaveGraphviz (const std::string &file)
{
  typedef boost::adjacency_list_traits<boost::setS, boost::setS, boost::undirectedS> Traits;

  typedef boost::property<boost::vertex_name_t, std::string,
                          boost::property<boost::vertex_index_t, uint32_t>>
      nodeProperty;

  typedef boost::no_property edgeProperty;

  typedef boost::adjacency_list<boost::setS, boost::setS, boost::undirectedS, nodeProperty,
                                edgeProperty>
      Graph;

  typedef map<string, Traits::vertex_descriptor> node_map_t;
  node_map_t graphNodes;
  Graph graph;

  for (NodeContainer::Iterator node = m_compute_nodecontainer.Begin ();
       node != m_compute_nodecontainer.End (); node++)
    {
      std::pair<node_map_t::iterator, bool> retval = graphNodes.insert (make_pair (
          Names::FindName (*node), add_vertex (nodeProperty (Names::FindName (*node)), graph)));
      // NS_ASSERT (ok == true);

      put (boost::vertex_index, graph, retval.first->second, (*node)->GetId ());
    }

  for (std::list<Link>::const_iterator link = m_linksList.begin (); link != m_linksList.end ();
       link++)
    {
      node_map_t::iterator from = graphNodes.find (Names::FindName (link->GetFromNode ()));
      node_map_t::iterator to = graphNodes.find (Names::FindName (link->GetToNode ()));

      // add_edge (node->second, otherNode->second, m_graph);
      boost::add_edge (from->second, to->second, graph);
    }

  ofstream of (file.c_str ());
  boost::property_map<Graph, boost::vertex_name_t>::type names = get (boost::vertex_name, graph);
  write_graphviz (of, graph, make_name_writer (names));
}

} // namespace inc
} // namespace ndn
} // namespace ns3
