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

#include "ndn-orchestration-node-app.hpp"

#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/incSIM-module.h"
#include "ns3/random-variable-stream.h"

NS_LOG_COMPONENT_DEFINE("ndn.inc.NdnOrchestrationComputeNodeApp");
    /*
  Orchestrator provider application to be installed on nodes in order to control the enable status of the compute functions on them
  */
namespace ns3{
namespace ndn{
namespace inc{
    NS_OBJECT_ENSURE_REGISTERED(NdnOrchestrationComputeNodeApp);
    TypeId
    NdnOrchestrationComputeNodeApp::GetTypeId()
    {
      static TypeId tid =
          TypeId("ns3::ndn::inc::NdnOrchestrationComputeNodeApp")
              .SetParent<App>()
              .AddConstructor<NdnOrchestrationComputeNodeApp>()
              .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue(""),
                            MakeNameAccessor(&NdnOrchestrationComputeNodeApp::m_prefix), MakeNameChecker())
              .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                            MakeUintegerAccessor(&NdnOrchestrationComputeNodeApp::m_virtualPayloadSize),
                            MakeUintegerChecker<uint32_t>())
              .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                            TimeValue(Seconds(1)), MakeTimeAccessor(&NdnOrchestrationComputeNodeApp::m_freshness),
                            MakeTimeChecker())
              .AddAttribute("Signature", "Fake signature, 0 valid signature (default), other values application-specific",
                            UintegerValue(0), MakeUintegerAccessor(&NdnOrchestrationComputeNodeApp::m_signature),
                            MakeUintegerChecker<uint32_t>())
              .AddAttribute ("ComputeNodePointer", "the pointer of compute node to access node info",
                            PointerValue (), MakePointerAccessor (&NdnOrchestrationComputeNodeApp::m_compute_node),
                            MakePointerChecker<IncOrchestrationComputeNode> ())
              .AddAttribute("KeyLocator",
                            "Name to be used for key locator.  If root, then key locator is not used",
                            NameValue(), MakeNameAccessor(&NdnOrchestrationComputeNodeApp::m_keyLocator), MakeNameChecker())
              .AddTraceSource("EnableFunction",
                            "Trace called every time there is an enable function action",
                            MakeTraceSourceAccessor(&NdnOrchestrationComputeNodeApp::m_onFuncEnableTrace),
                            "ns3::ndn::inc::NdnOrchestrationComputeNodeApp::EnableFunctionCallback")
              .AddTraceSource("DisableFunction",
                            "Trace called every time there is an enable function action",
                            MakeTraceSourceAccessor(&NdnOrchestrationComputeNodeApp::m_onFuncDisableTrace),
                            "ns3::ndn::inc::NdnOrchestrationComputeNodeApp::DisableFunctionCallback");
      return tid;
    }

    NdnOrchestrationComputeNodeApp::NdnOrchestrationComputeNodeApp()
    {
      NS_LOG_FUNCTION_NOARGS();
    }

    NdnOrchestrationComputeNodeApp::~NdnOrchestrationComputeNodeApp() {}
    // inherited from Application base class.
    void
    NdnOrchestrationComputeNodeApp::StartApplication()
    {
      NS_LOG_FUNCTION_NOARGS();
      App::StartApplication();
      FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
    }

    void
    NdnOrchestrationComputeNodeApp::StopApplication()
    {
      NS_LOG_FUNCTION_NOARGS();
      App::StopApplication();
    }

    void
    NdnOrchestrationComputeNodeApp::SetComputeNode(Ptr<IncOrchestrationComputeNode> cn){
	  m_compute_node=cn;
    }

    Ptr<IncOrchestrationComputeNode>
    NdnOrchestrationComputeNodeApp::GetComputeNode(void){
	  return m_compute_node;
    }
    /*
    Callback function on receiving an interest.
    Fetches the INC Compute application running on the host.
    Modifies the enable status of computation based on the interest packet.
    Acknowledges the changes with a data packet.
    */
    void
    NdnOrchestrationComputeNodeApp::OnInterest(shared_ptr<const Interest> interest)
    {
      App::OnInterest(interest); // tracing inside
      NS_LOG_FUNCTION(this << interest);
      if (!m_active)
        return;
      m_interest = interest->getName();
      if(m_interest.getSubName(0,1)== "/Orchestrator")
      {
        OrchestratorRequestResolution(m_interest);
      }
      else
      {
        auto data = make_shared<Data>();
        data->setName(m_interest);
        data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));
        data->setContent(make_shared<::ndn::Buffer>(m_virtualPayloadSize));
        Signature signature;
        SignatureInfo signatureInfo(static_cast<::ndn::tlv::SignatureTypeValue>(255));
        if (m_keyLocator.size() > 0)
        {
          signatureInfo.setKeyLocator(m_keyLocator);
        }
        signature.setInfo(signatureInfo);
        signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));
        data->setSignature(signature);
        NS_LOG_INFO("[Node Orchestrator App] node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());
        // to create real wire encoding
        data->wireEncode();
        m_transmittedDatas(data, this, m_face);
        m_appLink->onReceiveData(*data);
      }
    }

  void
  NdnOrchestrationComputeNodeApp::OrchestratorRequestResolution(Name m_interest)
  {
      std::string payload;
      if((m_interest.getSubName(2,1) == "/BootstrapInfo") || (m_interest.getSubName(2,1)== "/NodeStatusFetch"))
      {
        payload = StatusFetchHandler(m_interest);
        std::cout<<payload<<std::endl;
      }
      else if(m_interest.getSubName(2,1)== "/FunctionStatus")
      {
        payload = FunctionStatusRequestHandler(m_interest);
      }
      else if(m_interest.getSubName(2,1)== "/FunctionSwitch")
      {
        NS_LOG_INFO("Inside ndn orchestration node app: Orchestration Request Resolution : FunctionSwitch");
        payload = FunctionSwitchHandler(m_interest);
      }
      else
      {
        payload = "Unknown request";
      }
      auto data = make_shared<Data>();
      data->setName(m_interest);
      //data->setContent(make_shared<::ndn::Buffer>(m_virtualPayloadSize));
      data->setContent(::ndn::encoding::makeStringBlock(::ndn::tlv::Content, payload));
      Signature signature;
      SignatureInfo signatureInfo(static_cast<::ndn::tlv::SignatureTypeValue>(255));
      if (m_keyLocator.size() > 0)
      {
        signatureInfo.setKeyLocator(m_keyLocator);
      }
      signature.setInfo(signatureInfo);
      signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));
      data->setSignature(signature);
      data->setFreshnessPeriod(::ndn::time::milliseconds(1));
      NS_LOG_INFO("[Node Orchestrator App] node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());
      NS_LOG_INFO("[Node Orchestrator App] node(" << GetNode()->GetId() << ") responding with Data: " << data->getContent().value());
      // to create real wire encoding
      data->wireEncode();
      m_transmittedDatas(data, this, m_face);
      m_appLink->onReceiveData(*data);
  }

  std::string
  NdnOrchestrationComputeNodeApp::StatusFetchHandler(Name m_interest){
	NS_LOG_INFO("[Node Orchestrator App] Received node status request from orchestrator"<<std::endl);
  if(m_interest.getSubName(2,1) == "/BootstrapInfo")
  {
    m_compute_node->ResetNodeBusyCounter();
  }

	std::stringstream ss;
	ss << "name=/"<< m_compute_node->GetName()<< ";"
	<< "processor_core="<< m_compute_node->GetProcessorCore() << ";"
	<< "processor_type="<<m_compute_node->GetProcessorClockSpeed()<<";"
	<< "RAM=" << m_compute_node->GetRam() << ";"
	<< "ROM=" << m_compute_node->GetRom() << ";"
	<< "links=" << m_compute_node->GetLinks() << ";"
	<< "runtimes=" << m_compute_node->GetSupportedRuntimes() << ";"
  << "node_busy_counter="<<m_compute_node->GetNodeBusyCounter()<<";"
  << "data="<<m_compute_node->GetProvidedData()<<";"
  << "functions="
	//<< "exec times=" << m_compute_node->GetFunction("/SumOfNumbers")->GetCounter() << ";"
	;
	for(auto& it:m_compute_node->GetFunctionMap()){
	        	ss<<it.first<<":"
	        		<<it.second->GetEnableStatus()<<":"
						  <<it.second->GetCounter()<<":"
						  <<it.second->GetMissExecCounter()<<":"
              <<it.second->GetCpu()<<":"
              <<it.second->GetRam()<<":"
              <<it.second->GetRom()<<":"
              <<it.second->GetFuncSize()<<":"
              <<it.second->GetInputList();
	        }
	std::string payload = ss.str();
  m_compute_node->ResetNodeBusyCounter();
	return payload;
}

std::string
NdnOrchestrationComputeNodeApp::FunctionStatusRequestHandler(Name m_interest){
	NS_LOG_INFO("[Node Orchestrator App] Received function counter request from orchestrator"<<std::endl);
	std::stringstream ss;
	ss<<"Function status from node: "<<m_compute_node->GetName()<<std::endl;
	for(auto& it:m_compute_node->GetFunctionMap()){
	        	ss<<"function_name="<<it.first<<";"
              <<"enable_status="<<it.second->GetEnableStatus()<<";"
              <<"execution_counter="<<it.second->GetCounter()<<";"<<std::endl;
	        }
	std::string payload = ss.str();
	return payload;
}

std::string
NdnOrchestrationComputeNodeApp::FunctionSwitchHandler(Name m_interest){
	NS_LOG_INFO("[Node Orchestrator App] Received enable/disable function request from orchestrator"<<std::endl);
	std::string operation = (m_interest.getSubName(3,1)).toUri(); // eg name: Orchestrator/Node1/FunctionSwitch/Disable/Func1/Func2/Func3
  bool flag;

  for(uint32_t i = 4; i<m_interest.size();i++)
  {
	  std::string func_name = (m_interest.getSubName(i,1)).toUri();

	  if(operation=="/Enable"){
		  flag=GetComputeNode()->EnableFunction(func_name);
      if(GetComputeNode()->CheckExcludeList(func_name))
        GetComputeNode()->RemoveFromExcludeList(func_name);
      if(flag)
      {
        this->m_onFuncEnableTrace(func_name);
      }
	  }
    if(operation == "/Disable"){
		  flag=GetComputeNode()->DisableFunction(func_name);
      GetComputeNode()->AddToExcludeList(func_name);
      if(flag)
      {
        this->m_onFuncDisableTrace(func_name);
      }
	  }
  }
	std::string payload;

	if(flag){
	  payload="operation success";
	}else{
	  payload="operation failed";
	}
	return payload;
}



      }//namespace inc
  } // namespace ndn
} // namespace ns3
