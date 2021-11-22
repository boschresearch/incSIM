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
 * Based on the ndn-consumer implementation of ns3
 * https://ndnsim.net/current/doxygen/classns3_1_1ndn_1_1Consumer.html
 * Originated from ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 * *****************************************************************************
 */

#include "inc-consumer-base-app.hpp"
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

NS_LOG_COMPONENT_DEFINE("inc.INCConsumerBaseApp");

namespace ns3 {
namespace ndn {
namespace inc {

NS_OBJECT_ENSURE_REGISTERED(INC_ConsumerBaseApp);

TypeId
INC_ConsumerBaseApp::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::inc::INC_ConsumerBaseApp")
      .SetGroupName("Inc")
      .SetParent<App>()
      .AddAttribute("StartSeq", "Initial sequence number", IntegerValue(0),
                    MakeIntegerAccessor(&INC_ConsumerBaseApp::m_seq), MakeIntegerChecker<int32_t>())
      .AddAttribute("Prefix", "Name of the Interest", StringValue("/"),
                    MakeNameAccessor(&INC_ConsumerBaseApp::m_interestName), MakeNameChecker())
      .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("20s"),
                    MakeTimeAccessor(&INC_ConsumerBaseApp::m_interestLifeTime), MakeTimeChecker())
      .AddAttribute("RetxTimer",
                    "Timeout defining how frequent retransmission timeouts should be checked",
                    StringValue("50ms"),
                    MakeTimeAccessor(&INC_ConsumerBaseApp::GetRetxTimer, &INC_ConsumerBaseApp::SetRetxTimer),
                    MakeTimeChecker())
      .AddAttribute ("ComputeNodePointer",
                     "the pointer of compute node to access functions",
                     PointerValue (),
                     MakePointerAccessor (&INC_ConsumerBaseApp::m_compute_node),
                     MakePointerChecker<IncOrchestrationComputeNode> ())
      .AddAttribute("ComputeNode",
                    "Is the consumer app on a compute node",
                    BooleanValue(false),
                    MakeBooleanAccessor(&INC_ConsumerBaseApp::m_compute_flag),
                    MakeBooleanChecker())
      .AddTraceSource("LastRetransmittedInterestDataDelayInc",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&INC_ConsumerBaseApp::m_lastRetransmittedInterestDataDelay),
                      "ns3::ndn::inc::INC_ConsumerBaseApp::LastRetransmittedInterestDataDelayCallback")
      .AddTraceSource("FirstInterestDataDelayInc",
                      "Delay between first transmitted Interest and received Data",
                      MakeTraceSourceAccessor(&INC_ConsumerBaseApp::m_firstInterestDataDelay),
                      "ns3::ndn::inc::INC_ConsumerBaseApp::FirstInterestDataDelayCallback")
      .AddTraceSource("SendInterestsInc",
                      "Trace called every time there is an Interest packet is send to the network",
                      MakeTraceSourceAccessor(&INC_ConsumerBaseApp::m_interestTrace),
                      "ns3::ndn::inc::INC_ConsumerBaseApp::IncSendsInterestTracedCallback")
      .AddTraceSource("IncomingDatasInc",
                      "Trace called every time there is an incoming Data packet",
                      MakeTraceSourceAccessor(&INC_ConsumerBaseApp::m_onDataTrace),
                      "ns3::ndn::inc::INC_ConsumerBaseApp::IncIncomingDatasCallback");
  return tid;
}

INC_ConsumerBaseApp::INC_ConsumerBaseApp()
  : m_rand(CreateObject<UniformRandomVariable>())
  , m_seq(0)
  , m_seqMax(0) // don't request anything
{
  NS_LOG_FUNCTION_NOARGS();

  m_rtt = CreateObject<RttMeanDeviation>();
}

void
INC_ConsumerBaseApp::SetRetxTimer(Time retxTimer)
{
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning()) {
    // m_retxEvent.Cancel (); // cancel any scheduled cleanup events
    Simulator::Remove(m_retxEvent); // slower, but better for memory
  }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule(m_retxTimer, &INC_ConsumerBaseApp::CheckRetxTimeout, this);
}

Time
INC_ConsumerBaseApp::GetRetxTimer() const
{
  return m_retxTimer;
}

void
INC_ConsumerBaseApp::CheckRetxTimeout()
{
  Time now = Simulator::Now();

  Time rto = m_rtt->RetransmitTimeout();
  // NS_LOG_DEBUG ("Current RTO: " << rto.ToDouble (Time::S) << "s");

  while (!m_seqTimeouts.empty()) {
    SeqTimeoutsContainer::index<i_timestamp>::type::iterator entry =
      m_seqTimeouts.get<i_timestamp>().begin();
    if (entry->time + rto <= now) // timeout expired?
    {
      uint32_t seqNo = entry->seq;
      m_seqTimeouts.get<i_timestamp>().erase(entry);
      OnTimeout(seqNo);
    }
    else
      break; // nothing else to do. All later packets need not be retransmitted
  }

  m_retxEvent = Simulator::Schedule(m_retxTimer, &INC_ConsumerBaseApp::CheckRetxTimeout, this);
}

// Application Methods
void
INC_ConsumerBaseApp::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS();
  // do base stuff
  App::StartApplication();

  ScheduleNextPacket();
}

void
INC_ConsumerBaseApp::StopApplication() // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS();

  // cancel periodic packet generation
  Simulator::Cancel(m_sendEvent);

  // cleanup base stuff
  App::StopApplication();
}

void
INC_ConsumerBaseApp::SendPacket()
{
  if (!m_active)
    return;

  NS_LOG_FUNCTION_NOARGS();
  uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

  while (m_retxSeqs.size()) {
    seq = *m_retxSeqs.begin();
    m_retxSeqs.erase(m_retxSeqs.begin());
    break;
  }

  if (seq == std::numeric_limits<uint32_t>::max()) {
    if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
      if (m_seq >= m_seqMax) {
        return; // we are totally done
      }
    }

    seq = m_seq++;
  }


  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
  nameWithSequence->appendSequenceNumber(seq);

  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
  interest->setInterestLifetime(interestLifeTime);
  interest->setMustBeFresh(true);

  NS_LOG_INFO("[ Consumer "<<this->GetNode()->GetId()<<"] Sending Interest: " << *interest<<std::endl);
  NS_LOG_INFO("> Interest for " << seq);

  WillSendOutInterest(seq);
  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);
  this->m_interestTrace(interest);
  ScheduleNextPacket();
}

///////////////////////////////////////////////////
//          Process incoming packets             //
///////////////////////////////////////////////////

void
INC_ConsumerBaseApp::OnData(shared_ptr<const Data> data)
{
  if (!m_active)
    return;

  // if(m_compute_flag==true)
  // {

  //   bool data_enable_flag = m_compute_node->EnableDataForCompute(data->getName().getSubName(1, 1).toUri());
  //   if(data_enable_flag)
  //   {
  //     Ptr<L3Protocol> L3protocol = m_compute_node->GetNode()->GetObject<L3Protocol>();
  //     shared_ptr<nfd::Forwarder> forwarder = L3protocol->getForwarder();
  //     nfd::fib::Fib& fib=forwarder->getFib();
  //     NS_LOG_INFO("[NFN producer: "<<m_compute_node->GetName()<<"] Print fib info of "<<m_compute_node->GetName()<<" at  "<<Simulator::Now ().GetSeconds ());
  //     for(auto it=fib.begin();it!=fib.end();it++){
  //  	  NS_LOG_INFO("	Prefix:  "<<it->getPrefix().toUri());
  //  	  int size=it->getNextHops().size();
  //  	  for(int i=0;i<size;i++){
  //  	  	NS_LOG_INFO("		Nexthop "<<i<<" :"<<it->getNextHops().at(i).getFace().getRemoteUri());
  //   	}
  //   }
  // }
  // }

  App::OnData(data); // tracing inside
  NS_LOG_FUNCTION(this << data);
  NS_LOG_INFO ("[Consumer "<< m_node->GetId()<<"]Received content object: " << boost::cref(*data));
  std::string provided_data = data->getName().getSubName(1,1).toUri();
  if(data->getName().getSubName(0,1)== "/Data" && m_compute_flag==true)
  {
    uint32_t data_size = data->wireEncode().size();
    provided_data.append(":"+ std::to_string(data_size));
    //m_compute_node->AddProvidedData(provided_data);
  }
  else
  {
    this->m_onDataTrace(data);
  }
  // This could be a problem......
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << seq);

  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  NS_LOG_INFO("Hop count: " << hopCount);

  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find(seq);
  if (entry != m_seqLastDelay.end()) {
    m_lastRetransmittedInterestDataDelay(this, seq, Simulator::Now() - entry->time, hopCount);
  }

  entry = m_seqFullDelay.find(seq);
  if (entry != m_seqFullDelay.end()) {
    m_firstInterestDataDelay(this, seq, Simulator::Now() - entry->time, m_seqRetxCounts[seq], hopCount);
  }

  m_seqRetxCounts.erase(seq);
  m_seqFullDelay.erase(seq);
  m_seqLastDelay.erase(seq);

  m_seqTimeouts.erase(seq);
  m_retxSeqs.erase(seq);

  m_rtt->AckSeq(SequenceNumber32(seq));
}

void
INC_ConsumerBaseApp::OnNack(shared_ptr<const lp::Nack> nack)
{
  /// tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

void
INC_ConsumerBaseApp::OnTimeout(uint32_t sequenceNumber)
{
  NS_LOG_FUNCTION(sequenceNumber);
  // std::cout << Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " <<
  // m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n";

  m_rtt->IncreaseMultiplier(); // Double the next RTO
  m_rtt->SentSeq(SequenceNumber32(sequenceNumber),
                 1); // make sure to disable RTT calculation for this sample
  m_retxSeqs.insert(sequenceNumber);
  ScheduleNextPacket();
}

void
INC_ConsumerBaseApp::WillSendOutInterest(uint32_t sequenceNumber)
{
  NS_LOG_DEBUG("Trying to add " << sequenceNumber << " with " << Simulator::Now() << ". already "
                                << m_seqTimeouts.size() << " items");

  m_seqTimeouts.insert(SeqTimeout(sequenceNumber, Simulator::Now()));
  m_seqFullDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqLastDelay.erase(sequenceNumber);
  m_seqLastDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqRetxCounts[sequenceNumber]++;

  m_rtt->SentSeq(SequenceNumber32(sequenceNumber), 1);
}

} //namespace inc
} // namespace ndn
} // namespace ns3
