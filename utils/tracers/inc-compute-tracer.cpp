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


#include "inc-compute-tracer.hpp"
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

NS_LOG_COMPONENT_DEFINE("IncComputeTracer");

namespace ns3 {
namespace ndn {
namespace inc {
static std::list<std::tuple<shared_ptr<std::ostream>, std::list<Ptr<IncComputeTracer>>>> g_tracers;

void
IncComputeTracer::Destroy()
{
  g_tracers.clear();
}

void
IncComputeTracer::InstallAll(const std::string& file, Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncComputeTracer>> tracers;
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
    Ptr<IncComputeTracer> trace = Install(*node, outputStream, averagingPeriod);
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
IncComputeTracer::Install(const NodeContainer& nodes, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncComputeTracer>> tracers;
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
    Ptr<IncComputeTracer> trace = Install(*node, outputStream, averagingPeriod);
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
IncComputeTracer::Install(Ptr<Node> node, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncComputeTracer>> tracers;
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

  Ptr<IncComputeTracer> trace = Install(node, outputStream, averagingPeriod);
  tracers.push_back(trace);

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

Ptr<IncComputeTracer>
IncComputeTracer::Install(Ptr<Node> node, shared_ptr<std::ostream> outputStream,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  NS_LOG_DEBUG("Install tracer on node: " << node->GetId());
  Ptr<IncComputeTracer> trace = Create<IncComputeTracer>(outputStream, node);
  //trace->SetAveragingPeriod(averagingPeriod);

  return trace;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

IncComputeTracer::IncComputeTracer(shared_ptr<std::ostream> os, Ptr<Node> node)
  : m_nodePtr(node)
  , m_os(os)
{
  NS_LOG_DEBUG("IncComputeTracer - Constructor");

  m_node = boost::lexical_cast<std::string>(m_nodePtr->GetId());

  Connect();

  std::string name = Names::FindName(node);
  if (!name.empty()) {
    m_node = name;
  }
}

IncComputeTracer::IncComputeTracer(shared_ptr<std::ostream> os,
  const std::string& node)
  : m_node(node)
  , m_os(os)
{
  Connect();
}

IncComputeTracer::~IncComputeTracer(){

  NS_LOG_DEBUG("Destructor IncComputeTracer");

  PrintSum(*m_os);

  double dRec = m_stats.m_overallDataReceived;
  double iSent = m_stats.m_overallInterestSend;
  double l_intDataRatio = dRec / iSent;

  NS_LOG_DEBUG("SUMMARY - node: " << m_node << ", hopEq1: " << m_stats.m_overallOneHopSatisfied
    << ", hopGreater1: " << m_stats.m_overallManyHopSatisfied
    <<  ", overallInterestsSend: " << m_stats.m_overallInterestSend
    <<  ", OverallDataReceived: " << m_stats.m_overallDataReceived
    << ", D/I Ratio: " << std::to_string(l_intDataRatio));
};

void
IncComputeTracer::Connect()
{
  NS_LOG_DEBUG("Connect and register callbacks");
  // get a handle to the application installed on the compute nodes
  int num_apps = m_nodePtr->GetNApplications();
  for(int i = 0; i<num_apps;i++)
  {
    Ptr<Application> compute_node_app = m_nodePtr->GetApplication(i);
    compute_node_app->TraceConnectWithoutContext("SendComputeInterests",
        MakeCallback(&IncComputeTracer::SendComputeInterests, this));
    compute_node_app->TraceConnectWithoutContext("RcvComputeInterests",
        MakeCallback(&IncComputeTracer::RcvComputeInterests, this));
    compute_node_app->TraceConnectWithoutContext("OutgoingComputeDatas",
        MakeCallback(&IncComputeTracer::OutgoingComputeDatas, this));
    compute_node_app->TraceConnectWithoutContext("IncomingComputeDatas",
        MakeCallback(&IncComputeTracer::IncomingComputeDatas, this));
    compute_node_app->TraceConnectWithoutContext("NodeBusyFailure",
        MakeCallback(&IncComputeTracer::NodeBusy, this));
    compute_node_app->TraceConnectWithoutContext("FuncDisabledFailure",
        MakeCallback(&IncComputeTracer::FuncDisabled, this));
    compute_node_app->TraceConnectWithoutContext("NFN_FuncEnabled",
        MakeCallback(&IncComputeTracer::FuncEnabled, this));
    compute_node_app->TraceConnectWithoutContext("NFN_IntForwarded",
        MakeCallback(&IncComputeTracer::IntForwarded, this));
    compute_node_app->TraceConnectWithoutContext("NFN_IntQueued",
        MakeCallback(&IncComputeTracer::IntQueued, this));
    Reset();
  }
}

void
IncComputeTracer::SetAveragingPeriod(const Time& period)
{
  m_period = period;
  m_printEvent.Cancel();
  m_printEvent = Simulator::Schedule(m_period,
    &IncComputeTracer::PeriodicPrinter, this);
}

void
IncComputeTracer::PeriodicPrinter()
{
  Print(*m_os);
  Reset();

  m_printEvent = Simulator::Schedule(m_period,
    &IncComputeTracer::PeriodicPrinter, this);
}

void
IncComputeTracer::PrintHeader(std::ostream& os) const
{
  os  << "Node,"
      << "IReceived,"
      << "ISent,"
      << "DReceived,"
      << "DSent,"
      << "IQueued,"
      << "Nodebusy,"
      << "InterestForwarded,"
      << "FuncDisabled,"
      << "NFNFuncEnabled,"
      << "SentInterestNames,"
      << "ReceivedInterestNames,"
      << "FuncNames,"
      << "DisabledFuncNames,"
      << "NfnEnabledFuncNames";
}

void
IncComputeTracer::Reset()
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
IncComputeTracer::Print(std::ostream& os) const
{
  Time time = Simulator::Now();

  PRINTER("InterestSend", m_interestSendOut);
  PRINTER("DataReceived", m_dataReceived);
}

#define SUM_PRINTER(ireceived, isent, dreceived, dsent, iQueued, nBusy, intForwarded, fDisabled, fEnabled, iSentnames, iReceivednames, nbFuncNames, disFuncNames, enFuncNames)               \
  os  << m_node << "," << ireceived<< "," << isent << "," << dreceived << "," << dsent<<"," << iQueued <<"," << nBusy <<"," <<intForwarded <<","<<fDisabled << "," << fEnabled << ","   \
  << iSentnames <<"," << iReceivednames <<"," << nbFuncNames <<","<<disFuncNames << "," << enFuncNames <<"\n";

void
IncComputeTracer::PrintSum(std::ostream& os) const
{
  std::stringstream ss;
  ss << "<" << boost::algorithm::join(m_stats.m_Send_interestsNames, "; ") << ">";
  std::string Send_interestsNamesStr = ss.str();
  ss.clear();
  ss << "<" << boost::algorithm::join(m_stats.m_Rcv_interestsNames, "; ") << ">";
  std::string Rcv_interestsNamesStr = ss.str();
  ss.clear();
  ss << "<" << boost::algorithm::join(m_stats.m_nodeBusy_funcNames, "; ") << ">";
  std::string Nb_funcNamesStr = ss.str();
  ss.clear();
  ss << "<" << boost::algorithm::join(m_stats.m_funcDisabled_funcNames, "; ") << ">";
  std::string Dis_funcNamesStr = ss.str();
  ss << "<" << boost::algorithm::join(m_stats.m_nfnFuncEnabled_funcNames, ";") << ">";
  std::string En_funcNamesStr = ss.str();
  SUM_PRINTER(m_stats.m_overallInterestReceived, m_stats.m_overallInterestSend, m_stats.m_overallDataReceived, m_stats.m_overallDataSend, m_stats.m_overallIntQueued,
  m_stats.m_overallNodeBusy, m_stats.m_overallIntForwarded, m_stats.m_overallFuncDisabled, m_stats.m_overallNfnFuncEnabled,
  Send_interestsNamesStr, Rcv_interestsNamesStr, Nb_funcNamesStr, Dis_funcNamesStr, En_funcNamesStr);
}

// ---------------------------------------------------- //
//            TRACER CALLBACK FUNCTIONS                 //
// ---------------------------------------------------- //
void
IncComputeTracer::SendComputeInterests(shared_ptr<const Interest> interest) {
  m_stats.m_interestSendOut++;
  m_stats.m_overallInterestSend++;
  m_stats.m_Send_interestsNames.push_back(interest->getName().toUri());
}

void
IncComputeTracer::RcvComputeInterests(shared_ptr<const Interest> interest) {
  m_stats.m_interestReceived++;
  m_stats.m_overallInterestReceived++;
  m_stats.m_Rcv_interestsNames.push_back(interest->getName().toUri());
}

void
IncComputeTracer::IncomingComputeDatas(shared_ptr<const Data> data){
    m_stats.m_dataReceived++;
    m_stats.m_overallDataReceived++;
    m_stats.m_Rcv_datasNames.push_back(data->getName().toUri());
}

void
IncComputeTracer::OutgoingComputeDatas(shared_ptr<const Data> data){
    m_stats.m_dataSendOut++;
    m_stats.m_overallDataSend++;
    m_stats.m_Send_datasNames.push_back(data->getName().toUri());
}

void
IncComputeTracer::NodeBusy(std::string func_name){
  m_stats.m_overallNodeBusy++;
  m_stats.m_nodeBusy_funcNames.push_back(func_name);
}

void
IncComputeTracer::FuncDisabled(std::string func_name){
  m_stats.m_overallFuncDisabled++;
  m_stats.m_funcDisabled_funcNames.push_back(func_name);
}

void
IncComputeTracer::FuncEnabled(std::string func_name){
  m_stats.m_overallNfnFuncEnabled++;
  m_stats.m_nfnFuncEnabled_funcNames.push_back(func_name);
}

void
IncComputeTracer::IntForwarded(std::string interest_name){
  m_stats.m_overallIntForwarded++;
  m_stats.m_nfnIntForwarded_intNames.push_back(interest_name);
}

void
IncComputeTracer::IntQueued(std::string interest_name){
  m_stats.m_overallIntQueued++;
  m_stats.m_nfnIntQueued_intNames.push_back(interest_name);
}

} //namespace inc
} // namespace ndn
} // namespace ns3
