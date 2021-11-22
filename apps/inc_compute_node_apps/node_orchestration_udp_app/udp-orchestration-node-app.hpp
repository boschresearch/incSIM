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
 * Based on the udp-echo-server application implementation of ns3
 * https://www.nsnam.org/doxygen/udp-echo-server_8cc_source.html
 * Originated from University of Washington
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 *          Liming Liu <fixed-term.liming.liu@de.bosch.com>
 * *****************************************************************************
 */


#ifndef TEST_SERVER_H
#define TEST_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include <unordered_map>
//#include "ns3/ndnSIM-module.h"
#include "ns3/incSIM-module.h"


namespace ns3 {
class Socket;
class Packet;
namespace ndn {
namespace inc {
/**
 *
 * \brief Application installed on compute node in order to be orchestrated.
 *  Will listen to orchestrator over udp socket and do operations on request from orchestrator
 *
 *
 */
class UdpOrchestrationComputeNodeApp : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  UdpOrchestrationComputeNodeApp ();
  virtual ~UdpOrchestrationComputeNodeApp ();


  void SetComputeNode(Ptr<IncOrchestrationComputeNode> cn);
  Ptr<IncOrchestrationComputeNode> GetComputeNode(void);

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<ns3::Socket> socket);

  virtual Ptr<Packet>
  OrchestrationRequestResolution(std::string packet_data);


  virtual Ptr<Packet>
  StatusFetchHandler(std::string packet_data);


  Ptr<Packet>
  FunctionSwitchHandler(std::string packet_data);


  virtual Ptr<Packet>
  FunctionStatusRequestHandler(std::string packet_data);


  virtual Ptr<Packet>
  UnknownPacketHandler(std::string packet_data);

  typedef void ( *EnableFunctionCallback)(std::string);
  typedef void ( *DisableFunctionCallback)(std::string);

  uint16_t m_port; //!< Port on which we listen for incoming packets.
  Ptr<ns3::Socket> m_socket; //!< IPv4 Socket
  Ptr<ns3::Socket> m_socket6; //!< IPv6 Socket
  ns3::Address m_local; //!< local multicast address



  /// Callbacks for tracing the packet Rx events
  TracedCallback<Ptr<const ns3::Packet> > m_rxTrace;

  /// Callbacks for tracing the packet Rx events, includes source and destination addresses
  TracedCallback<Ptr<const ns3::Packet>, const ns3::Address &, const ns3::Address &> m_rxTraceWithAddresses;

  //Pointer to the compute node, which contains the ns3::Node instance the app is installed on
  Ptr<IncOrchestrationComputeNode> m_compute_node;
  TracedCallback<std::string> m_onFuncEnableTrace;
  TracedCallback<std::string> m_onFuncDisableTrace;


};
}//namespace inc
}// namespace ndn
} // namespace ns3

#endif /* INC_UDP_ORCHESTRATION_SERVER_APP_H */
