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

#include "orchestration-management-app.hpp"

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
#include "ns3/ndnSIM/model/ndn-global-router.hpp"
#include "ns3/ndnSIM/helper/boost-graph-ndn-global-routing-helper.hpp"
#include "ns3/node-list.h"
#include "ns3/random-variable-stream.h"

NS_LOG_COMPONENT_DEFINE ("ndn.inc.OrchestrationManagementApp");

namespace ns3 {
  namespace ndn {
    namespace inc {
      NS_OBJECT_ENSURE_REGISTERED (OrchestrationManagementApp);

        // register NS-3 type
    TypeId
    OrchestrationManagementApp::GetTypeId ()
    {
      static TypeId tid =
          TypeId ("ns3::ndn::inc::OrchestrationManagementApp")
              .SetParent<Application> ()
              .AddConstructor<OrchestrationManagementApp> ()
              .AddAttribute ("ExecutionTime", "Time at which orchestration command is sent",
                             TimeValue (Seconds (0)),
                             MakeTimeAccessor (&OrchestrationManagementApp::m_ExeTime),
                             MakeTimeChecker ())
              .AddAttribute ("Interval", "Interval of orchestration", TimeValue (Seconds (1.0)),
                             MakeTimeAccessor (&OrchestrationManagementApp::m_interval),
                             MakeTimeChecker ())
              .AddAttribute ("Periodic", "Periodic interests?", BooleanValue (false),
                             MakeBooleanAccessor (&OrchestrationManagementApp::m_periodic),
                             MakeBooleanChecker ())
              .AddAttribute ("OrchestrationStrategy",
                             "Based on how the inc compute functions are orchestrated",
                             StringValue (""),
                             MakeStringAccessor (&OrchestrationManagementApp::m_orchestration_strategy),
                             MakeStringChecker ())
              .AddAttribute ("CommunicationModel", "In-Band/Out-of-Band", StringValue (""),
                             MakeStringAccessor (&OrchestrationManagementApp::m_communication_model),
                             MakeStringChecker ());
      return tid;
    }

    OrchestrationManagementApp::OrchestrationManagementApp ()
    {
      NS_LOG_FUNCTION (this);
      //do nothing
    }

    OrchestrationManagementApp::~OrchestrationManagementApp ()
    {
      //do nothing
    }

    void
    OrchestrationManagementApp::StartApplication ()
    {

      if (m_periodic == false)
        {
          Simulator::Schedule (m_ExeTime, &OrchestrationManagementApp::orchestrate, this);
        }
      else
        {
          m_first_time = true;
          ScheduleNextPacket ();
        }
    }

    void
    OrchestrationManagementApp::ScheduleNextPacket ()
    {
      if (m_first_time)
        {
          Simulator::Schedule (m_ExeTime, &OrchestrationManagementApp::orchestrate, this);
        }
      else
        {
          Simulator::Schedule (m_interval, &OrchestrationManagementApp::orchestrate, this);
        }
    }

    void
    OrchestrationManagementApp::StopApplication ()
    {
      NS_LOG_FUNCTION (this);
    }

    void
    OrchestrationManagementApp::orchestrate ()
    {
      if(m_first_time)
      {
        m_storage_handler.calculate_hop_distance();
        m_first_time = false;
      }
      if (m_orchestration_strategy.compare ("Function Switch") == 0)
        {
          FunctionSwitchHandler ();
        }

      if(m_orchestration_strategy.compare("Heuristics")==0)
        {

        }

      if (m_periodic == true)
        ScheduleNextPacket ();
    }

    void
    OrchestrationManagementApp::FunctionSwitchHandler ()
    {
      NS_LOG_INFO("Inside Function Switch Handler at time " << Simulator::Now());
      bool enable_status, disable_flag, enable_flag;
      m_excluded_list.clear ();
      uint32_t execution_counter, node_busy_counter;
      std::string orchestration_control_enable_interest;
      std::string orchestration_control_disable_interest;
      std::vector<std::string> busy_node_functions;

      //disable function from busy nodes
      for (std::map<std::string,
                    ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_iterator iter =
               m_storage_handler.cbegin ();
           iter != m_storage_handler.cend (); ++iter)
           {
             int sum_exe_counter=0;
             for(auto f_it = (iter->second).functions_current.begin(); f_it != (iter->second).functions_current.end(); f_it++)
             {
               sum_exe_counter = sum_exe_counter + f_it->func_exe_counter;
             }
             if(sum_exe_counter > ((m_interval/3.0)*(iter->second).cpu.first)*(4/5))
             {
               orchestration_control_disable_interest = "";
               disable_flag = false;
               orchestration_control_disable_interest = ("/Orchestrator" + iter->first + "/FunctionSwitch/Disable");

               for (auto func_iter = (iter->second.functions_current).begin ();
               func_iter != (iter->second.functions_current).end (); ++func_iter)
               {
                 enable_status=func_iter->status;
                 execution_counter = func_iter->func_exe_counter;
                 NS_LOG_INFO("execution count of func " <<func_iter->funcName.toUri() << "  in node "<<iter->first <<" is "<<execution_counter);
                 if((enable_status == true) && (execution_counter == 0))
                 {
                   orchestration_control_disable_interest.append (func_iter->funcName.toUri ()); //disable function at this node
                   disable_flag = true;
                   //busy_node_functions.push_back(func_iter->funcName.toUri ());
                 }
                 if((enable_status == true) && (execution_counter > 8))
                 {
                    busy_node_functions.push_back(func_iter->funcName.toUri ());
                 }
               }
               Ptr<Node> Current_Node = this->GetNode ();
               if (m_communication_model.compare ("In-Band") == 0)
                {
                  Ptr<ndn::inc::NdnOrchestrationCommunicationApp> m_comm_handler =  Current_Node->GetApplication (0)
                      ->GetObject<ndn::inc::NdnOrchestrationCommunicationApp> ();
                  if (disable_flag == true)
                    m_comm_handler->SendOrchestrationRequest (orchestration_control_disable_interest);
                }
               if (m_communication_model.compare ("Out-of-Band") == 0)
                {
                  Ptr<ndn::inc::UdpOrchestrationCommunicationApp> m_comm_handler =
                    Current_Node->GetApplication (0)
                       ->GetObject<ndn::inc::UdpOrchestrationCommunicationApp> ();
                  if (disable_flag == true)
                    m_comm_handler->SendOnceWithFill (orchestration_control_disable_interest);
                }
             }
           }

        //enabling functions in nodes which only forward interest i.e.; total execution counter =0;
        for(std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_reverse_iterator it = m_storage_handler.crbegin (); it != m_storage_handler.crend (); ++it)
        {
          bool enable_flag = false;
          orchestration_control_enable_interest = ("/Orchestrator" + it->first + "/FunctionSwitch/Enable");
          int sum_exe_counter=0;
          for(auto f_it = (it->second).functions_current.begin(); f_it != (it->second).functions_current.end(); f_it++)
          {
            sum_exe_counter = sum_exe_counter + f_it->func_exe_counter;
            bool enable_status;
            int missed_execution_counter;
            enable_status = f_it->status;
            missed_execution_counter = f_it->func_interest_counter;
            if(enable_status == false && (missed_execution_counter>5))
            {
              orchestration_control_enable_interest.append (f_it->funcName.toUri());
              enable_flag = true;
            }
          }
          if((sum_exe_counter==0) || (sum_exe_counter < ((m_interval/3.0)*(it->second).cpu.first)/2)) //enabling functions in nodes that only forward interests or is only half utilized
          {
            for(auto f_it = (it->second).functions_current.begin(); f_it != (it->second).functions_current.end(); f_it++)
             {
               if((f_it->status==false) && (std::find(busy_node_functions.begin(), busy_node_functions.end(), f_it->funcName.toUri())!=busy_node_functions.end()))
               {
                orchestration_control_enable_interest.append (f_it->funcName.toUri());
                enable_flag = true;
               }
             }
          }
            Ptr<Node> Current_Node = this->GetNode ();
            if (m_communication_model.compare ("In-Band") == 0)
              {
                Ptr<ndn::inc::NdnOrchestrationCommunicationApp> m_comm_handler = Current_Node->GetApplication (0)->GetObject<ndn::inc::NdnOrchestrationCommunicationApp> ();
                if (enable_flag == true)
                m_comm_handler->SendOrchestrationRequest (orchestration_control_enable_interest);
              }
            if (m_communication_model.compare ("Out-of-Band") == 0)
              {
                Ptr<ndn::inc::UdpOrchestrationCommunicationApp> m_comm_handler = Current_Node->GetApplication (0)->GetObject<ndn::inc::UdpOrchestrationCommunicationApp> ();
                if (enable_flag == true)
                m_comm_handler->SendOnceWithFill (orchestration_control_enable_interest);
              }
        }
        //enable functions in nodes whose missed execution counter is greater than a threshold and function status is disabled
        for (std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_reverse_iterator iter =
               m_storage_handler.crbegin (); iter != m_storage_handler.crend (); iter++)
        {
          bool enable_flag = false;
          orchestration_control_enable_interest = ("/Orchestrator" + iter->first + "/FunctionSwitch/Enable");
          for(auto f_it = (iter->second).functions_current.begin(); f_it != (iter->second).functions_current.end(); f_it++)
          {
            bool enable_status;
            int missed_execution_counter;
            enable_status = f_it->status;
            missed_execution_counter = f_it->func_interest_counter;
            if(enable_status == false && missed_execution_counter>10)
            {
              orchestration_control_enable_interest.append (f_it->funcName.toUri());
              enable_flag = true;
            }
          }
          Ptr<Node> Current_Node = this->GetNode ();
          if (m_communication_model.compare ("In-Band") == 0)
          {
             Ptr<ndn::inc::NdnOrchestrationCommunicationApp> m_comm_handler = Current_Node->GetApplication (0)->GetObject<ndn::inc::NdnOrchestrationCommunicationApp> ();
             if (enable_flag == true)
             m_comm_handler->SendOrchestrationRequest (orchestration_control_enable_interest);
          }
          if (m_communication_model.compare ("Out-of-Band") == 0)
          {
             Ptr<ndn::inc::UdpOrchestrationCommunicationApp> m_comm_handler = Current_Node->GetApplication (0)->GetObject<ndn::inc::UdpOrchestrationCommunicationApp> ();
             if (enable_flag == true)
               m_comm_handler->SendOnceWithFill (orchestration_control_enable_interest);
          }
        }
    }

    bool
    OrchestrationManagementApp::CheckAtleastOnceEnabled (std::string compute_node_name,
                                                         std::string func_name)
    {
      for (std::map<std::string,
                    ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_iterator iter =
               m_storage_handler.cbegin ();
           iter != m_storage_handler.cend (); iter++)
        {
          for (auto func_iter = (iter->second.functions_current).begin ();
               func_iter != (iter->second.functions_current).end (); ++func_iter)
            {
              std::vector<std::pair<std::string, std::string>>::iterator it;
              it = std::find (m_excluded_list.begin (), m_excluded_list.end (),
                              std::make_pair (iter->first, func_name));
              if ((func_iter->funcName == func_name) // if the function is same
                  && (iter->first.compare (compute_node_name) != 0) //if node is different
                  && (func_iter->status == true) //if function is enabled
                  && (it == m_excluded_list.end ()) //if not already on exclude list
                  )
                {
                  m_excluded_list.push_back (std::make_pair (compute_node_name, func_name));
                  return true;
                }
            }
        }
      return false;
    }

    } // namespace inc
  } // namespace ndn
} // namespace ns3
