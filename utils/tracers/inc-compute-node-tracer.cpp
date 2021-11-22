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


#include "inc-compute-node-tracer.hpp"
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

NS_LOG_COMPONENT_DEFINE("IncComputeNodeTracer");

namespace ns3 {
namespace ndn {
namespace inc {
static std::list<std::tuple<shared_ptr<std::ostream>, std::list<Ptr<IncComputeNodeTracer>>>> g_tracers;

void
IncComputeNodeTracer::Destroy()
{
  g_tracers.clear();
}

void
IncComputeNodeTracer::InstallAll(const std::string& file, Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncComputeNodeTracer>> tracers;
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
    Ptr<IncComputeNodeTracer> trace = Install(*node, outputStream, averagingPeriod);
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
IncComputeNodeTracer::Install(const NodeContainer& nodes, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncComputeNodeTracer>> tracers;
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
    Ptr<IncComputeNodeTracer> trace = Install(*node, outputStream, averagingPeriod);
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
IncComputeNodeTracer::Install(Ptr<Node> node, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncComputeNodeTracer>> tracers;
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

  Ptr<IncComputeNodeTracer> trace = Install(node, outputStream, averagingPeriod);
  tracers.push_back(trace);

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

Ptr<IncComputeNodeTracer>
IncComputeNodeTracer::Install(Ptr<Node> node, shared_ptr<std::ostream> outputStream,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  NS_LOG_DEBUG("Install tracer on node: " << node->GetId());
  Ptr<IncComputeNodeTracer> trace = Create<IncComputeNodeTracer>(outputStream, node);
  //trace->SetAveragingPeriod(averagingPeriod);

  return trace;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

IncComputeNodeTracer::IncComputeNodeTracer(shared_ptr<std::ostream> os, Ptr<Node> node)
  : m_nodePtr(node)
  , m_os(os)
{
  NS_LOG_DEBUG("IncComputeNodeTracer - Constructor");

  m_node = boost::lexical_cast<std::string>(m_nodePtr->GetId());

  Connect();

  std::string name = Names::FindName(node);
  if (!name.empty()) {
    m_node = name;
  }
}

IncComputeNodeTracer::IncComputeNodeTracer(shared_ptr<std::ostream> os,
  const std::string& node)
  : m_node(node)
  , m_os(os)
{
  Connect();
}

IncComputeNodeTracer::~IncComputeNodeTracer(){

  NS_LOG_DEBUG("Destructor IncComputeNodeTracer");
  PrintSum(*m_os);

};

void
IncComputeNodeTracer::Connect()
{
  NS_LOG_DEBUG("Connect and register callbacks");
  // get a handle to the application installed on the compute nodes
  int num_apps = m_nodePtr->GetNApplications();
  for(int i = 0; i<num_apps;i++)
  {
    Ptr<Application> compute_node_app = m_nodePtr->GetApplication(i);
    compute_node_app->TraceConnectWithoutContext("ExecutingFunctionTrace",
        MakeCallback(&IncComputeNodeTracer::FuncExecution, this));
    Reset();
  }
}

void
IncComputeNodeTracer::SetAveragingPeriod(const Time& period)
{
  m_period = period;
  m_printEvent.Cancel();
  m_printEvent = Simulator::Schedule(m_period,
    &IncComputeNodeTracer::PeriodicPrinter, this);
}

void
IncComputeNodeTracer::PeriodicPrinter()
{
  Print(*m_os);
  Reset();

  m_printEvent = Simulator::Schedule(m_period,
    &IncComputeNodeTracer::PeriodicPrinter, this);
}

void
IncComputeNodeTracer::PrintHeader(std::ostream& os) const
{
  os  << "Node,"
      << "TotalNumExecutions,"
      << "TotalCPUUtilized,"
      << "TotalRAMUtilized,"
      << "TotalROMUtilized,"
      << "TotalOccupancyTime,"
      << "FunctionsExecuted,"
      << "FunctionsExecTime";
}

void
IncComputeNodeTracer::Reset()
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
IncComputeNodeTracer::Print(std::ostream& os) const
{
  Time time = Simulator::Now();

  //PRINTER("InterestSend", m_interestSendOut);
  //PRINTER("DataReceived", m_dataReceived);
}

#define SUM_PRINTER(overallExecutions, overallCPU, overallRAM, overallROM, overallExecTime, FuncNames)               \
  os  << m_node << "," << overallExecutions<< "," << overallCPU << "," << overallRAM << "," << overallROM <<"," << overallExecTime <<"," << FuncNames <<"\n";

void
IncComputeNodeTracer::PrintSum(std::ostream& os) const
{
  std::stringstream ss;
  ss << "<" << boost::algorithm::join(m_stats.m_executed_funcNames, "; ") << ">";
  std::string executedFuncNames = ss.str();

  SUM_PRINTER(m_stats.m_overallExecutions, m_stats.m_overallCPUUtilized, m_stats.m_overallRAMUtilized, m_stats.m_overallROMUtilized ,m_stats.m_overallExecTime, executedFuncNames);
}

// ---------------------------------------------------- //
//            TRACER CALLBACK FUNCTIONS                 //
// ---------------------------------------------------- //
void
IncComputeNodeTracer::FuncExecution(std::vector<std::string> func_properties){
  m_stats.m_overallCPUUtilized = m_stats.m_overallCPUUtilized + std::stoi(func_properties.at(1));
  m_stats.m_overallRAMUtilized = m_stats.m_overallRAMUtilized + std::stoi(func_properties.at(2));
  m_stats.m_overallROMUtilized = m_stats.m_overallROMUtilized + std::stoi(func_properties.at(3));
  m_stats.m_overallExecTime = m_stats.m_overallExecTime + std::stof(func_properties.at(4));
  m_stats.m_overallExecutions++;
  std::string func_name;
  func_name = func_properties.at(0);
  func_name.append("-"+func_properties.at(4));
  m_stats.m_executed_funcNames.push_back(func_name);
}




} //namespace inc
} // namespace ndn
} // namespace ns3
