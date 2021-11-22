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
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 * *****************************************************************************
 */

#ifndef NDN_ORCHESTRATOR_COMMUNICATION_APP_H_
#define NDN_ORCHESTRATOR_COMMUNICATION_APP_H_

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer.hpp"
#include "ns3/message_handler.hpp"

namespace ns3{
namespace ndn{
namespace inc{
    class NdnOrchestrationCommunicationApp : public Consumer
    {
      /*
       Orchestrator Consumer application to be installed on the orchestrator nodes in order to control the enable status
       of the compute functions on them by sending the approipriate interest messages.
      */
    public:
      // register NS-3 type "CustomApp"
      static TypeId
      GetTypeId();

      NdnOrchestrationCommunicationApp();
      ~NdnOrchestrationCommunicationApp();
      // (overridden from ndn::App) Processing upon start of the application
      virtual void
      StartApplication();

      // (overridden from ndn::App) Processing when application is stopped
      virtual void
      StopApplication();

      // (overridden from ndn::App) Callback that will be called when Data arrives
      virtual void
      OnData(std::shared_ptr<const ndn::Data> contentObject);

      void
      SendOrchestrationRequest(std::string);

      void
      SendInterestPeriodic();

      void
      SendInterest();

      //overridden from ndn::consumer for periodic interests
      void
      ScheduleNextPacket();

      typedef void ( *SendsInterestTracedCallback)(shared_ptr<const Interest> interest);
      typedef void ( *SendsDecisionTracedCallback)(shared_ptr<const Interest> interest);
      typedef void ( *IncomingDatasCallback)(shared_ptr<const Data>);


      TracedCallback<shared_ptr<const Interest>> 	m_OrchestrationInterestTrace;   			///< @brief trace of incoming interests
      TracedCallback<shared_ptr<const Interest>> 	m_OrchestrationDecisionTrace;   			///< @brief trace of incoming interests
      TracedCallback<shared_ptr<const Data>> 		  m_onOrchestrationDataTrace;         		///< @brief trace of incoming data

    private:
      Time m_ExeTime;
      bool m_periodic;
      Time m_interval;
      bool m_first_time;
      OrchestrationMessageHandler m_message_handler;
    };
}//namespace inc
} // namespace ndn
} // namespace ns3


#endif
