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

#include "udp-orchestration-node-app.hpp"

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include <sstream>
#include <iostream>
#include "ns3/pointer.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/incSIM-module.h"
#include <boost/algorithm/string.hpp>

NS_LOG_COMPONENT_DEFINE ("ndn.inc.UdpOrchestrationComputeNodeApp");

namespace ns3 {
namespace ndn {
namespace inc {
NS_OBJECT_ENSURE_REGISTERED (UdpOrchestrationComputeNodeApp);

TypeId
UdpOrchestrationComputeNodeApp::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::ndn::UdpOrchestrationComputeNodeApp")
          .SetParent<Application> ()
          .SetGroupName ("Applications")
          .AddConstructor<UdpOrchestrationComputeNodeApp> ()
          .AddAttribute ("Port", "Port on which we listen for incoming packets.", UintegerValue (9),
                         MakeUintegerAccessor (&UdpOrchestrationComputeNodeApp::m_port),
                         MakeUintegerChecker<uint16_t> ())
          .AddAttribute ("ComputeNodePointer", "the pointer of compute node to access node info",
                         PointerValue (),
                         MakePointerAccessor (&UdpOrchestrationComputeNodeApp::m_compute_node),
                         MakePointerChecker<IncOrchestrationComputeNode> ())
          .AddTraceSource ("Rx", "A packet has been received",
                           MakeTraceSourceAccessor (&UdpOrchestrationComputeNodeApp::m_rxTrace),
                           "ns3::Packet::TracedCallback")
          .AddTraceSource (
              "RxWithAddresses", "A packet has been received",
              MakeTraceSourceAccessor (&UdpOrchestrationComputeNodeApp::m_rxTraceWithAddresses),
              "ns3::Packet::TwoAddressTracedCallback");
  return tid;
}

UdpOrchestrationComputeNodeApp::UdpOrchestrationComputeNodeApp ()
{

  NS_LOG_FUNCTION (this);
}
void
UdpOrchestrationComputeNodeApp::SetComputeNode (Ptr<IncOrchestrationComputeNode> cn)
{
  m_compute_node = cn;
}

Ptr<IncOrchestrationComputeNode>
UdpOrchestrationComputeNodeApp::GetComputeNode (void)
{
  return m_compute_node;
}

UdpOrchestrationComputeNodeApp::~UdpOrchestrationComputeNodeApp ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
}

void
UdpOrchestrationComputeNodeApp::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
UdpOrchestrationComputeNodeApp::StartApplication (void)
{

  NS_LOG_FUNCTION (this);
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = ns3::Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = ns3::Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      if (m_socket6->Bind (local6) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }
  m_socket->SetRecvCallback (MakeCallback (&UdpOrchestrationComputeNodeApp::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&UdpOrchestrationComputeNodeApp::HandleRead, this));
}

void
UdpOrchestrationComputeNodeApp::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket>> ());
    }
  if (m_socket6 != 0)
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket>> ());
    }
}

Ptr<Packet>
UdpOrchestrationComputeNodeApp::OrchestrationRequestResolution (std::string packet_data)
{
  Ptr<Packet> response;
  if (packet_data == "/BootstrapInfo" || (packet_data == "/NodeStatusFetch"))
    {
      response = StatusFetchHandler (packet_data);
      return response;
    }
  else if (packet_data == "/FunctionStatus")
    {
      response = FunctionStatusRequestHandler (packet_data);
      return response;
    }
  else if (packet_data.find("/FunctionSwitch")!= packet_data.npos)
    {
      response = FunctionSwitchHandler (packet_data);
      return response;
    }
  else
    {
      response = UnknownPacketHandler (packet_data);
      return response;
    }
}

//-------------------------------------------------------------------------//
//--------------------------Status Fetch handler---------------------------//
//-------------------------------------------------------------------------//
Ptr<Packet>
UdpOrchestrationComputeNodeApp::StatusFetchHandler (std::string packet_data)
{
  Ptr<Packet> response;
  NS_LOG_INFO ("[Computation Node] Receive bootstrapp node status request from orchestrator"
               << std::endl);
  std::stringstream ss;
  ss << "/Orchestrator/" << m_compute_node->GetName ()<<packet_data<<"/"
     << "name=/" << m_compute_node->GetName () << ";"
     << "processor_core=" << m_compute_node->GetProcessorCore () << ";"
     << "processor_type=" << m_compute_node->GetProcessorClockSpeed () << ";"
     << "RAM=" << m_compute_node->GetRam () << ";"
     << "ROM=" << m_compute_node->GetRom () << ";"
     << "links=" << m_compute_node->GetLinks () << ";"
     << "runtimes=" << m_compute_node->GetSupportedRuntimes () << ";"
     << "node_busy_counter="<<m_compute_node->GetNodeBusyCounter()<<";"
     << "data="<<m_compute_node->GetProvidedData()<<";"
     << "functions=";
  for (auto &it : m_compute_node->GetFunctionMap ())
    {
      ss << it.first << ":"
      << it.second->GetEnableStatus () << ":"
      << it.second->GetCounter ()<< ":"
      << it.second->GetMissExecCounter ()<< ":"
      << it.second->GetCpu()<< ":"
      << it.second->GetRam()<< ":"
      <<it.second->GetRom()<<":"
      <<it.second->GetFuncSize()<<":"
      <<it.second->GetInputList();
    }
  std::string payload = ss.str ();
  m_compute_node->ResetNodeBusyCounter();
  NS_LOG_INFO ("[Computation Node] Sending packet content:");
  NS_LOG_INFO (payload << std::endl);
  response = Create<Packet> ((uint8_t *) payload.c_str (), payload.size () + 1);
  return response;
}

//-------------------------------------------------------------------------//
//---------------------------Function Switch Handler-----------------------//
//-------------------------------------------------------------------------//

Ptr<Packet>
UdpOrchestrationComputeNodeApp::FunctionSwitchHandler (std::string packet_data)
{
  Ptr<Packet> response;
  std::string payload;
  NS_LOG_INFO("Packet data is "<<packet_data<<std::endl);
  bool flag=false;
  NS_LOG_INFO ("[Computation Node]Receive enable/disable function request from orchestrator"
               << std::endl);
  //eg name: Orchestrator/Node1/FunctionSwitch/Disable/Func1/Func2/Func3
  std::vector<std::string> tokens;
  boost::split (tokens, packet_data, boost::is_any_of ("/"));
  for(auto i =tokens.begin(); i!=tokens.end();++i)
  std::cout<<*i<<endl;
  auto itr = tokens.begin();
  for (std::advance(itr,5); itr != tokens.end()-1; ++itr) //starting from the fourth token
    {
      if (tokens[4].compare("Enable")==0)
        {
          std::string func_name = (*itr).insert(0,1,'/');
          flag = GetComputeNode ()->EnableFunction(func_name);
          if(GetComputeNode()->CheckExcludeList(func_name))
            GetComputeNode()->RemoveFromExcludeList(func_name);
          if(flag)
            {
              this->m_onFuncEnableTrace(func_name);
            }
        }
      else if(tokens[4].compare("Disable")==0)
        {
          std::string func_name = (*itr).insert(0,1,'/');
          flag = GetComputeNode ()->DisableFunction(func_name);
          GetComputeNode()->AddToExcludeList(func_name);
          if(flag)
          {
            this->m_onFuncDisableTrace(func_name);
          }
        }
    }

  if (flag)
    {
      payload = "operation success";
    }
  else
    {
      payload = "operation failed";
    }
  NS_LOG_INFO ("[Computation Node] Sending packet content:");
  NS_LOG_INFO (payload << std::endl);
  response = Create<Packet> ((uint8_t *) payload.c_str (), payload.size () + 1);
  return response;
}

//-------------------------------------------------------------------------//
//-------------------Function Status Request Handler-----------------------//
//-------------------------------------------------------------------------//
Ptr<Packet>
UdpOrchestrationComputeNodeApp::FunctionStatusRequestHandler (std::string packet_data)
{
  Ptr<Packet> response;
  NS_LOG_INFO ("[Computation Node] Receive function counter request from orchestrator"
               << std::endl);
  std::stringstream ss;
  ss << "/Orchestrator/" << m_compute_node->GetName () << "/FunctionStatus/";
  for (auto &it : m_compute_node->GetFunctionMap ())
    {
      ss << "function_name=" << it.first << ";"
         << "enable_status=" << it.second->GetEnableStatus () << ";"
         << "execution_counter=" << it.second->GetCounter () << ";"
         << std::endl;
    }
  std::string payload = ss.str ();
  NS_LOG_INFO ("[Computation Node] Sending packet content:");
  NS_LOG_INFO (payload << std::endl);
  response = Create<Packet> ((uint8_t *) payload.c_str (), payload.size () + 1);
  return response;
}

Ptr<Packet>
UdpOrchestrationComputeNodeApp::UnknownPacketHandler (std::string packet_data)
{
  Ptr<Packet> response;
  std::cout<<"Received Packet is : "<<packet_data<<std::endl;
  NS_LOG_INFO ("[Computation Node] Receive unknown request from orchestrator" << std::endl);
  std::stringstream ss;
  ss << "Unknown orchestration instruction";
  std::string payload = ss.str ();
  NS_LOG_INFO ("[Computation Node] Sending packet content:");
  NS_LOG_INFO (payload << std::endl);
  response = new Packet (payload);
  return response;
}

void
UdpOrchestrationComputeNodeApp::HandleRead (Ptr<ns3::Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet, response;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      socket->GetSockName (localAddress);
      m_rxTrace (packet);
      m_rxTraceWithAddresses (packet, from, localAddress);
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("[Computation Node] At time "
                       << Simulator::Now ().GetSeconds () << "s server received "
                       << packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port "
                       << InetSocketAddress::ConvertFrom (from).GetPort () << std::endl);
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("[Computation Node] At time "
                       << Simulator::Now ().GetSeconds () << "s server received "
                       << packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port "
                       << Inet6SocketAddress::ConvertFrom (from).GetPort () << std::endl);
        }

      //------------------------------------------------------------//
      //-------------------------Orchestration----------------------//
      //------------------------------------------------------------//

      Ptr<Node> Current_Node = this->GetNode ();
      //Ptr<ns3::ndn::INC_Compute> inc_compute = Current_Node->GetApplication(0)->GetObject<ns3::ndn::INC_Compute>();

      uint8_t *buffer = new uint8_t[packet->GetSize ()];
      packet->CopyData (buffer, packet->GetSize ());
      std::string packet_data = std::string ((char *) buffer);
      response = OrchestrationRequestResolution (packet_data);

      //------------------------------------------------------------//
      //----------------------End of Orchestration------------------//
      //------------------------------------------------------------//

      response->RemoveAllPacketTags ();
      response->RemoveAllByteTags ();

      NS_LOG_LOGIC ("[Computation Node] Sending node status info");
      socket->SendTo (response, 0, from);

      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("[Computation Node] At time "
                       << Simulator::Now ().GetSeconds () << "s server sent "
                       << response->GetSize () << " bytes to "
                       << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port "
                       << InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("[Computation Node] At time "
                       << Simulator::Now ().GetSeconds () << "s server sent "
                       << response->GetSize () << " bytes to "
                       << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port "
                       << Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
    }
}
} //namespace inc
} //namespace ndn
} // Namespace ns3
