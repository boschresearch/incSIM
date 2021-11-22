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
  * Based on the cs-tracer.cpp implementation of ndnSIM
  * https://github.com/named-data-ndnSIM/ndnSIM/blob/master/utils/tracers/ndn-cs-tracer.cpp
  *
  * This tracer defines an application monitoring component for the ns3 network
  * simulator. The tracer is intended for data/result generting nodes in the
  * incl. data producers, compute nodes, etc.. The tracer monitors incoming
  * Interests and outgoing Data packets on application level and can be plugged
  * into typical ndnSIM applications.
  *
  *  Date: November 8th, 2021
  *  Contributors:
  *      Robert Bosch GmbH - initial API and functionality
  *          Dennis Grewe <dennis.grewe@de.bosch.com>
  *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
  * *****************************************************************************
  */


#include "inc-orchestrator-tracer-node.hpp"
#include <fstream>
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/callback.h"
#include "ns3/simulator.h"
#include "ns3/node-list.h"
#include "ns3/log.h"

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/incSIM-module.h"
#include "ns3/ndnSIM-module.h"

#include <ndn-cxx/lp/tags.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

NS_LOG_COMPONENT_DEFINE("IncOrchestratorTracerNode");

namespace ns3 {
namespace ndn {
namespace inc {
static std::list<std::tuple<shared_ptr<std::ostream>, std::list<Ptr<IncOrchestratorTracerNode>>>> g_tracers;

void
IncOrchestratorTracerNode::Destroy()
{
  g_tracers.clear();
}

void
IncOrchestratorTracerNode::InstallAll(const std::string& file, Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncOrchestratorTracerNode>> tracers;
  shared_ptr<std::ostream> outputStream;
  if (file != "-") {
    shared_ptr<std::ofstream> os(new std::ofstream());
    os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!os->is_open()) {
      NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
      return;
    }

    outputStream = os;
  }
  else {
    outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
  }

  for (NodeList::Iterator node = NodeList::Begin(); node != NodeList::End(); node++) {
    Ptr<IncOrchestratorTracerNode> trace = Install(*node, outputStream, averagingPeriod);
    tracers.push_back(trace);
  }

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

void
IncOrchestratorTracerNode::Install(const NodeContainer& nodes, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncOrchestratorTracerNode>> tracers;
  shared_ptr<std::ostream> outputStream;
  if (file != "-") {
    shared_ptr<std::ofstream> os(new std::ofstream());
    os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!os->is_open()) {
      NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
      return;
    }

    outputStream = os;
  }
  else {
    outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
  }

  for (NodeContainer::Iterator node = nodes.Begin(); node != nodes.End(); node++) {
    Ptr<IncOrchestratorTracerNode> trace = Install(*node, outputStream, averagingPeriod);
    tracers.push_back(trace);
  }

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

void
IncOrchestratorTracerNode::Install(Ptr<Node> node, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncOrchestratorTracerNode>> tracers;
  shared_ptr<std::ostream> outputStream;
  if (file != "-") {
    shared_ptr<std::ofstream> os(new std::ofstream());
    os->open(file.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!os->is_open()) {
      NS_LOG_ERROR("File " << file << " cannot be opened for writing. Tracing disabled");
      return;
    }

    outputStream = os;
  }
  else {
    outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
  }

  Ptr<IncOrchestratorTracerNode> trace = Install(node, outputStream, averagingPeriod);
  tracers.push_back(trace);

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

Ptr<IncOrchestratorTracerNode>
IncOrchestratorTracerNode::Install(Ptr<Node> node, shared_ptr<std::ostream> outputStream,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  NS_LOG_DEBUG("Install tracer on node: " << node->GetId());
  Ptr<IncOrchestratorTracerNode> trace = Create<IncOrchestratorTracerNode>(outputStream, node);
  //trace->SetAveragingPeriod(averagingPeriod);

  return trace;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

IncOrchestratorTracerNode::IncOrchestratorTracerNode(shared_ptr<std::ostream> os, Ptr<Node> node)
  : m_nodePtr(node)
  , m_os(os)
{
  NS_LOG_DEBUG("IncOrchestratorTracerNode - Constructor");

  m_node = boost::lexical_cast<std::string>(m_nodePtr->GetId());

  Connect();

  std::string name = Names::FindName(node);
  if (!name.empty()) {
    m_node = name;
  }
}

IncOrchestratorTracerNode::IncOrchestratorTracerNode(shared_ptr<std::ostream> os,
  const std::string& node)
  : m_node(node)
  , m_os(os)
{
  Connect();
}

IncOrchestratorTracerNode::~IncOrchestratorTracerNode(){

  NS_LOG_DEBUG("Destructor IncOrchestratorTracerNode");

  PrintSum(*m_os);
};

void
IncOrchestratorTracerNode::Connect()
{
  NS_LOG_DEBUG("Connect and register callbacks");
  // get a handle to the application installed on the compute nodes
    Ptr<Application> orch_node_app = m_nodePtr->GetApplication(0);
    orch_node_app->TraceConnectWithoutContext("EnableFunction",
        MakeCallback(&IncOrchestratorTracerNode::EnableFunctionCounter,this));
    orch_node_app->TraceConnectWithoutContext("DisableFunction",
        MakeCallback(&IncOrchestratorTracerNode::DisableFunctionCounter,this));
    Reset();
}

void
IncOrchestratorTracerNode::SetAveragingPeriod(const Time& period)
{
  m_period = period;
  m_printEvent.Cancel();
  m_printEvent = Simulator::Schedule(m_period,
    &IncOrchestratorTracerNode::PeriodicPrinter, this);
}

void
IncOrchestratorTracerNode::PeriodicPrinter()
{
  Print(*m_os);
  Reset();

  m_printEvent = Simulator::Schedule(m_period,
    &IncOrchestratorTracerNode::PeriodicPrinter, this);
}

void
IncOrchestratorTracerNode::PrintHeader(std::ostream& os) const
{
  os  << "Node,"
      << "Functions Enabled,"
      << "Enabled Functions Names,"
      << "Functions Disabled,"
      << "Disabled Functions Names";
}

void
IncOrchestratorTracerNode::Reset()
{
  m_stats.Reset();
}

// ---------------------------------------------------- //
//            PRINTER FUNCTIONS                         //
// ---------------------------------------------------- //

#define PRINTER(printName, fieldName)                                                              \
  os << time.ToDouble(Time::S) << "\t" << m_node << "\t" << printName << "\t" << m_stats.fieldName \
     << "\n";

void
IncOrchestratorTracerNode::Print(std::ostream& os) const
{
  Time time = Simulator::Now();

  PRINTER("FunctionEnabled", m_enableFunctions);
  PRINTER("FunctionDisabled", m_disableFunctions);
}

#define SUM_PRINTER(funcEnableCounter, funcEnableNames, funcDisableCounter, funcDisableNames)               \
  os << m_node<< "," <<funcEnableCounter<< "," <<funcEnableNames<< "," <<funcDisableCounter<< "," <<funcDisableNames<< "\n";

void
IncOrchestratorTracerNode::PrintSum(std::ostream& os) const
{
  std::stringstream ss;
  ss << "<" << boost::algorithm::join(m_stats.m_enabledFunctionsNames, "; ") << ">";
  std::string En_functionsNamesStr = ss.str();
  ss.clear();
  ss << "<" << boost::algorithm::join(m_stats.m_disabledFunctionsNames, "; ") << ">";
  std::string Dis_functionsNamesStr = ss.str();

  SUM_PRINTER(m_stats.m_enableFunctions, En_functionsNamesStr, m_stats.m_disableFunctions, Dis_functionsNamesStr);
}

// ---------------------------------------------------- //
//            TRACER CALLBACK FUNCTIONS                 //
// ---------------------------------------------------- //
void
IncOrchestratorTracerNode::EnableFunctionCounter(std::string func_name){
  m_stats.m_enableFunctions++;
  m_stats.m_enabledFunctionsNames.push_back(func_name);
}

void
IncOrchestratorTracerNode::DisableFunctionCounter(std::string func_name){
  m_stats.m_disableFunctions++;
  m_stats.m_disabledFunctionsNames.push_back(func_name);
}

} //namespace inc
} // namespace ndn
} // namespace ns3
