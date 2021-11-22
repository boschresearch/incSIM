/*
 * ******************************************************************************
 * Copyright (c) 2021 Robert Bosch GmbH.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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

#include "udp-orchestration-helpers.hpp"
#include "ns3/udp-orchestration-node-app.hpp"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/incSIM-module.h"

namespace ns3{
namespace ndn{
namespace inc{
UdpOrchestrationServerHelper::UdpOrchestrationServerHelper (uint16_t port)
{
  m_factory.SetTypeId (UdpOrchestrationComputeNodeApp::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
UdpOrchestrationServerHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpOrchestrationServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpOrchestrationServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpOrchestrationServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

ApplicationContainer
UdpOrchestrationServerHelper::Install (Ptr<Node> node, std::vector<std::string> function_enable_list) const
{
  Ptr<UdpOrchestrationComputeNodeApp> app = m_factory.Create<UdpOrchestrationComputeNodeApp> ();
  for(std::string t_func_name:function_enable_list){
    app->GetComputeNode()->EnableFunction(t_func_name);
  }
  node->AddApplication (app);
  return ApplicationContainer(app);
}

Ptr<Application>
UdpOrchestrationServerHelper::InstallPriv (Ptr<Node> n) const
{
  Ptr<Application> app = m_factory.Create<UdpOrchestrationComputeNodeApp> ();
  n->AddApplication (app);
  return app;
}

UdpOrchestrationClientHelper::UdpOrchestrationClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (UdpOrchestrationCommunicationApp::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

UdpOrchestrationClientHelper::UdpOrchestrationClientHelper (Address address)
{
  m_factory.SetTypeId (UdpOrchestrationCommunicationApp::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void
UdpOrchestrationClientHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
UdpOrchestrationClientHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<UdpOrchestrationCommunicationApp>()->SetFill (fill);
}

void
UdpOrchestrationClientHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<UdpOrchestrationCommunicationApp>()->SetFill (fill, dataLength);
}

void
UdpOrchestrationClientHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<UdpOrchestrationCommunicationApp>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
UdpOrchestrationClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpOrchestrationClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpOrchestrationClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}



Ptr<Application>
UdpOrchestrationClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpOrchestrationCommunicationApp> ();
  node->AddApplication (app);

  return app;
}

}//namespace inc
} // namespace ndn
} // namespace ns3
