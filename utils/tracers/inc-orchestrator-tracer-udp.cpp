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


#include "inc-orchestrator-tracer-udp.hpp"

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

NS_LOG_COMPONENT_DEFINE("IncOrchestratorCommunicationTracerUdp");

namespace ns3 {
namespace ndn {
namespace inc {
static std::list<std::tuple<shared_ptr<std::ostream>, std::list<Ptr<IncOrchestratorCommunicationTracerUdp>>>> g_tracers;

void
IncOrchestratorCommunicationTracerUdp::Destroy()
{
  g_tracers.clear();
}

void
IncOrchestratorCommunicationTracerUdp::InstallAll(const std::string& file, Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncOrchestratorCommunicationTracerUdp>> tracers;
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
    Ptr<IncOrchestratorCommunicationTracerUdp> trace = Install(*node, outputStream, averagingPeriod);
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
IncOrchestratorCommunicationTracerUdp::Install(const NodeContainer& nodes, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncOrchestratorCommunicationTracerUdp>> tracers;
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
    Ptr<IncOrchestratorCommunicationTracerUdp> trace = Install(*node, outputStream, averagingPeriod);
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
IncOrchestratorCommunicationTracerUdp::Install(Ptr<Node> node, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncOrchestratorCommunicationTracerUdp>> tracers;
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

  Ptr<IncOrchestratorCommunicationTracerUdp> trace = Install(node, outputStream, averagingPeriod);
  tracers.push_back(trace);

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

Ptr<IncOrchestratorCommunicationTracerUdp>
IncOrchestratorCommunicationTracerUdp::Install(Ptr<Node> node, shared_ptr<std::ostream> outputStream,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  NS_LOG_DEBUG("Install tracer on node: " << node->GetId());

  Ptr<IncOrchestratorCommunicationTracerUdp> trace = Create<IncOrchestratorCommunicationTracerUdp>(outputStream, node);
  //trace->SetAveragingPeriod(averagingPeriod);

  return trace;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

IncOrchestratorCommunicationTracerUdp::IncOrchestratorCommunicationTracerUdp(shared_ptr<std::ostream> os, Ptr<Node> node)
  : m_nodePtr(node)
  , m_os(os)
{
  NS_LOG_DEBUG("IncOrchestratorCommunicationTracerUdp - Constructor");

  m_node = boost::lexical_cast<std::string>(m_nodePtr->GetId());

  Connect();

  std::string name = Names::FindName(node);
  if (!name.empty()) {
    m_node = name;
  }
}

IncOrchestratorCommunicationTracerUdp::IncOrchestratorCommunicationTracerUdp(shared_ptr<std::ostream> os,
  const std::string& node)
  : m_node(node)
  , m_os(os)
{
  Connect();
}

IncOrchestratorCommunicationTracerUdp::~IncOrchestratorCommunicationTracerUdp(){

  NS_LOG_DEBUG("Destructor IncOrchestratorCommunicationTracerUdp");

  PrintSum(*m_os);

  double dRec = m_stats.m_overallOrchestrationDataReceived;
  double iSent = m_stats.m_overallOrchestrationInterestSend;
  double l_intDataRatio = dRec / iSent;

  NS_LOG_DEBUG("SUMMARY - node: " << m_node << ", hopEq1: " << m_stats.m_overallOneHopSatisfied
    << ", hopGreater1: " << m_stats.m_overallManyHopSatisfied
    <<  ", overallInterestsSend: " << m_stats.m_overallOrchestrationInterestSend
    <<  ", OverallDataReceived: " << m_stats.m_overallOrchestrationDataReceived
    << ", D/I Ratio: " << std::to_string(l_intDataRatio));
};

void
IncOrchestratorCommunicationTracerUdp::Connect()
{
  NS_LOG_DEBUG("Connect and register callbacks");
  std::vector<Ptr<Application>> orchestrator_app;
  uint32_t num_apps = m_nodePtr->GetNApplications();
  std::cout<<"no of apps on orchestrator  is "<<num_apps<<std::endl;
  // get a handle to the application installed on the moblie nodes
  for(int i = 0; i<num_apps ; i++)
  {
    orchestrator_app.push_back(m_nodePtr->GetApplication(i));
    orchestrator_app.at(i)->TraceConnectWithoutContext("UdpTx",
        MakeCallback(&IncOrchestratorCommunicationTracerUdp::SendOrchestrationInterests, this));
    orchestrator_app.at(i)->TraceConnectWithoutContext("UdpRx",
       MakeCallback(&IncOrchestratorCommunicationTracerUdp::IncomingOrchestrationDatas, this));
    orchestrator_app.at(i)->TraceConnectWithoutContext("UdpDeciTx",
        MakeCallback(&IncOrchestratorCommunicationTracerUdp::SendOrchestrationDecisions, this));
    Reset();
  }
}

void
IncOrchestratorCommunicationTracerUdp::SetAveragingPeriod(const Time& period)
{
  m_period = period;
  m_printEvent.Cancel();
  m_printEvent = Simulator::Schedule(m_period,
    &IncOrchestratorCommunicationTracerUdp::PeriodicPrinter, this);
}

void
IncOrchestratorCommunicationTracerUdp::PeriodicPrinter()
{
  Print(*m_os);
  Reset();

  m_printEvent = Simulator::Schedule(m_period,
    &IncOrchestratorCommunicationTracerUdp::PeriodicPrinter, this);
}

void
IncOrchestratorCommunicationTracerUdp::PrintHeader(std::ostream& os) const
{
  os  << "Node,"
      << "ISent,"
      << "DReceived,"
      << "DecisionsSent,"
      << "OrchestratorInterests,"
      << "OrchestratorDecisions";
}

void
IncOrchestratorCommunicationTracerUdp::Reset()
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
IncOrchestratorCommunicationTracerUdp::Print(std::ostream& os) const
{
  Time time = Simulator::Now();

  PRINTER("InterestSend", m_OrchestrationInterestSendOut);
  PRINTER("DataReceived", m_OrchestrationDataReceived);
}

#define SUM_PRINTER(isent, dreceived, deciSent, inames, dnames)               \
  os  << m_node << "," << isent << "," << dreceived << "," << deciSent << "," << inames << "," <<dnames<<"\n";

void
IncOrchestratorCommunicationTracerUdp::PrintSum(std::ostream& os) const
{
  std::stringstream ss;
  ss << "<" << boost::algorithm::join(m_stats.m_interestsNames, "; ") << ">";
  std::string interestsNamesStr = ss.str();

  ss.clear();
  ss << "<" << boost::algorithm::join(m_stats.m_orchestrationDecisions, "; ") << ">";
  std::string decisionsNamesStr = ss.str();

  SUM_PRINTER(m_stats.m_overallOrchestrationInterestSend, m_stats.m_overallOrchestrationDataReceived,
   m_stats.m_overallOrchestrationDecisionsSend, interestsNamesStr, decisionsNamesStr);
}

// ---------------------------------------------------- //
//            TRACER CALLBACK FUNCTIONS                 //
// ---------------------------------------------------- //
void
IncOrchestratorCommunicationTracerUdp::SendOrchestrationInterests(ns3::Ptr<const Packet> p) {
  m_stats.m_OrchestrationInterestSendOut++;
  m_stats.m_overallOrchestrationInterestSend++;
  std::string packet_data;
  uint8_t *buffer = new uint8_t[p->GetSize ()];
  p->CopyData (buffer, p->GetSize ());
  packet_data = std::string ((char *) buffer);
  m_stats.m_interestsNames.push_back(packet_data);
}

void
IncOrchestratorCommunicationTracerUdp::SendOrchestrationDecisions(ns3::Ptr<const Packet> p) {
  m_stats.m_OrchestrationDecisionEnforced++;
  m_stats.m_overallOrchestrationDecisionsSend++;
  std::string packet_data;
  uint8_t *buffer = new uint8_t[p->GetSize ()];
  p->CopyData (buffer, p->GetSize ());
  packet_data = std::string ((char *) buffer);
  m_stats.m_orchestrationDecisions.push_back(packet_data);
}

void
IncOrchestratorCommunicationTracerUdp::IncomingOrchestrationDatas(ns3::Ptr<const Packet> p){
  // int hopCount = 0;
  // auto hopCountTag = data->getTag<lp::HopCountTag>();
  // if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    m_stats.m_OrchestrationDataReceived++;
    m_stats.m_overallOrchestrationDataReceived++;
  //   hopCount = *hopCountTag;
  //   if (hopCount == 0) {
  //     NS_LOG_DEBUG("received data hopcount 0");
  //   }
  //   else if (hopCount == 1) {
  //     // data was received 1 hop count away
  //     m_stats.m_overallOneHopSatisfied++;
  //   }
  //   else {
  //     m_stats.m_overallManyHopSatisfied++;
  //   }
  // }
}

} //namespace inc
} // namespace ndn
} // namespace ns3
