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

#include "ndn-orchestration-communication-app.hpp"

#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/incSIM-module.h"
#include "ns3/node-info-storage.hpp"
#include "ns3/orchestration-management-app.hpp"

#include "ns3/random-variable-stream.h"

NS_LOG_COMPONENT_DEFINE("ndn.inc.NdnOrchestrationCommunicationApp");

namespace ns3{
namespace ndn{
namespace inc{
    NS_OBJECT_ENSURE_REGISTERED(NdnOrchestrationCommunicationApp);

    // register NS-3 type
    TypeId
    NdnOrchestrationCommunicationApp::GetTypeId()
    {
      static TypeId tid =
          TypeId("ns3::ndn::inc::NdnOrchestrationCommunicationApp")
              .SetParent<Consumer>()
              .AddConstructor<NdnOrchestrationCommunicationApp>()
              .AddAttribute("ExecutionTime", "Time at which orchestration command is sent", TimeValue(Seconds(0)),
                            MakeTimeAccessor(&NdnOrchestrationCommunicationApp::m_ExeTime), MakeTimeChecker())
              .AddAttribute ("Interval", "The time to wait between packets", TimeValue (Seconds (1.0)),
                         MakeTimeAccessor (&NdnOrchestrationCommunicationApp::m_interval),
                         MakeTimeChecker ())
              .AddAttribute("Periodic", "Periodic interests?", BooleanValue(false),
                      MakeBooleanAccessor(&NdnOrchestrationCommunicationApp::m_periodic), MakeBooleanChecker())
              .AddTraceSource("SendOrchInterests", "Trace called every time there is an Interest packet is send to the network",
                      MakeTraceSourceAccessor(&NdnOrchestrationCommunicationApp::m_OrchestrationInterestTrace),
                      "ns3::ndn::inc::NdnOrchestrationCommunicationApp::SendsOrchestrstionInterestTracedCallback")
              .AddTraceSource("IncomingOrchDatas", "Trace called every time there is an incoming Data packet",
                      MakeTraceSourceAccessor(&NdnOrchestrationCommunicationApp::m_onOrchestrationDataTrace),
                      "ns3::ndn::inc::NdnOrchestrationCommunicationApp::IncomingDatasCallback")
              .AddTraceSource("SendOrchDecisions", "Trace called every time there is an outgoing decision",
                      MakeTraceSourceAccessor(&NdnOrchestrationCommunicationApp::m_OrchestrationDecisionTrace),
                      "ns3::ndn::inc::NdnOrchestrationCommunicationApp::SendsOrchestrationDecisionTracedCallback");
      return tid;
    }

    NdnOrchestrationCommunicationApp::NdnOrchestrationCommunicationApp()
    {
      NS_LOG_FUNCTION_NOARGS();
    }

    NdnOrchestrationCommunicationApp::~NdnOrchestrationCommunicationApp(){}

    // Processing upon start of the application
    void
    NdnOrchestrationCommunicationApp::StartApplication()
    {
      // initialize ndn::App
      ndn::App::StartApplication();
      NS_LOG_FUNCTION_NOARGS();
      // Schedule send of first interest
      if (m_periodic==false)
      {
        Simulator::Schedule(m_ExeTime, &NdnOrchestrationCommunicationApp::SendInterest, this);
      }
      else
      {
        m_first_time =true;
        ScheduleNextPacket();
      }
    }

    // Processing when application is stopped
    void
    NdnOrchestrationCommunicationApp::StopApplication()
    {
      // cleanup ndn::App
      ndn::App::StopApplication();
    }

    void
    NdnOrchestrationCommunicationApp::ScheduleNextPacket()
    {
      if(m_first_time)
      {
        Simulator::Schedule(m_ExeTime, &NdnOrchestrationCommunicationApp::SendInterestPeriodic, this);
        m_first_time = false;
      }
      else
      {
        Simulator::Schedule(m_interval, &NdnOrchestrationCommunicationApp::SendInterestPeriodic, this);
      }

    }

    void
    NdnOrchestrationCommunicationApp::SendOrchestrationRequest(std::string prefix)
    {
      Name interest_prefix = prefix;
      shared_ptr<Name> orchestration_interest = make_shared<Name>(interest_prefix);
      shared_ptr<Interest> interest = make_shared<Interest>(interest_prefix);
      interest->setName(*orchestration_interest);
      interest->setCanBePrefix(false);
      interest->setMustBeFresh(true);
      NS_LOG_INFO("[Orchestrator Communication App] Sending Interest: " << *interest);
      m_transmittedInterests(interest, this, m_face);
      m_appLink->onReceiveInterest(*interest);
      this->m_OrchestrationDecisionTrace(interest);
    }

    void
    NdnOrchestrationCommunicationApp::SendInterest()
    {
      //---------------------------------------//
      //----Sending one Interest packet out----//
      //---------------------------------------//
      //Name interest_prefix = m_prefix.toUri() + m_NodeName.toUri() + m_FuncName.toUri() + m_enableStatus.toUri();
      Name interest_prefix = Consumer::m_interestName.toUri();
      NS_LOG_INFO("[Orchestrator Communication App] interest prefix "<<interest_prefix);
      shared_ptr<Name> orchestration_interest = make_shared<Name>(interest_prefix);
      shared_ptr<Interest> interest = make_shared<Interest>(interest_prefix);
      interest->setName(*orchestration_interest);
      interest->setCanBePrefix(false);
      interest->setMustBeFresh(true);
      NS_LOG_INFO("[Orchestrator Communication App] Sending Interest: " << *interest);
      m_transmittedInterests(interest, this, m_face);
      m_appLink->onReceiveInterest(*interest);
      this->m_OrchestrationInterestTrace(interest);
    }

    void
    NdnOrchestrationCommunicationApp::SendInterestPeriodic()
    {
      Name interest_prefix = Consumer::m_interestName.toUri();
      NS_LOG_INFO("[Orchestrator Communication App] interest prefix "<< interest_prefix);
      shared_ptr<Name> orchestration_interest = make_shared<Name>(interest_prefix);
      shared_ptr<Interest> interest = make_shared<Interest>(interest_prefix);
      interest->setName(*orchestration_interest);
      interest->setCanBePrefix(false);
      interest->setMustBeFresh(true);
      m_transmittedInterests(interest, this, m_face);
      m_appLink->onReceiveInterest(*interest);
      this->m_OrchestrationInterestTrace(interest);
      ScheduleNextPacket();
    }

    // Callback that will be called when Data arrives
    void
    NdnOrchestrationCommunicationApp::OnData(std::shared_ptr<const ndn::Data> data)
    {
      std::string actionToDo;
      std::string dataName = (data->getName()).toUri();
      this->m_onOrchestrationDataTrace(data);
      const char *buffer = (char*) data->getContent().value();
      std::string stringBuffer(buffer);
      std::string content = stringBuffer.substr(0, data->getContent().value_size());
      if(Name(dataName).getSubName(0,1)== "/Orchestrator")
      {
        if(Name(dataName).getSubName(2,1) == "/BootstrapInfo")
        {
          actionToDo = "/BootstrapInfo";
        }
        else if(Name(dataName).getSubName(2,1) == "/NodeStatusFetch")
        {
          actionToDo = "/NodeStatusFetch";
        }
        else
        {
          actionToDo  = "/Others";
        }
      }
      else
      {
        actionToDo = "/Others";
      }
      m_message_handler.HandleMessage(actionToDo, content);
    }

    }//namespace inc
  } // namespace ndn
} // namespace ns3
