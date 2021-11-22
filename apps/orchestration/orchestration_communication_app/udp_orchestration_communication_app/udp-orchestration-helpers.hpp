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

#ifndef UDP_ORCHESTRATION_HELPER_NODE_INFO_H
#define UDP_ORCHESTRATION_HELPER_NODE_INFO_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/incSIM-module.h"

namespace ns3 {
  namespace ndn{
     namespace inc{
/**
 *
 * \brief A helper class to help installing UdpOrchestrationNodeApp and UdpOrchestrationOrchestratorApp
 */
class UdpOrchestrationServerHelper
{
public:

  UdpOrchestrationServerHelper (uint16_t port);


  void SetAttribute (std::string name, const AttributeValue &value);


  ApplicationContainer Install (Ptr<Node> node) const;


  ApplicationContainer Install (std::string nodeName) const;


  ApplicationContainer Install (NodeContainer c) const;

  ApplicationContainer Install (Ptr<Node> node, std::vector<std::string> function_enable_list) const;



private:

  Ptr<Application> InstallPriv (Ptr<Node> cn) const;

  ObjectFactory m_factory; //!< Object factory.
};

class UdpOrchestrationClientHelper
{
public:

  UdpOrchestrationClientHelper (Address ip, uint16_t port);

  UdpOrchestrationClientHelper (Address addr);


  void SetAttribute (std::string name, const AttributeValue &value);


  void SetFill (Ptr<Application> app, std::string fill);


  void SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength);


  void SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength);


  ApplicationContainer Install (Ptr<Node> node) const;


  ApplicationContainer Install (std::string nodeName) const;


  ApplicationContainer Install (NodeContainer c) const;


private:

  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.


};
  }//namespace inc
  } // namespace ndn
} // namespace ns3

#endif /* UDP_ORCHESTRATION_HELPER_H */
