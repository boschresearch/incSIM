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

#ifndef INC_CONSUMER_H
#define INC_CONSUMER_H
#include "ns3/inc-orchestration-compute-node.h"
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/ndnSIM/apps/ndn-app.hpp"

#include "ns3/random-variable-stream.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/utils/ndn-rtt-estimator.hpp"

#include <set>
#include <map>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

namespace ns3 {
namespace ndn {
namespace inc {

/**
 * @ingroup ndn-apps
 * \brief NDN application for sending out Interest packets
 */
class INC_ConsumerBaseApp : public App {
public:
  static TypeId
  GetTypeId();

  /**
   * \brief Default constructor
   * Sets up randomizer function and packet sequence number
   */
  INC_ConsumerBaseApp();
  virtual ~INC_ConsumerBaseApp(){};

  // From App
  virtual void
  OnData(shared_ptr<const Data> contentObject);

  // From App
  virtual void
  OnNack(shared_ptr<const lp::Nack> nack);

  /**
   * @brief Timeout event
   * @param sequenceNumber time outed sequence number
   */
  virtual void
  OnTimeout(uint32_t sequenceNumber);

  /**
   * @brief Actually send packet
   */
  void
  SendPacket();

  /**
   * @brief An event that is fired just before an Interest packet is actually send out (send is
   *inevitable)
   *
   * The reason for "before" even is that in certain cases (when it is possible to satisfy from the
   *local cache),
   * the send call will immediately return data, and if "after" even was used, this after would be
   *called after
   * all processing of incoming data, potentially producing unexpected results.
   */
  virtual void
  WillSendOutInterest(uint32_t sequenceNumber);

public:
  typedef void (*LastRetransmittedInterestDataDelayCallback)(Ptr<App> app, uint32_t seqno, Time delay, int32_t hopCount);
  typedef void (*FirstInterestDataDelayCallback)(Ptr<App> app, uint32_t seqno, Time delay, uint32_t retxCount, int32_t hopCount);
  typedef void ( *IncSendsInterestTracedCallback)(shared_ptr<const Interest> interest);
  typedef void ( *IncIncomingDatasTracedCallback)(shared_ptr<const Data>);

protected:
  // from App
  virtual void
  StartApplication();

  virtual void
  StopApplication();

  /**
   * \brief Constructs the Interest packet and sends it using a callback to the underlying NDN
   * protocol
   */
  virtual void
  ScheduleNextPacket() = 0;

  /**
   * \brief Checks if the packet need to be retransmitted becuase of retransmission timer expiration
   */
  void
  CheckRetxTimeout();

  /**
   * \brief Modifies the frequency of checking the retransmission timeouts
   * \param retxTimer Timeout defining how frequent retransmission timeouts should be checked
   */
  void
  SetRetxTimer(Time retxTimer);

  /**
   * \brief Returns the frequency of checking the retransmission timeouts
   * \return Timeout defining how frequent retransmission timeouts should be checked
   */
  Time
  GetRetxTimer() const;



protected:
  Ptr<UniformRandomVariable> m_rand; ///< @brief nonce generator

  uint32_t m_seq;      ///< @brief currently requested sequence number
  uint32_t m_seqMax;   ///< @brief maximum number of sequence number
  EventId m_sendEvent; ///< @brief EventId of pending "send packet" event
  Time m_retxTimer;    ///< @brief Currently estimated retransmission timer
  EventId m_retxEvent; ///< @brief Event to check whether or not retransmission should be performed
  bool m_compute_flag;
  Ptr<IncOrchestrationComputeNode> m_compute_node;
  Ptr<RttEstimator> m_rtt; ///< @brief RTT estimator

  Time m_offTime;          ///< \brief Time interval between packets
  Name m_interestName;     ///< \brief NDN Name of the Interest (use Name)
  Time m_interestLifeTime; ///< \brief LifeTime for interest packet
  std::pair<Name, Time> SentTime;
  std::pair<Name, Time> RecvTime;
  /// @cond include_hidden
  /**
   * \struct This struct contains sequence numbers of packets to be retransmitted
   */
  struct RetxSeqsContainer : public std::set<uint32_t> {
  };

  RetxSeqsContainer m_retxSeqs; ///< \brief ordered set of sequence numbers to be retransmitted

  /**
   * \struct This struct contains a pair of packet sequence number and its timeout
   */
  struct SeqTimeout {
    SeqTimeout(uint32_t _seq, Time _time)
      : seq(_seq)
      , time(_time)
    {
    }

    uint32_t seq;
    Time time;
  };
  /// @endcond

  /// @cond include_hidden
  class i_seq {
  };
  class i_timestamp {
  };
  /// @endcond

  /// @cond include_hidden
  /**
   * \struct This struct contains a multi-index for the set of SeqTimeout structs
   */
  struct SeqTimeoutsContainer
    : public boost::multi_index::
        multi_index_container<SeqTimeout,
                              boost::multi_index::
                                indexed_by<boost::multi_index::
                                             ordered_unique<boost::multi_index::tag<i_seq>,
                                                            boost::multi_index::
                                                              member<SeqTimeout, uint32_t,
                                                                     &SeqTimeout::seq>>,
                                           boost::multi_index::
                                             ordered_non_unique<boost::multi_index::
                                                                  tag<i_timestamp>,
                                                                boost::multi_index::
                                                                  member<SeqTimeout, Time,
                                                                         &SeqTimeout::time>>>> {
  };

  SeqTimeoutsContainer m_seqTimeouts; ///< \brief multi-index for the set of SeqTimeout structs

  SeqTimeoutsContainer m_seqLastDelay;
  SeqTimeoutsContainer m_seqFullDelay;
  std::map<uint32_t, uint32_t> m_seqRetxCounts;

  TracedCallback<Ptr<App> /* app */, uint32_t /* seqno */, Time /* delay */, int32_t /*hop count*/>
    m_lastRetransmittedInterestDataDelay;
  TracedCallback<Ptr<App> /* app */, uint32_t /* seqno */, Time /* delay */,
                 uint32_t /*retx count*/, int32_t /*hop count*/> m_firstInterestDataDelay;

  TracedCallback<shared_ptr<const Interest>> 	m_interestTrace;   			///< @brief trace of outgoing interests
  TracedCallback<shared_ptr<const Data>> 		m_onDataTrace;         		///< @brief trace of incoming data
  /// @endcond
};

} //namespace inc
} // namespace ndn
} // namespace ns3

#endif
