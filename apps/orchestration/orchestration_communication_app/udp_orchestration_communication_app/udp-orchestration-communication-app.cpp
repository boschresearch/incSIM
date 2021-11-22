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
 * Based on the upd-echo-client implementation of ns3
 * https://www.nsnam.org/doxygen/udp-echo-client_8h_source.html
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 * *****************************************************************************
 */

#include "udp-orchestration-communication-app.hpp"

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/incSIM-module.h"

namespace ns3 {
namespace ndn {
namespace inc {
NS_LOG_COMPONENT_DEFINE ("ndn.inc.UdpOrchestrationCommunicationApp");

NS_OBJECT_ENSURE_REGISTERED (UdpOrchestrationCommunicationApp);

TypeId
UdpOrchestrationCommunicationApp::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::ndn::inc::UdpOrchestrationCommunicationApp")
          .SetParent<Application> ()
          .SetGroupName ("Applications")
          .AddConstructor<UdpOrchestrationCommunicationApp> ()
          .AddAttribute ("MaxPackets", "The maximum number of packets the application will send",
                         UintegerValue (100),
                         MakeUintegerAccessor (&UdpOrchestrationCommunicationApp::m_count),
                         MakeUintegerChecker<uint32_t> ())
          .AddAttribute ("Interval", "The time to wait between packets", TimeValue (Seconds (1.0)),
                         MakeTimeAccessor (&UdpOrchestrationCommunicationApp::m_interval),
                         MakeTimeChecker ())
          .AddAttribute ("RemoteAddress", "The destination Address of the outbound packets",
                         AddressValue (),
                         MakeAddressAccessor (&UdpOrchestrationCommunicationApp::m_peerAddress),
                         MakeAddressChecker ())
          .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                         UintegerValue (0),
                         MakeUintegerAccessor (&UdpOrchestrationCommunicationApp::m_peerPort),
                         MakeUintegerChecker<uint16_t> ())
          .AddAttribute ("PacketSize", "Size of echo data in outbound packets", UintegerValue (100),
                         MakeUintegerAccessor (&UdpOrchestrationCommunicationApp::SetDataSize,
                                               &UdpOrchestrationCommunicationApp::GetDataSize),
                         MakeUintegerChecker<uint32_t> ())
          .AddAttribute("ExecutionTime", "Time at which orchestration command is sent", TimeValue(Seconds(0)),
                            MakeTimeAccessor(&UdpOrchestrationCommunicationApp::m_ExeTime), MakeTimeChecker())
          .AddAttribute("Periodic", "Periodic interests?", BooleanValue(false),
                      MakeBooleanAccessor(&UdpOrchestrationCommunicationApp::m_periodic), MakeBooleanChecker())
          .AddTraceSource ("UdpTx", "A new packet is created and is sent",
                           MakeTraceSourceAccessor (&UdpOrchestrationCommunicationApp::m_txTrace),
                           "ns3::Packet::TxTracedCallback")
          .AddTraceSource ("UdpRx", "A packet has been received",
                           MakeTraceSourceAccessor (&UdpOrchestrationCommunicationApp::m_rxTrace),
                           "ns3::Packet::RxTracedCallback")
          .AddTraceSource ("UdpDeciTx", "A packet has been received",
                           MakeTraceSourceAccessor (&UdpOrchestrationCommunicationApp::m_deciTxTrace),
                           "ns3::Packet::DeciTxTracedCallback")
          .AddTraceSource (
              "TxWithAddresses", "A new packet is created and is sent",
              MakeTraceSourceAccessor (&UdpOrchestrationCommunicationApp::m_txTraceWithAddresses),
              "ns3::Packet::TwoAddressTracedCallback")
          .AddTraceSource (
              "RxWithAddresses", "A packet has been received",
              MakeTraceSourceAccessor (&UdpOrchestrationCommunicationApp::m_rxTraceWithAddresses),
              "ns3::Packet::TwoAddressTracedCallback");
  return tid;
}

UdpOrchestrationCommunicationApp::UdpOrchestrationCommunicationApp ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
}

UdpOrchestrationCommunicationApp::~UdpOrchestrationCommunicationApp ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete[] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void
UdpOrchestrationCommunicationApp::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
UdpOrchestrationCommunicationApp::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
UdpOrchestrationCommunicationApp::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
UdpOrchestrationCommunicationApp::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (
              InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (
              Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&UdpOrchestrationCommunicationApp::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  m_first_time = true;
  if(m_periodic==true)
  ScheduleTransmit (Seconds(m_interval));
  else
  ScheduleOnce();
}

void
UdpOrchestrationCommunicationApp::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket>> ());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}

void
UdpOrchestrationCommunicationApp::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so
  // neither will we.
  //
  delete[] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t
UdpOrchestrationCommunicationApp::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void
UdpOrchestrationCommunicationApp::SetFill (std::string fill)
{
  NS_LOG_FUNCTION (this << fill);

  uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete[] m_data;
      m_data = new uint8_t[dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_data, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void
UdpOrchestrationCommunicationApp::SetFill (uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
  if (dataSize != m_dataSize)
    {
      delete[] m_data;
      m_data = new uint8_t[dataSize];
      m_dataSize = dataSize;
    }

  memset (m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void
UdpOrchestrationCommunicationApp::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete[] m_data;
      m_data = new uint8_t[dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void
UdpOrchestrationCommunicationApp::ScheduleTransmit (Time dt)
{
  if(m_first_time==true)
  {
    m_sendEvent = Simulator::Schedule(m_ExeTime, &UdpOrchestrationCommunicationApp::Send, this);
    m_first_time = false;
  }
  else
  {
    NS_LOG_FUNCTION (this << dt);
    m_sendEvent = Simulator::Schedule (dt, &UdpOrchestrationCommunicationApp::Send, this);
  }
}

void
UdpOrchestrationCommunicationApp::ScheduleOnce()
{
  m_sendEvent = Simulator::Schedule(m_ExeTime,&UdpOrchestrationCommunicationApp::SendOnce, this);
}

void
UdpOrchestrationCommunicationApp::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;
  if (m_dataSize)
    {
      //
      // If m_dataSize is non-zero, we have a data buffer of the same size that we
      // are expected to copy and send.  This state of affairs is created if one of
      // the Fill functions is called.  In this case, m_size must have been set
      // to agree with m_dataSize
      //
      NS_ASSERT_MSG (
          m_dataSize == m_size,
          "UdpOrchestrationCommunicationApp::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG (m_data, "UdpOrchestrationCommunicationApp::Send(): m_dataSize but no m_data");
      p = Create<Packet> (m_data, m_dataSize);
    }
  else
    {
      //
      // If m_dataSize is zero, the client has indicated that it doesn't care
      // about the data itself either by specifying the data size by setting
      // the corresponding attribute or by not calling a SetFill function.  In
      // this case, we don't worry about it either.  But we do allow m_size
      // to have a value different from the (zero) m_dataSize.
      //
      p = Create<Packet> (m_size);
    }
  Address localAddress;
  m_socket->GetSockName (localAddress);
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      m_txTraceWithAddresses (
          p, localAddress,
          InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      m_txTraceWithAddresses (
          p, localAddress,
          Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  NS_LOG_INFO ("[Orchestrator] packet is " << p);
  m_socket->Send (p);
  ++m_sent;

  std::cout << std::endl;

  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to "
                   << Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to "
                   << Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to "
                   << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port "
                   << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to "
                   << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << " port "
                   << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }

  if (m_sent < m_count)
    {
      ScheduleTransmit (m_interval);
    }
}

void
UdpOrchestrationCommunicationApp::SendOnceWithFill (std::string fill)
{
  NS_LOG_FUNCTION (this);
  //NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> p = new Packet (fill);

  Address localAddress;
  m_socket->GetSockName (localAddress);
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_deciTxTrace (p);

  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      m_txTraceWithAddresses (
          p, localAddress,
          InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      m_txTraceWithAddresses (
          p, localAddress,
          Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  NS_LOG_INFO ("[Orchestrator] packet is " << p);
  p->RemoveAllPacketTags ();
  p->RemoveAllByteTags ();
  m_socket->Send (p);
  ++m_sent;

  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time " << Simulator::Now ().GetSeconds () << "s client sent "
                                             << p->GetSize () << " bytes to "
                                             << Ipv4Address::ConvertFrom (m_peerAddress) << " port "
                                             << m_peerPort);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time " << Simulator::Now ().GetSeconds () << "s client sent "
                                             << p->GetSize () << " bytes to "
                                             << Ipv6Address::ConvertFrom (m_peerAddress) << " port "
                                             << m_peerPort);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << p->GetSize ()
                   << " bytes to " << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 ()
                   << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << p->GetSize ()
                   << " bytes to " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }
}

void
UdpOrchestrationCommunicationApp::SendOnce()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;
  if (m_dataSize)
    {
      NS_ASSERT_MSG (
          m_dataSize == m_size,
          "UdpOrchestrationCommunicationApp::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG (m_data, "UdpOrchestrationCommunicationApp::Send(): m_dataSize but no m_data");
      p = Create<Packet> (m_data, m_dataSize+1);
    }
  else
    {
      p = Create<Packet> (m_size+1);
    }
  Address localAddress;
  m_socket->GetSockName (localAddress);
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      m_txTraceWithAddresses (
          p, localAddress,
          InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      m_txTraceWithAddresses (
          p, localAddress,
          Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  NS_LOG_INFO ("[Orchestrator] packet is " << p);
  m_socket->Send (p);
  ++m_sent;

  std::cout << std::endl;

  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to "
                   << Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to "
                   << Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to "
                   << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port "
                   << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("[Orchestrator] At time "
                   << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to "
                   << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << " port "
                   << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }
}


void
UdpOrchestrationCommunicationApp::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  std::string packet_data;
  std::string actionToDo;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("[Orchestrator] At time "
                       << Simulator::Now ().GetSeconds () << "s client received "
                       << packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port "
                       << InetSocketAddress::ConvertFrom (from).GetPort ());
          NS_LOG_INFO ("[Orchestrator] packet content:");
          uint8_t *buffer = new uint8_t[packet->GetSize ()];
          packet->CopyData (buffer, packet->GetSize ());
          packet_data = std::string ((char *) buffer);
          NS_LOG_INFO (packet_data << std::endl);
          NS_LOG_INFO (std::endl);
          m_rxTrace(packet);
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("[Orchestrator] At time "
                       << Simulator::Now ().GetSeconds () << "s client received "
                       << packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port "
                       << Inet6SocketAddress::ConvertFrom (from).GetPort ());
          NS_LOG_INFO ("[Orchestrator] packet content: ");
          uint8_t *buffer = new uint8_t[packet->GetSize ()];
          packet->CopyData (buffer, packet->GetSize ());
          packet_data = std::string ((char *) buffer);
          NS_LOG_INFO (packet_data << std::endl);
          NS_LOG_INFO (std::endl);
          m_rxTrace(packet);
        }

      if (packet_data.find ("/Orchestrator") == 0)
        {
          if (packet_data.find ("/BootstrapInfo/") != packet_data.npos)
            {
              actionToDo = "/BootstrapInfo";
              int index = packet_data.find (actionToDo);
              int last_position = index + actionToDo.size() + 1;
              packet_data.erase (0, last_position);
            }
          else if (packet_data.find ("/NodeStatusFetch") != packet_data.npos)
            {
              actionToDo = "/NodeStatusFetch";
              int index = packet_data.find (actionToDo);
              int last_position = index + actionToDo.size() + 1;
              packet_data.erase (0, last_position);

            }
          else
            {
              actionToDo = "/Others";
            }
        }
      else
        {
          actionToDo = "/Others";
        }

      m_message_handler.HandleMessage (actionToDo, packet_data);
    }
}
} //namespace inc
} // namespace ndn
} // namespace ns3
