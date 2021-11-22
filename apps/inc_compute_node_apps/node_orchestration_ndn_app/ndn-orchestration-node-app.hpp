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
 *          Liming Liu <fixed-term.liming.liu@de.bosch.com>
 * *****************************************************************************
 */

#ifndef NDN_ORCHESTRATOR_NODE_APP_H_
#define NDN_ORCHESTRATOR_NODE_APP_H_
#include "ns3/incSIM-module.h"
#include "ns3/ndnSIM/apps/ndn-app.hpp"

namespace ns3{
namespace ndn{
namespace inc{
    class NdnOrchestrationComputeNodeApp : public App
    {
      /*
      Orchestrator provider application to be installed on nodes in order to control the enable status of the compute functions on them
      */
    public:
      NdnOrchestrationComputeNodeApp();
      // register NS-3 type "CustomApp"
      static TypeId
      GetTypeId();

      // (overridden from ndn::App) Processing upon start of the application
      virtual void
      StartApplication();

      // (overridden from ndn::App) Processing when application is stopped
      virtual void
      StopApplication();

      // (overridden from ndn::App) Callback that will be called when Data arrives
      virtual void
      OnInterest(shared_ptr<const Interest> interest);

      Ptr<IncOrchestrationComputeNode> GetComputeNode(void);
      void SetComputeNode(Ptr<IncOrchestrationComputeNode> cn);

      void
      OrchestratorRequestResolution(Name m_interest);

      std::string
      StatusFetchHandler(Name interest);

      std::string
      FunctionStatusRequestHandler(Name interest);

      std::string
      FunctionSwitchHandler(Name m_interest);

      ~NdnOrchestrationComputeNodeApp();
      typedef void ( *EnableFunctionCallback)(std::string);
      typedef void ( *DisableFunctionCallback)(std::string);

    private:
      Name m_prefix;
      uint32_t m_virtualPayloadSize;
      Time m_freshness;
      Name m_interest;
      uint32_t m_signature;
      Name m_keyLocator;
      //Pointer to the compute node, which contains the ns3::Node instance the app is installed on
      Ptr<IncOrchestrationComputeNode> m_compute_node;
      TracedCallback<std::string> m_onFuncEnableTrace;
      TracedCallback<std::string> m_onFuncDisableTrace;


    };
   }//namespace inc
  } // namespace ndn
} // namespace ns3
#endif
