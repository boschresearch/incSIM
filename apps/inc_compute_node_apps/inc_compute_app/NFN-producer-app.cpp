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
 * Based on the ndn-producer implementation of ns3
 * https://ndnsim.net/current/doxygen/classns3_1_1ndn_1_1Producer.html
 * Originated from ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 *  Date: November 8th, 2021
 *  Contributors:
 *      Robert Bosch GmbH - initial API and functionality
 *          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
 *          Liming Liu <fixed-term.liming.liu@de.bosch.com>
 * *****************************************************************************
 */
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/packet.h"
#include "./NFN-producer-app.hpp"
#include <pthread.h>

#include <memory>

/*
NfnProducerApp is a modification of ndn producer application that can respond to ndn interest packets
requesting for computations along with responding to non-compute interest.
On receiving such an interest, it can initiate executing the corresponding function.
Once the computation is complete, it returns the result in a data packet corresponding to the interest.
If the function to be executed requires any input arguments,
this application can fetch the arguments by sending interest packets for the same and start executing once all the arguments are available.
*/
NS_LOG_COMPONENT_DEFINE ("NfnProducerApp");

namespace ns3 {
namespace ndn {
namespace inc {

NS_OBJECT_ENSURE_REGISTERED (NfnProducerApp);

TypeId
NfnProducerApp::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::ndn::inc::NfnProducerApp")
          .SetGroupName ("Inc")
          .SetParent<App> ()
          .AddConstructor<NfnProducerApp> ()
          .AddAttribute ("Prefix", "Prefix, for which producer has the data", StringValue ("/"),
                         MakeNameAccessor (&NfnProducerApp::m_prefix), MakeNameChecker ())
          .AddAttribute ("ComputeNodePointer", "the pointer of compute node to access functions",
                         PointerValue (), MakePointerAccessor (&NfnProducerApp::m_compute_node),
                         MakePointerChecker<IncOrchestrationComputeNode> ())
          .AddAttribute (
              "Postfix",
              "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
              StringValue ("/"), MakeNameAccessor (&NfnProducerApp::m_postfix), MakeNameChecker ())
          .AddAttribute ("PayloadSize", "Virtual payload size for Content packets",
                         UintegerValue (1024),
                         MakeUintegerAccessor (&NfnProducerApp::m_virtualPayloadSize),
                         MakeUintegerChecker<uint32_t> ())
          .AddAttribute ("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                         TimeValue (Seconds (0.5)), MakeTimeAccessor (&NfnProducerApp::m_freshness),
                         MakeTimeChecker ()) //TODO : Generated values following a distribution according to request frequency
          .AddAttribute (
              "Signature",
              "Fake signature, 0 valid signature (default), other values application-specific",
              UintegerValue (0), MakeUintegerAccessor (&NfnProducerApp::m_signature),
              MakeUintegerChecker<uint32_t> ())
          .AddAttribute ("KeyLocator",
                         "Name to be used for key locator.  If root, then key locator is not used",
                         NameValue (), MakeNameAccessor (&NfnProducerApp::m_keyLocator),
                         MakeNameChecker ())
          .AddTraceSource (
              "SendComputeInterests",
              "Trace called every time there is an Interest packet is send to the network",
              MakeTraceSourceAccessor (&NfnProducerApp::m_onOutgoingInterestTrace),
              "ns3::ndn::inc::NfnProducerApp::SendInterestTracedCallback")
          .AddTraceSource (
              "RcvComputeInterests",
              "Trace called every time there is an Interest packet received from the network",
              MakeTraceSourceAccessor (&NfnProducerApp::m_onIncomingInterestTrace),
              "ns3::ndn::inc::NfnProducerApp::RcvInterestTracedCallback")
          .AddTraceSource ("ExecutingFunctionTrace",
                           "Trace called every time a function is executed",
                           MakeTraceSourceAccessor (&NfnProducerApp::m_onFuncExecutionTrace),
                           "ns3::ndn::inc::NfnProducerApp::FuncExecutingCallback")
          .AddTraceSource ("IncomingComputeDatas",
                           "Trace called every time there is an incoming Data packet",
                           MakeTraceSourceAccessor (&NfnProducerApp::m_onIncomingDataTrace),
                           "ns3::ndn::inc::NfnProducerApp::IncomingDatasCallback")
          .AddTraceSource ("OutgoingComputeDatas",
                           "Trace called every time there is an outgoing data packet",
                           MakeTraceSourceAccessor (&NfnProducerApp::m_onOutgoingDataTrace),
                           "ns3::ndn::inc::NfnProducerApp::OutgoingDatasCallback")
          .AddTraceSource ("NodeBusyFailure",
                           "Trace called when function could not be executed due to resource "
                           "unavailability at the compute node",
                           MakeTraceSourceAccessor (&NfnProducerApp::m_onNodeBusyTrace),
                           "ns3::ndn::inc::NfnProducerApp::NodeBusyCallback")
          .AddTraceSource ("FuncDisabledFailure",
                           "Trace called when function could not be executed due to function "
                           "unavailability at the compute node",
                           MakeTraceSourceAccessor (&NfnProducerApp::m_onFuncDisabledTrace),
                           "ns3::ndn::inc::NfnProducerApp::FuncDisabledCallback")
          .AddTraceSource ("NFN_IntForwarded",
                           "Trace called when NFN RE performs interest forwarding",
                           MakeTraceSourceAccessor (&NfnProducerApp::m_onForwardInterestTrace),
                           "ns3::ndn::inc::NfnProducerApp::nfnForwardInterestCallback")
          .AddTraceSource ("NFN_IntQueued",
                            "Trace called when interest is pushes into queue",
                            MakeTraceSourceAccessor( &NfnProducerApp::m_onQueueInterestTrace),
                            "ns3::ndn::inc::NfnProducerApp::nfnQueueInterestCallback")
          .AddTraceSource ("NFN_FuncEnabled", "Trace called when NFN performs a code drag",
                           MakeTraceSourceAccessor (&NfnProducerApp::m_nfnFuncEnabledTrace),
                           "ns3::ndn::inc::NfnProducerApp::nfnFuncEnabledCallback");
  return tid;
}

NfnProducerApp::NfnProducerApp ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

// inherited from Application base class.
void
NfnProducerApp::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (m_active != true);
  m_active = true;

  NS_ASSERT_MSG (GetNode ()->GetObject<L3Protocol> () != 0,
                 "Ndn stack should be installed on the node " << GetNode ());

  // step 1. Create a face
  auto appLink = make_unique<AppLinkService> (this);
  auto transport = make_unique<NullTransport> ("nfnProducerAppFace://", "nfnProducerAppFace://",
                                               ::ndn::nfd::FACE_SCOPE_LOCAL);
  // @TODO Consider making AppTransport instead
  m_face = std::make_shared<Face> (std::move (appLink), std::move (transport));
  m_appLink = static_cast<AppLinkService *> (m_face->getLinkService ());
  m_face->setMetric (1);

  // step 2. Add face to the Ndn stack
  GetNode ()->GetObject<L3Protocol> ()->addFace (m_face);

  FibHelper::AddRoute (GetNode (), m_prefix, m_face, 0);

  //pass producer app face to compute node,
  //the producer app will be used by compute node to register nexthop face in NFD FIBwhen new function is enabled/disabled
  m_compute_node->SetProducerAppFace (m_face);
  m_queue_size = m_compute_node->GetQueueSize();
}

void
NfnProducerApp::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  App::StopApplication ();
}

NfnProducerApp::~NfnProducerApp ()
{
}

//resource management funcitons
void
NfnProducerApp::ReleaseResource (Ptr<INC_Computation> func)
{
  m_compute_node->SetRam (m_compute_node->GetRam () + func->GetRam ());
  m_compute_node->SetProcessorCore (m_compute_node->GetProcessorCore () + func->GetCpu ());
  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                 << "] At time: " << Simulator::Now ().GetSeconds ()
                                 << ". Release resource booked by: " << func->getName ()
                                 << " on node " << m_compute_node->GetName ()
                                 << " finished successfully!" << std::endl);
}

void
NfnProducerApp::BookResource (Ptr<INC_Computation> func)
{
  std::vector<std::string> trace_value;
  trace_value.push_back (func->getName ().toUri ());
  trace_value.push_back (to_string (func->GetCpu ()));
  trace_value.push_back (to_string (func->GetRam ()));
  trace_value.push_back (to_string (func->GetRom ()));
  trace_value.push_back (to_string (func->GetExecTime ()));
  m_onFuncExecutionTrace (trace_value);
  m_compute_node->SetRam (m_compute_node->GetRam () - func->GetRam ());
  m_compute_node->SetProcessorCore (m_compute_node->GetProcessorCore () - func->GetCpu ());
  //std::cout << std::endl;
  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                 << "] At time: " << Simulator::Now ().GetSeconds ()
                                 << ". Book resource by: " << func->getName () << " on node "
                                 << m_compute_node->GetName () << " finished successfully!"
                                 << std::endl);
}

bool
NfnProducerApp::CheckResource (Ptr<INC_Computation> func)
{
  if ((m_compute_node->GetRam () < func->GetRam ()) ||
      (m_compute_node->GetProcessorCore () < func->GetCpu ()))
    {
      //std::cout << std::endl;
      NS_LOG_DEBUG ("[NFN producer: "
                   << m_compute_node->GetName () << "] Check resource for function "
                   << func->getName () << "on node " << m_compute_node->GetName ()
                   << " failed, compute node resource not sufficient" << std::endl);
      //m_compute_node->AddNodeBusyCounter ();
      //this->m_onNodeBusyTrace (func->getName ().toUri ());
      return false;
    }
  else
    {
      NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                     << "] Check resource for function " << func->getName ()
                                     << "on node " << m_compute_node->GetName () << " passed"
                                     << std::endl);
      return true;
    }
}

void
NfnProducerApp::ScheduledExecutionEnd (shared_ptr<Data> data, Ptr<INC_Computation> func)
{
  ReleaseResource (func);
  this->m_onOutgoingDataTrace (data);
  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName () << "] Execute Function: "
                                 << func->getName () << " success , will send response");
  NS_LOG_DEBUG("Before Sending data "<<*data);
  m_transmittedDatas (data, this, m_face); //Send the result of the execution as a data packet.
  m_appLink->onReceiveData (*data);
  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                 << "] respond packet to consumer at: "
                                 << Simulator::Now ().GetSeconds () << std::endl);

  if (!(m_compute_node->IsQueueEmpty()))
    {
      NS_LOG_DEBUG ("Checking pending interests in queue!" << std::endl);
      InterestComponentStruct t_ics = m_waiting_list.front ();
      if (CheckResource (t_ics.m_func))
        {
          NS_LOG_DEBUG ("popped interest from queue:" << t_ics.m_interest.toUri () << std::endl);
          m_compute_node->DecrementQueueFill();
          m_waiting_list.pop ();
          DoExecution (t_ics);
        }
    }
}

void
NfnProducerApp::DoExecution (InterestComponentStruct ics)
{
  BookResource (ics.m_func);
  //scheduled packet sending time
  double scheduled_sending_time = Simulator::Now ().GetSeconds () + ics.m_func->GetExecTime ();
  ics.m_func->Execute ();
  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                 << "] Start executing function for interest "
                                 << ics.m_interest.toUri () << std::endl);
  auto data = PrepareDataPacket (ics.m_interest, 50, ns3::Time(Seconds(3.0)));
  //schedule the sending event
  Simulator::Schedule (Seconds (scheduled_sending_time - Simulator::Now ().GetSeconds ()),
                       &NfnProducerApp::ScheduledExecutionEnd, this, data, ics.m_func);
  DeletePendingContentTableEntry (ics);
}

void
NfnProducerApp::DeletePendingContentTableEntry (InterestComponentStruct ics)
{

  auto it = std::find (m_pending_content_table.begin (), m_pending_content_table.end (), ics);
  if (it != m_pending_content_table.end ())
    {
      //The entry was stored in pending content table, erase it
      m_pending_content_table.erase (it);
      NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                     << "] Delete pending content table entry "
                                     << ics.m_interest.toUri () << std::endl);
    }
}

shared_ptr<Data>
NfnProducerApp::PrepareDataPacket (Name name, uint32_t payload_size, ns3::Time freshness)
{
  auto data = make_shared<Data> ();
  data->setName (name);
  data->setFreshnessPeriod (::ndn::time::milliseconds (freshness.GetMilliSeconds ()));
  data->setContent (make_shared<::ndn::Buffer> (payload_size));
  Signature signature;
  SignatureInfo signatureInfo (static_cast<::ndn::tlv::SignatureTypeValue> (255));
  if (m_keyLocator.size () > 0)
    {
      signatureInfo.setKeyLocator (m_keyLocator);
    }
  signature.setInfo (signatureInfo);
  signature.setValue (::ndn::makeNonNegativeIntegerBlock (::ndn::tlv::SignatureValue, m_signature));
  data->setSignature (signature);
  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName () << "] node(" << GetNode ()->GetId ()
                                 << ") responding with Data: " << data->getName () << std::endl);
  // to create real wire encoding
  data->wireEncode ();
  return data;
}

//-----------------------------------------------------------------------------//
//-----------------Receiving the data arguments in Data packets----------------//
//-----------------------------------------------------------------------------//

void
NfnProducerApp::OnData (shared_ptr<const Data> data)
{
  if (!m_active)
    return;
  App::OnData (data); // tracing inside
  NS_LOG_INFO (this << data);
  this->m_onIncomingDataTrace (data);
  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName () << "] Received content object: "
                                 << boost::cref (*data) << std::endl);

  //delete from sending interest list
  m_sending_interest_list.erase (data->getName ());

  //iterate through m_pending_content_table, if any entry(pending interest) needs the data/function in Data packet received, set the flag to true
  //Note: iterate from size-1 to 0 because some entry might be erased in the loop, and iterate backward to avoid position mis-matching
  for (int i = m_pending_content_table.size () - 1; i >= 0; i--)
    {

      //InterestComponentStruct t_ics=m_pending_content_table[i];
      bool executable = true;

      //Current entry needs Data content
      if (m_pending_content_table[i].m_pending_args.find (data->getName ()) !=
          m_pending_content_table[i].m_pending_args.end ())
        {
          NS_LOG_DEBUG ("[NFN producer: "
                       << m_compute_node->GetName ()
                       << "] Content object matches a pending content entry for interest:"
                       << m_pending_content_table[i].m_interest.toUri ());

          if (data->getName ().toUri ().find ("/Function") != string::npos)
            { //data is the response of code drag
              //enable function locally
              NS_LOG_DEBUG ("[NFN producer: "
                           << m_compute_node->GetName ()
                           << "] Received code drag response, will enable the local function"
                           << std::endl);
              int pos_func_name =
                  1; //  Assume format of Function name is: /Function/(function name)
              bool func_enable_flag = m_compute_node->EnableFunction (
                  data->getName ().getSubName (pos_func_name, 1).toUri ());
              if (!func_enable_flag)
                {
                  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                                 << "] Enable local function failed, abort the "
                                                    "operation and clean up producer status"
                                                 << std::endl);
                  DeletePendingContentTableEntry (m_pending_content_table[i]);
                  continue;
                }
              else
                {
                  this->m_nfnFuncEnabledTrace (
                      data->getName ().getSubName (pos_func_name, 1).toUri ());
                }
            }
          if (data->getName ().toUri ().find ("/Data") != string::npos)
            {
              //enable data locally
              NS_LOG_DEBUG ("[NFN producer: "
                           << m_compute_node->GetName ()
                           << "] Received data drag response, will enable the local data"
                           << std::endl);
              std::string provided_data = data->getName ().getSubName (1, 1).toUri ();
              uint32_t data_size = data->wireEncode ().size ();
              provided_data.append (":" + std::to_string (data_size));
              NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                             << "] Provided data added to compute node "
                                             << provided_data);
              //m_compute_node->AddProvidedData (provided_data);
            }

          auto t_pair = m_pending_content_table[i].m_pending_args.find (data->getName ());
          if (t_pair != m_pending_content_table[i].m_pending_args.end () && t_pair->second == false)
            {
              //update pending flag of the content
              t_pair->second = true;

              for (auto it = m_pending_content_table[i].m_pending_args.begin ();
                   it != m_pending_content_table[i].m_pending_args.end (); it++)
                {
                  if (it->second == false)
                    {
                      //if there still is pending content for the entry
                      executable = false;
                    }
                }
              if (executable)
                {
                  ResolveAndHandleDecisions (m_pending_content_table[i]);
                }
            }
        }
    }
}

void
NfnProducerApp::ResolveAndHandleDecisions (InterestComponentStruct t_ics)
{
  //invoke interest resolution engine to get fetch decisions
  std::pair<NfnInterestResolutionEngine::NRE_Decision, shared_ptr<vector<Name>>> result;
  result = m_engine.GetFetchDecisions (t_ics.interest_ptr, m_compute_node);
  NS_LOG_DEBUG ("[NFN Producer: " << m_compute_node->GetName () << "] received resolution decision: "
                                 << result.first << std::endl);

  if (m_compute_node->CheckExcludeList (t_ics.m_func->getName ().toUri ())) //forward the request upstream as per orchestrator decision
  {
    //(m_compute_node->GetFunction(t_ics.m_func->getName().toUri()))->AddMissExecCounter();
    this->m_onFuncDisabledTrace (t_ics.m_func->getName ().toUri ());
    result.first = NfnInterestResolutionEngine::FORWARD;
  }

  if (result.first == NfnInterestResolutionEngine::NACK)
    { //resolution engine return null, means the interest is illegal or node is not capable of execution
      NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                     << "] Computation of Interest: " << t_ics.m_interest.toUri ()
                                     << "on node " << m_compute_node->GetName ()
                                     << " is not available" << std::endl);
      auto nack = make_shared<lp::Nack> (*t_ics.interest_ptr);
      nack->setReason (lp::NackReason::NO_ROUTE);
      m_appLink->onReceiveNack (*nack);
      NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                     << "] respond nack to consumer at: "
                                     << Simulator::Now ().GetSeconds () << std::endl);
    }

  else if (result.first == NfnInterestResolutionEngine::FETCH)
    {
      //store the pending content in a map, and send interests to fetch the content
      for (Name t_name : *result.second)
        {
          //update pending arguments table
          if (t_ics.m_pending_args.find (t_name) != t_ics.m_pending_args.end ())
            {
              t_ics.m_pending_args.find (t_name)->second = false;
            }
          else
            {
              t_ics.m_pending_args.emplace (t_name, false);
            }
          NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName () << "] Adds "
                                         << t_name.toUri () << " to pending content table"
                                         << std::endl);
          if (m_sending_interest_list.find (t_name) == m_sending_interest_list.end ())
            {
              //pending content has not been sent by other interest handling, then send
              m_sending_interest_list.insert (t_name);
              //send interest
              shared_ptr<Name> ptr_name = make_shared<Name> (t_name);
              shared_ptr<Interest> t_interest = make_shared<Interest> ();
              t_interest->setName (*ptr_name);
              t_interest->setCanBePrefix (false);
              t_interest->setMustBeFresh(true);
              NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                             << "] Sending Interest: " << *t_interest << std::endl);
              this->m_onOutgoingInterestTrace (t_interest);
              m_transmittedInterests (t_interest, this, m_face);
              m_appLink->onReceiveInterest (*t_interest);
            }
        }
      m_pending_content_table.insert (m_pending_content_table.begin (), t_ics);
    }

  else if (result.first == NfnInterestResolutionEngine::FORWARD)
    {
      NS_LOG_DEBUG("As per resolution result, interest " <<(t_ics.interest_ptr)->getName().toUri() <<" has to be forwarded");
      ForwardInterest(t_ics);
    }

  else if (result.first == NfnInterestResolutionEngine::EXECUTE)
    {
      //Did not go through fetch state.
      if (CheckResource (t_ics.m_func))
        {
          NS_LOG_DEBUG ("Resources available for execution :" << t_ics.m_func->getName ().toUri ()
                                                             << std::endl);
          DoExecution (t_ics);
        }
      else
        {
          //added to waiting queue
          if (!( m_compute_node->IsQueueFull() ))
            {
              NS_LOG_DEBUG ("Resources not available for execution, adding interest to queue:"
                           << t_ics.m_interest.toUri () << std::endl);
              m_compute_node->IncrementQueueFill();
              m_waiting_list.push (t_ics);
              this->m_onQueueInterestTrace(t_ics.m_interest.toUri());
            }
          else
            {
              m_compute_node->AddNodeBusyCounter ();
              this->m_onNodeBusyTrace (t_ics.m_func->getName ().toUri ());
              if (std::string ("push").compare (QUEUE_ACTION) == 0)
                {
                  NS_LOG_DEBUG ("The size of the queue is " << m_waiting_list.size ());
                  m_waiting_list.pop ();
                  m_waiting_list.push (t_ics);
                }
              else if (std::string ("nack").compare (QUEUE_ACTION) == 0)
                {
                  //If node resources are not available
                  NS_LOG_DEBUG ("[NFN producer: "
                               << m_compute_node->GetName ()
                               << "] Execute Function: " << t_ics.m_func->getName ()
                               << " failed, compute node resource not sufficient" << std::endl);
                  auto nack = make_shared<lp::Nack> (*t_ics.interest_ptr);
                  nack->setReason (lp::NackReason::CONGESTION);
                  m_appLink->onReceiveNack (*nack);
                  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                                 << "] respond nack to consumer at: "
                                                 << Simulator::Now ().GetSeconds () << std::endl);
                }
              else if (std::string ("forward").compare (QUEUE_ACTION) == 0)
                {
                  NS_LOG_DEBUG("Resource not available, interest " <<(t_ics.interest_ptr)->getName().toUri() <<" has to be forwarded");
                  ForwardInterest(t_ics);
                }
            }
        }
    }
}

//----------------------------------------------------------------------------//
//----------------------------Forward Interest--------------------------------//
//----------------------------------------------------------------------------//

void
NfnProducerApp::ForwardInterest(InterestComponentStruct t_ics)
{
  this->m_onForwardInterestTrace (t_ics.m_interest.toUri ());
  Ptr<L3Protocol> L3protocol = m_compute_node->GetNode ()->GetObject<L3Protocol> ();
  shared_ptr<nfd::Forwarder> forwarder = L3protocol->getForwarder ();
  shared_ptr<nfd::pit::Entry> pitEntry = forwarder->getPit ().find (*(t_ics.interest_ptr));
  nfd::pit::Pit& pit=forwarder->getPit();
  if(pitEntry != NULL)
  {
    //Delete the out Record so that the interest is not suppressed due to duplication
    //if((m_compute_node->GetName().compare("compute_node_8")==0) || (m_compute_node->GetName().compare("compute_node_7")==0))
    //{

    //}
  NS_LOG_INFO("Before deleting pit out record");
  for(auto it=pit.begin();it!=pit.end();it++)
    {
       if(it->getName() == t_ics.m_interest)
          {
            for(auto itr=it->getOutRecords().begin();itr!=it->getOutRecords().end() ;itr++)
              {
                NS_LOG_INFO("pit out record is "<<itr->getFace().getRemoteUri());
              }
          }
    }
  NS_LOG_INFO ("The pit entry name is " << pitEntry->getName ().toUri ()
                                                        << std::endl);
  NS_LOG_INFO("[NFN Producer: " << m_compute_node->GetName ()
                                << "] Deleting pit entry outRecord"
                                << std::endl);
  pitEntry->deleteOutRecord (*m_face);
  NS_LOG_INFO("After deleting pit out record");
  for(auto it=pit.begin();it!=pit.end();it++)
    {
       if(it->getName() == t_ics.m_interest)
          {
            for(auto itr=it->getOutRecords().begin();itr!=it->getOutRecords().end() ;itr++)
              {
                NS_LOG_INFO("pit out record is "<<itr->getFace().getRemoteUri());
              }
          }
    }
  }
  //Remove next hop to this face and disable function so that interest is not looped back to the same node
  nfd::fib::Fib& fib=forwarder->getFib();
  std::string prefix="/lambda/Function";
  auto entry=fib.findExactMatch(Name(prefix));
  if(entry==0)
  {
     NS_LOG_DEBUG("[INC Node] update FIB about function disable, no matching entry found in fib for prefix:"<<prefix);
  }
  else
  {
    auto nextHops=entry->getNextHops();
    int nH_size = nextHops.size();
    //find inter-app face and remove it
    for(int i=0;i<nH_size;i++)
    {
   	  if(entry->getNextHops().at(i).getFace().getRemoteUri().toString()=="nfnProducerAppFace://")
      {
        fib.addOrUpdateNextHop(*entry, *m_face, 40);
   		  NS_LOG_DEBUG("[INC Node] update FIB about function disable success: "<<prefix);
   	  }
    }
  }
  bool disable_flag=false;
  auto func = m_compute_node->GetFunction(t_ics.m_func->getName().toUri());
  bool func_flag = func->GetEnableStatus();
  NS_LOG_DEBUG("Function status in node is "<<func_flag);
  if(func_flag)
  {
    m_compute_node->notifyFibFunctionDisable(t_ics.m_func->getName().toUri());
    disable_flag = true;
  }
	//print updated fib before forwarding interest
  if(m_compute_node->GetName().compare("compute_node_1")==0)
  {
	  for(auto it=fib.begin();it!=fib.end();it++)
    {
      if(it->getPrefix().toUri().compare("/lambda/Function")==0)
      {
        std::cout<<"FIB Status before forwarding Interest:	Prefix:  "<<it->getPrefix().toUri()<<std::endl;
	      int size=it->getNextHops().size();
	      for(int i=0;i<size;i++)
        {
	        std::cout<<"Nexthop "<<i<<" :"<<it->getNextHops().at(i).getFace().getRemoteUri()<<" with cost "<< it->getNextHops().at(i).getCost()<<std::endl;
	      }
      }
   }
  }

  //send new interest with same name
  NS_LOG_INFO ("[NFN producer: " << m_compute_node->GetName ()
                                 << "] Sending Interest: " << *(t_ics.interest_ptr)
                                 << std::endl);
  Name fwd_interest_name = t_ics.m_interest;
  shared_ptr<Name> ptr_fwd_interest_name = make_shared<Name> (fwd_interest_name);
  shared_ptr<Interest> ptr_fwd_interest = make_shared<Interest> ();
  ptr_fwd_interest->setName (*ptr_fwd_interest_name);
  ptr_fwd_interest->setCanBePrefix (t_ics.interest_ptr->getCanBePrefix ());
  ptr_fwd_interest->setMustBeFresh (t_ics.interest_ptr->getMustBeFresh ());
  ptr_fwd_interest->setInterestLifetime(t_ics.interest_ptr->getInterestLifetime());
  ptr_fwd_interest->setHopLimit(t_ics.interest_ptr->getHopLimit());

  m_transmittedInterests(ptr_fwd_interest, this, m_face);
  m_appLink->onReceiveInterest(*ptr_fwd_interest);


  //Delete pit in record to avoid data packet to be routed back to this node
  NS_LOG_INFO("Before deleting pit in record");
  for(auto it=pit.begin();it!=pit.end();it++)
  {
    if(it->getName() == t_ics.m_interest)
    {
      for(auto itr=it->getInRecords().begin();itr!=it->getInRecords().end() ;itr++)
      {
        NS_LOG_INFO("In record is "<<itr->getFace().getRemoteUri());
      }
    }
  }
  if(pitEntry!=NULL)
  {
    if(pitEntry->hasInRecords() && (pitEntry->getInRecord(*m_face) != pitEntry->in_end()))
      pitEntry->deleteInRecord (*m_face);
    NS_LOG_INFO("After deleting in record : local face id is "<<*m_face);
    for(auto it=pit.begin();it!=pit.end();it++)
    {
      if(it->getName() == t_ics.m_interest)
      {
        for(auto itr=it->getInRecords().begin();itr!=it->getInRecords().end() ;itr++)
        {
          NS_LOG_INFO("In record is "<<itr->getFace().getRemoteUri());
        }
      }
    }
  }

  //After sending, check pit out record to see where the interest is forwarded
  NS_LOG_INFO("After forwarding, check which port the interested was forwarded to");
  for(auto it=pit.begin();it!=pit.end();it++)
    {
       if(it->getName() == t_ics.m_interest)
          {
            for(auto itr=it->getOutRecords().begin();itr!=it->getOutRecords().end() ;itr++)
              {
                NS_LOG_INFO("pit out record is "<<itr->getFace().getRemoteUri());
              }
          }
    }

  entry=fib.findExactMatch(Name(prefix));
  if(entry==0)
  {
    NS_LOG_DEBUG("[INC Node] update FIB about function enable, no matching entry found in fib for prefix:"<<prefix<<", will insert new entry into fib");
    entry=fib.insert(Name(prefix)).first;
  }
  //add face to the fib entry
  if(m_face)
  {
    fib.addOrUpdateNextHop(*entry, *m_face, 0);
	  NS_LOG_DEBUG("[INC Node] update FIB about function enable, finish update");
  }
  else
  {
  	NS_LOG_DEBUG("[INC Node] producer app face is null, update fails");
  }

  if(m_compute_node->GetName().compare("compute_node_1")==0)
  {
    for(auto it=fib.begin();it!=fib.end();it++)
    {
      if(it->getPrefix().toUri().compare("/lambda/Function")==0)
      {
        std::cout<<" FIB Status after forwarding Interest:	Prefix:  "<<it->getPrefix().toUri()<<std::endl;
	      int size=it->getNextHops().size();
	      for(int i=0;i<size;i++)
        {
	         std::cout<<"Nexthop "<<i<<" :"<<it->getNextHops().at(i).getFace().getRemoteUri()<<" with cost "<< it->getNextHops().at(i).getCost()<<std::endl;
	      }
      }
    }
  }
}

//----------------------------------------------------------------------------//
//---------On interest for compute, send interests for data arguments---------//
//----------------------------------------------------------------------------//
void
NfnProducerApp::OnInterest (shared_ptr<const Interest> interest)
{
  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName () << "] New interest arrived on node "
                                 << m_compute_node->GetName ()<<": "<<interest->getName().toUri());

  App::OnInterest (interest); // tracing inside
  NS_LOG_INFO (this << interest);
  if (!m_active)
    return;



  Name interest_name = interest->getName ();
  if (interest_name.getSubName (0, 1) == Name ("/lambda")) //is INC interest
    {
        if(m_compute_node->GetName().compare("compute_node_1") == 0)
        {
          std::cout<<" ---------------- Interest reaches NFN application -------------------------"<<std::endl;
        }
      this->m_onIncomingInterestTrace (interest);
      //create new InterestComponentStruct to store interest information
      InterestComponentStruct new_ics;
      new_ics.m_interest = interest_name;
      new_ics.interest_ptr = interest;
      //Find the function to be executed
      for (u_int32_t i = 0; i < interest_name.size (); i++)
        {
          if (interest_name.getSubName (i, 1) == Name ("/Function"))
            new_ics.m_func =
                m_compute_node->GetFunction (interest_name.getSubName (i + 1, 1).toUri ());
        }
      ResolveAndHandleDecisions (new_ics);
      return;
    }
  else
    //Handling interests for other non-compute related data from the node.
    {
      if (interest_name.getSubName (0, 1) == Name ("/Function")) //is function fetch request
        {

          NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                         << "] Received code drag interest: "
                                         << interest_name.toUri ());
          Name func_name = interest_name.getSubName (1, 1);
          Ptr<INC_Computation> func = m_compute_node->GetFunction (func_name.toUri ());
          if (func)
            {
              if (func->GetEnableStatus ())
                {
                  //function is enabled locally, then respond to function requester with dummy payload
                  //prepare data paket
                  NS_LOG_DEBUG ("[NFN producer: "
                               << m_compute_node->GetName ()
                               << "] Local function enabled, will respond to code drag interest"
                               << std::endl);
                  ns3::Time func_freshness = ns3::Time(Seconds(10));
                  auto data = PrepareDataPacket (interest_name, func->GetFuncSize (), func_freshness);
                  m_transmittedDatas (data, this,
                                      m_face); //Send the result of the execution as a data packet.
                  m_appLink->onReceiveData (*data);
                  NS_LOG_DEBUG ("[NFN producer: " << m_compute_node->GetName ()
                                                 << "] respond packet to consumer at: "
                                                 << Simulator::Now ().GetSeconds () << std::endl);
                }
              else
                {
                  NS_LOG_DEBUG (
                      "[NFN producer: "
                      << m_compute_node->GetName ()
                      << "] Local function not enabled, cannot respond to code drag request"
                      << std::endl);
                }
              return;
            }
          else
            {
              NS_LOG_DEBUG ("[NFN producer: "
                           << m_compute_node->GetName ()
                           << "] Local function not found, cannot respond to code drag request"
                           << std::endl);
            }
        }
      else
        { //normal data requests, respond with dummy packet
          ns3::Time data_freshness = ns3::Time(0);
          auto data = PrepareDataPacket (interest->getName (), m_virtualPayloadSize, data_freshness);
          m_transmittedDatas (data, this, m_face);
          m_appLink->onReceiveData (*data);
        }
    }
}
} //namespace inc
} // namespace ndn
} // namespace ns3
