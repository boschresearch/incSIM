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
  *  Date: November 8th, 2021
  *  Contributors:
  *      Robert Bosch GmbH - initial API and functionality
  *          Dennis Grewe <dennis.grewe@de.bosch.com>
  *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
  * *****************************************************************************
  */

#include "inc-consumer-tracer.hpp"

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
#include "ns3/ndnSIM-module.h"
#include "ns3/incSIM-module.h"
#include <ndn-cxx/lp/tags.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

NS_LOG_COMPONENT_DEFINE("inc.IncConsumerTracer");

namespace ns3 {
namespace ndn {
namespace inc {
static std::list<std::tuple<shared_ptr<std::ostream>, std::list<Ptr<IncConsumerTracer>>>> g_tracers;

void
IncConsumerTracer::Destroy()
{
  g_tracers.clear();
}

void
IncConsumerTracer::InstallAll(const std::string& file, Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncConsumerTracer>> tracers;
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
    Ptr<IncConsumerTracer> trace = Install(*node, outputStream, averagingPeriod);
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
IncConsumerTracer::Install(const NodeContainer& nodes, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  using namespace boost;
  using namespace std;
  std::list<Ptr<IncConsumerTracer>> tracers;
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
    Ptr<IncConsumerTracer> trace = Install(*node, outputStream, averagingPeriod);
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
IncConsumerTracer::Install(Ptr<Node> node, const std::string& file,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{

  using namespace boost;
  using namespace std;

  std::list<Ptr<IncConsumerTracer>> tracers;
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

  Ptr<IncConsumerTracer> trace = Install(node, outputStream, averagingPeriod);
  tracers.push_back(trace);

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

Ptr<IncConsumerTracer>
IncConsumerTracer::Install(Ptr<Node> node, shared_ptr<std::ostream> outputStream,
                  Time averagingPeriod /* = Seconds (0.5)*/)
{
  NS_LOG_DEBUG("Install tracer on node: " << node->GetId());
  Ptr<IncConsumerTracer> trace = Create<IncConsumerTracer>(outputStream, node);
  //trace->SetAveragingPeriod(averagingPeriod);
  return trace;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

IncConsumerTracer::IncConsumerTracer(shared_ptr<std::ostream> os, Ptr<Node> node)
  : m_nodePtr(node)
  , m_os(os)
{
  NS_LOG_DEBUG("IncConsumerTracer - Constructor");
  m_node = boost::lexical_cast<std::string>(m_nodePtr->GetId());

  Connect();
  std::string name = Names::FindName(node);
  if (!name.empty()) {
    m_node = name;
  }
}

IncConsumerTracer::IncConsumerTracer(shared_ptr<std::ostream> os,
  const std::string& node)
  : m_node(node)
  , m_os(os)
{
  Connect();
}

IncConsumerTracer::~IncConsumerTracer(){

  NS_LOG_DEBUG("Destructor IncConsumerTracer");

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
IncConsumerTracer::Connect()
{
  NS_LOG_DEBUG("Connect and register callbacks");
  // get a handle to the application installed on the moblie nodes
  int num_apps = m_nodePtr->GetNApplications();
  for(int i=0; i<num_apps;i++)
  {
    Ptr<Application> consumer_app = m_nodePtr->GetApplication(i);
    consumer_app->TraceConnectWithoutContext("SendInterestsInc",
        MakeCallback(&IncConsumerTracer::SendConsumerInterests, this));
    consumer_app->TraceConnectWithoutContext("IncomingDatasInc",
       MakeCallback(&IncConsumerTracer::IncomingConsumerDatas, this));

    Reset();
  }
}

void
IncConsumerTracer::SetAveragingPeriod(const Time& period)
{
  m_period = period;
  m_printEvent.Cancel();
  m_printEvent = Simulator::Schedule(m_period,
    &IncConsumerTracer::PeriodicPrinter, this);
}

void
IncConsumerTracer::PeriodicPrinter()
{
  Print(*m_os);
  Reset();

  m_printEvent = Simulator::Schedule(m_period,
    &IncConsumerTracer::PeriodicPrinter, this);
}

void
IncConsumerTracer::PrintHeader(std::ostream& os) const
{
  os  << "Node,"
      << "ISent,"
      << "DReceived,"
      << "DataNames";
}

void
IncConsumerTracer::Reset()
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
IncConsumerTracer::Print(std::ostream& os) const
{
  Time time = Simulator::Now();

  PRINTER("InterestSend", m_interestSendOut);
  PRINTER("DataReceived", m_dataReceived);
}

#define SUM_PRINTER(isent, dreceived, dnames)               \
  os  << m_node << "," << isent << "," << dreceived << "," << dnames << "\n";

void
IncConsumerTracer::PrintSum(std::ostream& os) const
{
  std::stringstream ss;
  ss << "<" << boost::algorithm::join(m_stats.m_dataNames, "; ") << ">";
  std::string dataNamesStr = ss.str();

  SUM_PRINTER(m_stats.m_overallInterestSend, m_stats.m_overallDataReceived,
    dataNamesStr);
}

// ---------------------------------------------------- //
//            TRACER CALLBACK FUNCTIONS                 //
// ---------------------------------------------------- //
void
IncConsumerTracer::SendConsumerInterests(shared_ptr<const Interest> interest) {
  m_stats.m_interestSendOut++;
  m_stats.m_overallInterestSend++;

  m_stats.m_interestsNames.push_back(interest->getName().toUri());
}

void
IncConsumerTracer::IncomingConsumerDatas(shared_ptr<const Data> data){
    m_stats.m_dataReceived++;
    m_stats.m_overallDataReceived++;
    m_stats.m_dataNames.push_back(data->getName().toUri());

    // hopCount = *hopCountTag;
    // if (hopCount == 0) {
    //   NS_LOG_DEBUG("received data hopcount 0");
    // }
    // else if (hopCount == 1) {
    //   // data was received 1 hop count away
    //   m_stats.m_overallOneHopSatisfied++;
    // }
    // else {
    //   m_stats.m_overallManyHopSatisfied++;
    // }
  }

} //namespace inc
} // namespace ndn
} // namespace ns3
