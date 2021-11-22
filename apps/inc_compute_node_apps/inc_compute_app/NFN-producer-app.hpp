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
 * Based on the ndn-producer implementation of ns3
 * https://ndnsim.net/current/doxygen/classns3_1_1ndn_1_1Producer.html
 * Originated from ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 *          Liming Liu <fixed-term.liming.liu@de.bosch.com>
 * *****************************************************************************
 */

#ifndef NFN_PRODUCER_APP_H
#define NFN_PRODUCER_APP_H

#include <bits/stdint-uintn.h>
#include <memory>

#include "ns3/inc-orchestration-compute-node.h"
#include "ns3/INC-Computation.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/ndn-cxx/data.hpp"
#include "ns3/ndnSIM/ndn-cxx/interest.hpp"
#include "ns3/ndnSIM/ndn-cxx/name.hpp"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/NFN-interest-resolution-engine.hpp"
#include "ns3/simple-ref-count.h"
#include <queue>
#include "model/null-transport.hpp"

#define MAX_QUEUE_SIZE 10
#define QUEUE_ACTION "forward"

namespace ns3{
  namespace ndn{
    namespace inc{
      /**
       * @ingroup ndn-apps
       * @brief  A simple Interest-sink application, which replying every incoming Interest with Data packet with a specified
       * size and name same as in Interest.
       */
      class NfnProducerApp : public App
      {
        //A struct to support multi-interest handling
        //Each new interest is maintained as a InterestComponentStructure before execution finished
        struct InterestComponentStruct{
          Name m_interest;
          Ptr<INC_Computation> m_func;
          std::unordered_map<Name, bool> m_pending_args;
          shared_ptr<const Interest> interest_ptr;
          bool operator == (const InterestComponentStruct &rhs) const
          {
            return m_interest.toUri()==rhs.m_interest.toUri();
          }
        };

        public:

        static TypeId
        GetTypeId(void);

        NfnProducerApp();
        ~NfnProducerApp();

        // inherited from App
        virtual void
        OnInterest(shared_ptr<const Interest> interest);

        // inherated from App
        virtual void
        OnData(shared_ptr<const Data> contentObject);

        typedef void ( *SendInterestTracedCallback)(shared_ptr<const Interest> interest);
        typedef void ( *RcvInterestTracedCallback)(shared_ptr<const Interest> interest);
        typedef void ( *IncomingDatasCallback)(shared_ptr<const Data>);
        typedef void ( *OutgoingDatasCallback)(shared_ptr<const Data>);
        typedef void ( *NodeBusyCallback)(std::string);
        typedef void ( *FuncDisabledCallback)(std::string);
        typedef void ( *nfnFuncEnabledCallback)(std::string);
        typedef void ( *nfnForwardInterestCallback)(std::string);
        typedef void ( *nfnQueueInterestCallback)(std::string);
        typedef void ( *FuncExecutingCallback)(std::vector<std::string>);

        //resolve the interest with the help of resolution engine and handle the returned decisions
    	  virtual void
    	  ResolveAndHandleDecisions(InterestComponentStruct ics);

      protected:
        // inherited from Application base class.
        virtual void
        StartApplication(); // Called at time specified by Start

        virtual void
        StopApplication(); // Called at time specified by Stop

      private:

        //release resource occupied by function execution
        void
	      ReleaseResource (Ptr<INC_Computation> func);

        //book resource occupied by function execution
        void
	      BookResource(Ptr<INC_Computation> func);

        //check resource needed by function execution
        bool
	      CheckResource(Ptr<INC_Computation> func);

        //execute m_func, do resource management and cleanup
        void
	      DoExecution(InterestComponentStruct entry);

        //cleanup an InterestComponentStruct entry in m_pending_content_table when a interest finishes its local lifecycle
        void
	      DeletePendingContentTableEntry(InterestComponentStruct entry);

        //a helper function for sending data
	      shared_ptr<Data>
	      PrepareDataPacket(Name name, uint32_t payload_size, ns3::Time freshness_period);

        void
        ForwardInterest(InterestComponentStruct t_ics);

        /**
         * @brief  in simulation scenario function execution takes almost 0 time,
         * however we want to simulate INC computations that may take a specific period of time.
         * We can achieve this effect by intentionally delay the response to Interest consumer( delay time equals
         * the function execution time) and pretent the producer is busy executing the function
         */
        void
	      ScheduledExecutionEnd(shared_ptr<Data> data,Ptr<INC_Computation> func);

        Name m_prefix;
        Name m_postfix;
        uint32_t m_virtualPayloadSize;
        Time m_freshness;
        uint32_t m_queue_size;
        // m_pending_content_table is used to maintain interests information while they are being handled by producer
        vector<InterestComponentStruct> m_pending_content_table;
        //used to keep track of interest sent to network, avoid repetitive sending when multiple interests request for same data simultaneously
        //boolean flag set to true when the interest has been sent
        unordered_set<Name> m_sending_interest_list;
        std::queue<InterestComponentStruct> m_waiting_list;
        uint32_t m_signature;
        Name m_keyLocator;
        NfnInterestResolutionEngine m_engine;

        //Pointer to the compute node, which contains the ns3::Node instance the app is installed on
        Ptr<IncOrchestrationComputeNode> m_compute_node;

        TracedCallback<shared_ptr<const Interest>> 	m_onIncomingInterestTrace;   			///< @brief trace of incoming interests
        TracedCallback<shared_ptr<const Interest>> 	m_onOutgoingInterestTrace;   			///< @brief trace of incoming interests
        TracedCallback<shared_ptr<const Data>> 		m_onOutgoingDataTrace;              ///< @brief trace of outgoing data
        TracedCallback<shared_ptr<const Data>> m_onIncomingDataTrace;                 ///< @brief trace of incoming data
        TracedCallback<std::string> m_onNodeBusyTrace;
        TracedCallback<std::string> m_onFuncDisabledTrace;
        TracedCallback<std::string> m_nfnFuncEnabledTrace;
        TracedCallback<std::string> m_onForwardInterestTrace;
        TracedCallback<std::string> m_onQueueInterestTrace;
        TracedCallback<std::vector<std::string>> m_onFuncExecutionTrace;
      };
    }//namespace inc
  } // namespace ndn
} // namespace ns3

#endif // NDN_PRODUCER_H
