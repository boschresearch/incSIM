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
 * Based on the ndn-consumer-cbr implementation of ns3
 * https://ndnsim.net/current/doxygen/classns3_1_1ndn_1_1ConsumerCbr.html
 * Originated from ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 * *****************************************************************************
 */

#include "inc-consumer-app.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#include "utils/ndn-ns3-packet-tag.hpp"
#include "utils/ndn-rtt-mean-deviation.hpp"

#include <ndn-cxx/lp/tags.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>

NS_LOG_COMPONENT_DEFINE("inc.ConsumerApp");

namespace ns3 {
namespace ndn {
namespace inc {

NS_OBJECT_ENSURE_REGISTERED(ConsumerApp);

TypeId
ConsumerApp::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::inc::ConsumerApp")
      .SetGroupName("Inc")
      .SetParent<INC_ConsumerBaseApp>()
      .AddConstructor<ConsumerApp>()

      .AddAttribute("Interval", "Interval between interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&ConsumerApp::m_interval), MakeDoubleChecker<double>())

      .AddAttribute("Randomize",
                    "Type of send time randomization: none (default), uniform, exponential",
                    StringValue("none"),
                    MakeStringAccessor(&ConsumerApp::SetRandomize, &ConsumerApp::GetRandomize),
                    MakeStringChecker())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerApp::m_seqMax), MakeIntegerChecker<uint32_t>())
    ;

  return tid;
}

ConsumerApp::ConsumerApp()
  : m_interval(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();
}

ConsumerApp::~ConsumerApp()
{
}

void
ConsumerApp::ScheduleNextPacket()
{
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &INC_ConsumerBaseApp::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(m_interval)
                                                      : Seconds(m_random->GetValue()),
                                      &INC_ConsumerBaseApp::SendPacket, this);
}

void
ConsumerApp::SetRandomize(const std::string& value)
{
  if (value == "uniform") {
    m_random = CreateObject<UniformRandomVariable>();
    m_random->SetAttribute("Min", DoubleValue(0.0));
    m_random->SetAttribute("Max", DoubleValue(2 * m_interval));
  }
  else if (value == "exponential") {
    m_random = CreateObject<ExponentialRandomVariable>();
    m_random->SetAttribute("Mean", DoubleValue(m_interval));
    m_random->SetAttribute("Bound", DoubleValue(50 * m_interval));
  }
  else
    m_random = 0;

  m_randomType = value;
}

std::string
ConsumerApp::GetRandomize() const
{
  return m_randomType;
}

} //namespace inc
} // namespace ndn
} // namespace ns3
