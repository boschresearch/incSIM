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
  * Based on the app-delay-tracer.cpp implementation of ndnSIM
  * https://ndnsim.net/current/doxygen/classns3_1_1ndn_1_1AppDelayTracer.html
  *
  *  Date: November 8th, 2021
  *  Contributors:
  *      Robert Bosch GmbH - initial API and functionality
  *          Dennis Grewe <dennis.grewe@de.bosch.com>
  *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
  * *****************************************************************************
  */

#include "inc-app-delay-tracer.hpp"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/callback.h"

#include "apps/ndn-app.hpp"
#include "ns3/simulator.h"
#include "ns3/node-list.h"
#include "ns3/log.h"

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <fstream>

NS_LOG_COMPONENT_DEFINE("ndn.inc.IncAppDelayTracer");

namespace ns3 {
namespace ndn {
namespace inc {

static std::list<std::tuple<shared_ptr<std::ostream>, std::list<Ptr<IncAppDelayTracer>>>>
  g_tracers;

void
IncAppDelayTracer::Destroy()
{
  g_tracers.clear();
}

void
IncAppDelayTracer::InstallAll(const std::string& file)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncAppDelayTracer>> tracers;
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
    Ptr<IncAppDelayTracer> trace = Install(*node, outputStream);
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
IncAppDelayTracer::Install(const NodeContainer& nodes, const std::string& file)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncAppDelayTracer>> tracers;
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
    Ptr<IncAppDelayTracer> trace = Install(*node, outputStream);
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
IncAppDelayTracer::Install(Ptr<Node> node, const std::string& file)
{
  using namespace boost;
  using namespace std;

  std::list<Ptr<IncAppDelayTracer>> tracers;
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

  Ptr<IncAppDelayTracer> trace = Install(node, outputStream);
  tracers.push_back(trace);

  if (tracers.size() > 0) {
    // *m_l3RateTrace << "# "; // not necessary for R's read.table
    tracers.front()->PrintHeader(*outputStream);
    *outputStream << "\n";
  }

  g_tracers.push_back(std::make_tuple(outputStream, tracers));
}

Ptr<IncAppDelayTracer>
IncAppDelayTracer::Install(Ptr<Node> node, shared_ptr<std::ostream> outputStream)
{
  NS_LOG_DEBUG("Node: " << node->GetId());

  Ptr<IncAppDelayTracer> trace = Create<IncAppDelayTracer>(outputStream, node);

  return trace;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

IncAppDelayTracer::IncAppDelayTracer(shared_ptr<std::ostream> os, Ptr<Node> node)
  : m_nodePtr(node)
  , m_os(os)
{
  m_node = boost::lexical_cast<std::string>(m_nodePtr->GetId());

  Connect();

  std::string name = Names::FindName(node);
  if (!name.empty()) {
    m_node = name;
  }
}

IncAppDelayTracer::IncAppDelayTracer(shared_ptr<std::ostream> os, const std::string& node)
  : m_node(node)
  , m_os(os)
{
  Connect();
}

IncAppDelayTracer::~IncAppDelayTracer(){};

void
IncAppDelayTracer::Connect()
{
  //Config::ConnectWithoutContext("/NodeList/" + m_node
  //                                + "/ApplicationList/*/LastRetransmittedInterestDataDelayInc",
  //                              MakeCallback(&IncAppDelayTracer::LastRetransmittedInterestDataDelay,
  //                                           this));

  Config::ConnectWithoutContext("/NodeList/" + m_node + "/ApplicationList/*/FirstInterestDataDelayInc",
                                MakeCallback(&IncAppDelayTracer::FirstInterestDataDelay, this));
}

void
IncAppDelayTracer::PrintHeader(std::ostream& os) const
{
  os << "Time,"
     << "Node,"
     << "SeqNo,"

     << "Type,"
     << "DelayS,"
     << "DelayUS,"
     << "RetxCount,"
     << "HopCount";
}

void
IncAppDelayTracer::LastRetransmittedInterestDataDelay(Ptr<App> app, uint32_t seqno, Time delay,
                                                   int32_t hopCount)
{
  *m_os << Simulator::Now().ToDouble(Time::S) << "," << m_node << ","
        << seqno << ","
        << "LastDelay"
        << "," << delay.ToDouble(Time::S) << "," << delay.ToDouble(Time::US) << "," << 1 << ","
        << hopCount << "\n";
}

void
IncAppDelayTracer::FirstInterestDataDelay(Ptr<App> app, uint32_t seqno, Time delay, uint32_t retxCount,
                                       int32_t hopCount)
{
  *m_os << Simulator::Now().ToDouble(Time::S) << "," << m_node << ","
        << seqno << ","
        << "FullDelay"
        << "," << delay.ToDouble(Time::S) << "," << delay.ToDouble(Time::US) << "," << retxCount
        << "," << hopCount << "\n";
}

} //namespace inc
} // namespace ndn
} // namespace ns3
