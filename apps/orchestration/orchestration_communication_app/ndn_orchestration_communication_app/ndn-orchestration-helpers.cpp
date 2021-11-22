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
 * Based on the upd-echo-server-helper implementation of ns3
 * https://www.nsnam.org/doxygen/udp-echo-helper_8h_source.html
 * Originated by Mathieu Lacage <mathieu.lacage@sophia.inria.fr> under GPLv3
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 * *****************************************************************************
 */

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/names.h"
#include "ns3/ndn-orchestration-node-app.hpp"
#include "apps/ndn-app.hpp"

#ifdef NS3_MPI
#include "ns3/mpi-interface.h"
#endif

NS_LOG_COMPONENT_DEFINE("ndn.NdnOrchestrationHelper");

namespace ns3 {
namespace ndn {
namespace inc {

NdnOrchestrationHelper::NdnOrchestrationHelper(const std::string& app)
{
  m_factory.SetTypeId (NdnOrchestrationComputeNodeApp::GetTypeId ());
}

void
NdnOrchestrationHelper::SetPrefix(const std::string& prefix)
{
  m_factory.Set("Prefix", StringValue(prefix));
}

void
NdnOrchestrationHelper::SetAttribute(std::string name, const AttributeValue& value)
{
  m_factory.Set(name, value);
}

ApplicationContainer
NdnOrchestrationHelper::Install(Ptr<Node> node)
{
  ApplicationContainer apps;
  Ptr<Application> app = InstallPriv(node);
  if (app != 0)
    apps.Add(app);

  return apps;
}

ApplicationContainer
NdnOrchestrationHelper::Install(std::string nodeName)
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return Install(node);
}

ApplicationContainer
NdnOrchestrationHelper::Install (Ptr<Node> node, std::vector<std::string> function_enable_list)const
{
  Ptr<ns3::ndn::inc::NdnOrchestrationComputeNodeApp> app = m_factory.Create<ns3::ndn::inc::NdnOrchestrationComputeNodeApp> ();
  for(std::string t_func_name:function_enable_list){
    app->GetComputeNode()->EnableFunction(t_func_name);
  }
  node->AddApplication (app);
  return ApplicationContainer(app);
}

ApplicationContainer
NdnOrchestrationHelper::Install(NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
    Ptr<Application> app = InstallPriv(*i);
    if (app != 0)
      apps.Add(app);
  }

  return apps;
}

Ptr<Application>
NdnOrchestrationHelper::InstallPriv(Ptr<Node> node)
{
  Ptr<Application> app;
  Simulator::ScheduleWithContext(node->GetId(), Seconds(0), MakeEvent([=, &app] {
#ifdef NS3_MPI
        if (MpiInterface::IsEnabled() && node->GetSystemId() != MpiInterface::GetSystemId()) {
          // don't create an app if MPI is enabled and node is not in the correct partition
          return 0;
        }
#endif

        app = m_factory.Create<Application>();
        node->AddApplication(app);
      }));
  StackHelper::ProcessWarmupEvents();

  return app;
}

////////////////////////////////////////////////////////////////////////////

FactoryCallbackApp::FactoryCallbackApp(const FactoryCallback& factory)
  : m_factory(factory)
{
}

ApplicationContainer
FactoryCallbackApp::Install(Ptr<Node> node, const FactoryCallback& factory)
{
  ApplicationContainer apps;
  auto app = CreateObject<FactoryCallbackApp>(factory);
  node->AddApplication(app);
  apps.Add(app);
  return apps;
}

void
FactoryCallbackApp::StartApplication()
{
  m_impl = m_factory();
}

void
FactoryCallbackApp::StopApplication()
{
  m_impl.reset();
}

} //namespace inc
} // namespace ndn
} // namespace ns3
