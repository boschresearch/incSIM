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

#include "inc-orchestration-compute-node.h"
#include "ns3/log.h"
#include <boost/algorithm/string.hpp>


namespace ns3{
     namespace ndn{
        namespace inc{

    NS_LOG_COMPONENT_DEFINE ("IncOrchestrationComputeNode");

    NS_OBJECT_ENSURE_REGISTERED(IncOrchestrationComputeNode);

    TypeId IncOrchestrationComputeNode::GetTypeId (void)
    {
      static TypeId tid = TypeId ("ns3::ndn::inc::IncOrchestrationComputeNode")
        .SetParent<Object> ()
        .SetGroupName ("Network")
        .AddTraceSource("EmptyQueueTimer",
                      "Duration for which the node's queue was empty",
                      MakeTraceSourceAccessor(&IncOrchestrationComputeNode::m_emptyQueueTimer),
                      "ns3::ndn::inc::IncOrchestrationComputeNode::EmptyQueueCallback")
        .AddTraceSource("FullQueueTimer",
                      "Duration for which the node's queue was full",
                      MakeTraceSourceAccessor(&IncOrchestrationComputeNode::m_fullQueueTimer),
                      "ns3::ndn::inc::IncOrchestrationComputeNode::FullQueueCallback")
        ;

      return tid;
    }

    IncOrchestrationComputeNode::IncOrchestrationComputeNode(){
        Ptr<Node> n=CreateObject<Node> ();
        this->SetNode(n);
        m_provided_data = "null";
        m_full_timer_started=false;
        m_empty_timer_started = false;
        //InitialFunctionRegister();
        NS_LOG_FUNCTION (this);
    }

    IncOrchestrationComputeNode::IncOrchestrationComputeNode(uint32_t sid){
        Ptr<Node> n=CreateObject<Node> (sid);
        this->SetNode(n);
        m_provided_data = "null";
        //InitialFunctionRegister();
        NS_LOG_FUNCTION (this);
    }

    IncOrchestrationComputeNode::~IncOrchestrationComputeNode(){
        NS_LOG_FUNCTION (this);
    }


    uint32_t IncOrchestrationComputeNode::GetUUID(){
    	return this->m_uuid;
    }

    void IncOrchestrationComputeNode::SetUUID(uint32_t value){
    	this->m_uuid=value;
    }

    uint32_t IncOrchestrationComputeNode::GetProcessorCore(){
    	return this->m_processor_core;

    }
    void IncOrchestrationComputeNode::SetProcessorCore(uint32_t value){
    	this->m_processor_core=value;
    }

    uint32_t IncOrchestrationComputeNode::GetProcessorClockSpeed(){
     	return this->m_processor_speed;
    }

    void IncOrchestrationComputeNode::SetProcessorClockSpeed(uint32_t value){
     	this->m_processor_speed=value;
    }

     uint32_t IncOrchestrationComputeNode::GetRam(){
     	return this->m_ram;
     }

     void IncOrchestrationComputeNode::SetRam(uint32_t value){
        this->m_ram=value;
     }

     uint32_t IncOrchestrationComputeNode::GetRom(){
        return this->m_rom;
     }

     void IncOrchestrationComputeNode::SetRom(uint32_t value){
     	this->m_rom=value;
     }

     uint32_t IncOrchestrationComputeNode::GetQueueSize(){
     	return this->m_queue_size;
     }

     void IncOrchestrationComputeNode::SetQueueSize(uint32_t value){
        this->m_queue_size=value;
        this->m_queue_fill = 0;
     }

     bool IncOrchestrationComputeNode::IncrementQueueFill()
     {
       if(this->m_queue_fill < this->m_queue_size)
       {
          if(m_empty_timer_started==true)
          {
            m_empty_timer_started=false;
            std::cout<<"empty time duration is "<<Simulator::Now().GetSeconds()-m_q_empty_time<<std::endl;
            m_emptyQueueTimer(Simulator::Now().GetSeconds()-m_q_empty_time);
          }
          this->m_queue_fill++;
          return true;
       }
        else
        {
          if(m_full_timer_started==false)
          {
            m_q_full_time = Simulator::Now().GetSeconds();
            m_full_timer_started=true;
          }
          return false;
        }
     }

     bool IncOrchestrationComputeNode::DecrementQueueFill()
     {
       if(this->m_queue_fill > 0)
       {
         if(m_full_timer_started==true)
         {
           m_full_timer_started=false;
           m_fullQueueTimer(Simulator::Now().GetSeconds()-m_q_full_time);
         }
         this->m_queue_fill--;
         return true;
       }
       else
       {
         if(m_empty_timer_started==false)
         {
          m_q_empty_time = Simulator::Now().GetSeconds();
          std::cout<<"m_q_empty_time is "<<m_q_empty_time<<std::endl;
          m_empty_timer_started=true;
         }
         return false;
       }
     }

     bool IncOrchestrationComputeNode::IsQueueFull()
     {
       if (this->m_queue_fill == this->m_queue_size)
        return true;
       else
        return false;
     }

     bool IncOrchestrationComputeNode::IsQueueEmpty()
     {
       if (this->m_queue_fill == 0)
        return true;
       else
        return false;
     }

     std::string IncOrchestrationComputeNode::GetLinks(){
     	return this->m_links;
     }

     void IncOrchestrationComputeNode::SetLinks(std::string value){
     	this->m_links=value;
     }


     std::string IncOrchestrationComputeNode::GetProvidedData(){
       return this->m_provided_data;
     }

     bool IncOrchestrationComputeNode::CheckProvidedDataList(std::string value)
     {
        std::size_t found = this->m_provided_data.find(value);
        if(found == this->m_provided_data.npos)
          return false;
        else
          return true;
     }

     void IncOrchestrationComputeNode::AddProvidedData(std::string value){
       if(this->m_provided_data.compare("null")==0)
       {
         this->m_provided_data.clear();
         this->m_provided_data.append(value);
       }
       else
       {
         std::size_t found = this->m_provided_data.find(value);
         if(found == this->m_provided_data.npos)
         {
          this->m_provided_data.append(","+value);
          std::cout<<"Added data to node : "<<this->GetName()<<std::endl;
         }
       }
     }

     std::string IncOrchestrationComputeNode::GetName(){
     	return this->m_name;
     }

     void IncOrchestrationComputeNode::SetName(std::string value){
     	this->m_name=value;
     }

     std::string IncOrchestrationComputeNode::GetSupportedRuntimes(){
     	return this->m_supported_runtimes;
     }

     void IncOrchestrationComputeNode::SetSupportedRuntimes(std::string value){
     	this->m_supported_runtimes=value;
     }

     Ptr<Node> IncOrchestrationComputeNode::GetNode(){
         return this->m_node;
     }

    void IncOrchestrationComputeNode::SetNode(Ptr<Node> value){
        this->m_node=value;

    }
    uint32_t
    IncOrchestrationComputeNode::GetNodeBusyCounter()
    {
      return m_nodeBusyCounter;
    }

    void
    IncOrchestrationComputeNode::ResetNodeBusyCounter()
    {
      m_nodeBusyCounter = 0;
    }

    void
    IncOrchestrationComputeNode::AddNodeBusyCounter()
    {
      m_nodeBusyCounter = m_nodeBusyCounter + 1;
    }

    shared_ptr<Face>
	IncOrchestrationComputeNode::GetProducerAppFace(){
		return m_producer_app_face;
	}

	void
	IncOrchestrationComputeNode::SetProducerAppFace(shared_ptr<Face> face){
		m_producer_app_face=face;
	}

	std::string
	IncOrchestrationComputeNode::GetIncStrategy(){
		return m_inc_strategy;
	}

    void
	IncOrchestrationComputeNode::SetIncStragety(std::string inc_strategy){
    	m_inc_strategy=inc_strategy;
    }

    bool
    IncOrchestrationComputeNode::EnableFunction(std::string func_name){
    if(m_func_map.find(func_name)==m_func_map.end()){

        NS_LOG_INFO("[INC Node] function "<<func_name<<" not found");
        return false;
    }else{
        auto cur_func = m_func_map.at(func_name);



        cur_func->Enable();

        //if the network is on top of NFN, need to modify FIB in order to forward corresponding interest to local producer app
        if(boost::algorithm::to_lower_copy(m_inc_strategy)=="nfn" && m_producer_app_face){
                	notifyFibFunctionEnable(func_name);
		}

        NS_LOG_INFO("[INC Node] m_enable of function "<<func_name<<" on node "<<this->GetName()<<" is " << cur_func->GetEnableStatus());

        return cur_func->GetEnableStatus()==1?true:false;
    }

    }

    bool
    IncOrchestrationComputeNode::EnableDataForCompute(std::string data_name)
    {
      if(boost::algorithm::to_lower_copy(m_inc_strategy)=="nfn" && m_producer_app_face)
      {
        notifyFibDataEnable(data_name);
      }
      return true; //currently no check for data enable implemented. always returns true.
    }

    void
	IncOrchestrationComputeNode::SetFunctionMap(std::unordered_map<std::string,Ptr<INC_Computation>> map){
      m_func_map=map;
    }

    std::unordered_map<std::string,Ptr<INC_Computation>>
    IncOrchestrationComputeNode::GetFunctionMap(void){
      return m_func_map;
    }

    void
	IncOrchestrationComputeNode::AddNewFunction(std::string name, Ptr<INC_Computation> function){
      Ptr<INC_Computation> t_func=CreateObject<INC_Computation>();
      t_func->setName(function->getName());
      t_func->SetCpu(function->GetCpu());
      t_func->SetRam(function->GetRam());
      t_func->SetRom(function->GetRom());
      t_func->SetParamNumber(function->GetParamNumber());
      t_func->SetExecTime(function->GetExecTime());
      t_func->SetRuntimeEnvironment(function->GetRuntimeEnvironment());
      t_func->SetFuncSize(function->GetFuncSize());
      t_func->SetInputList(function->GetInputList());
      m_func_map.emplace(name,t_func);
    }

    Ptr<INC_Computation>
	  IncOrchestrationComputeNode::GetFunction(std::string name){
      return m_func_map[name];
    }

    bool
	IncOrchestrationComputeNode::DisableFunction(std::string func_name){

      if(m_func_map.find(func_name)==m_func_map.end()){
        NS_LOG_INFO("[INC Node] function "<<func_name<<" not found");
        return false;
      }else{
    	Ptr<INC_Computation> cur_func=m_func_map[func_name];


        cur_func->Disable();

        //if the network is on top of NFN, need to delete local producer face in function's entry
		if(boost::algorithm::to_lower_copy(m_inc_strategy)=="nfn" && m_producer_app_face){
			notifyFibFunctionDisable(func_name);
		}
        NS_LOG_INFO("[INC Node] m_enable of function"<<func_name<<" on node "<<this->GetName()<<" is " << cur_func->GetEnableStatus());
        return cur_func->GetEnableStatus()==0?true:false;
      }
    }


      void
   	  IncOrchestrationComputeNode::notifyFibFunctionEnable(std::string func_name)
      {
        //get the fib of local forwarder
   		  shared_ptr<nfd::Forwarder> forwarder = m_node->GetObject<L3Protocol>()->getForwarder();
   		  nfd::fib::Fib& fib=forwarder->getFib();
   		  //find the fib entry corresponding to the function
   		  std::string prefix="/lambda/Function"+func_name;
   		  auto entry=fib.findExactMatch(Name(prefix));
   		  if(entry==0)
        {
   			  NS_LOG_INFO("[INC Node] update FIB about function enable, no matching entry found in fib for prefix:"<<prefix<<", will insert new entry into fib");
   			  entry=fib.insert(Name(prefix)).first;
   		  }
    	  //add face to the fib entry
   		  if(m_producer_app_face)
        {
   			  fib.addOrUpdateNextHop(*entry, *m_producer_app_face, 0);
			    NS_LOG_INFO("[INC Node] update FIB about function enable, finish update");
   		  }
        else
        {
   		  	NS_LOG_INFO("[INC Node] producer app face is null, update fails");
   		  }
      }

      void
   	  IncOrchestrationComputeNode::notifyFibDataEnable(std::string data_name)
      {
       	//get the fib of local forwarder
     		shared_ptr<nfd::Forwarder> forwarder = m_node->GetObject<L3Protocol>()->getForwarder();
     		nfd::fib::Fib& fib=forwarder->getFib();
   	  	//find the fib entry corresponding to the function
   	    std::string prefix="/lambda/Data"+data_name+"/";
   		  auto entry=fib.findExactMatch(Name(prefix));
   		  if(entry==0)
        {
   			  NS_LOG_INFO("[INC Node] update FIB about data enable, no matching entry found in fib for prefix:"<<prefix<<", will insert new entry into fib");
   			  entry=fib.insert(Name(prefix)).first;
   		  }
        //add face to the fib entry
   		  if(m_producer_app_face)
        {
   			  fib.addOrUpdateNextHop(*entry, *m_producer_app_face, 0);
			    NS_LOG_INFO("[INC Node] update FIB about Data enable, finish update");
   		  }
        else
        {
   		  	NS_LOG_INFO("[INC Node] producer app face is null, update fails");
   		  }
      }


      void
	    IncOrchestrationComputeNode::notifyFibFunctionDisable(std::string func_name)
      {
       	//get the fib of local forwarder
   		  shared_ptr<nfd::Forwarder> forwarder = m_node->GetObject<L3Protocol>()->getForwarder();
   		  nfd::fib::Fib& fib=forwarder->getFib();

   		  //find the fib entry corresponding to the function
   		  std::string prefix="/lambda/Function"+func_name;
   		  auto entry=fib.findExactMatch(Name(prefix));
   		  if(entry==0)
        {
   		  	NS_LOG_INFO("[INC Node] update FIB about function disable, no matching entry found in fib for prefix:"<<prefix);
   			  return;
   		  }
   		  auto nextHops=entry->getNextHops();
   		  //find inter-app face and remove it
   		  for(int i=0;i<nextHops.size();i++)
        {
   			  if(entry->getNextHops().at(i).getFace().getRemoteUri().toString()=="nfnProducerAppFace://")
          {
   				  fib.removeNextHop(*entry ,entry->getNextHops().at(i).getFace());
   				  NS_LOG_INFO("[INC Node] update FIB about function disable success: "<<prefix);
   				  return;
   			  }
   		  }
   		  NS_LOG_INFO("[INC Node] update FIB about function disable, no matching next hop found in fib for prefix:"<<prefix);
   	  }


      void
	    IncOrchestrationComputeNode::notifyFibDataDisable(std::string data_name)
      {
       	//get the fib of local forwarder
   		  shared_ptr<nfd::Forwarder> forwarder = m_node->GetObject<L3Protocol>()->getForwarder();
   		  nfd::fib::Fib& fib=forwarder->getFib();

   		  //find the fib entry corresponding to the function
   		  std::string prefix="/lambda/Data"+data_name;
   		  auto entry=fib.findExactMatch(Name(prefix));
   		  if(entry==0)
        {
   		  	NS_LOG_INFO("[INC Node] update FIB about data disable, no matching entry found in fib for prefix:"<<prefix);
   			  return;
   		  }
   		  auto nextHops=entry->getNextHops();
   		  //find inter-app face and remove it
   		  for(int i=0;i<nextHops.size();i++)
        {
   			  if(entry->getNextHops().at(i).getFace().getRemoteUri().toString()=="nfnProducerAppFace://")
          {
   				  fib.removeNextHop(*entry ,entry->getNextHops().at(i).getFace());
   				  NS_LOG_INFO("[INC Node] update FIB about data disable success: "<<prefix);
   				  return;
   			  }
   		  }
   		  NS_LOG_INFO("[INC Node] update FIB about data disable, no matching next hop found in fib for prefix:"<<prefix);
   	  }

      void
      IncOrchestrationComputeNode::AddToExcludeList(std::string function_name)
      {
        m_FuncExcludeList.push_back(function_name);
      }

      void
      IncOrchestrationComputeNode::RemoveFromExcludeList(std::string function_name)
      {
        auto itr = std::find(m_FuncExcludeList.begin(), m_FuncExcludeList.end(), function_name);
        if (itr != m_FuncExcludeList.end()) m_FuncExcludeList.erase(itr);
      }

      bool
      IncOrchestrationComputeNode::CheckExcludeList(std::string function_name)
      {
        auto itr = std::find(m_FuncExcludeList.begin(), m_FuncExcludeList.end(), function_name);
        if (itr != m_FuncExcludeList.end()){
          return true;
        }
        else
        {
          return false;
        }

      }


    }
  }
}
