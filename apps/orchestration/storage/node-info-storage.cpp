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
 * *****************************************************************************
 */

#include "node-info-storage.hpp"
#include "ns3/log.h"

#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/channel-list.h"
#include "ns3/object-factory.h"

#include "ns3/ndnSIM/model/ndn-global-router.hpp"
#include "ns3/ndnSIM/helper/boost-graph-ndn-global-routing-helper.hpp"
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include "ns3/node-list.h"


NS_LOG_COMPONENT_DEFINE ("OrchestratorNodeInfoStorage");
namespace ns3{
     namespace ndn{
        namespace inc{

    NS_OBJECT_ENSURE_REGISTERED(OrchestratorNodeInfoStorage);

    TypeId
    OrchestratorNodeInfoStorage::GetTypeId (void)
    {
      static TypeId tid = TypeId ("ns3::ndn::inc::OrchestratorNodeInfoStorage")
        .SetParent<Object> ()
        .AddConstructor<OrchestratorNodeInfoStorage>();

      return tid;
    }

    OrchestratorNodeInfoStorage::OrchestratorNodeInfoStorage(){
        NS_LOG_FUNCTION (this);
    }

    OrchestratorNodeInfoStorage::~OrchestratorNodeInfoStorage(){
        NS_LOG_FUNCTION (this);
    }
    map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode> OrchestratorNodeInfoStorage::m_nodeInfoTable;

    std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_iterator
    OrchestratorNodeInfoStorage::cbegin()
    {
        return m_nodeInfoTable.cbegin();
    }

    std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_iterator
    OrchestratorNodeInfoStorage::cend()
    {
        return m_nodeInfoTable.cend();
    }

    std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_reverse_iterator
    OrchestratorNodeInfoStorage::crbegin()
    {
        return m_nodeInfoTable.crbegin();
    }

    std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_reverse_iterator
    OrchestratorNodeInfoStorage::crend()
    {
        return m_nodeInfoTable.crend();
    }


//Node info
    OrchestratorNodeInfoStorage::computeNode
    OrchestratorNodeInfoStorage::getNodeInfo(std::string nodeID)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            return itr->second;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
        }
    }

    void
    OrchestratorNodeInfoStorage::printCurrentNodeInfo(std::string nodeID)
    {
        std::cout<<"Print Current Node Info";
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            std::cout<< "Name : "<<(itr->second).nodeName<<std::endl
            <<"CPU :"<<(itr->second).cpu.second<<std::endl
            <<"RAM :"<<(itr->second).ram.second<<std::endl
            <<"ROM :"<<(itr->second).rom.second<<std::endl;
            for(auto i = (itr->second.functions_current).begin(); i != ((itr->second).functions_current).end(); i++)
                std::cout<<"Function Info ="<<i->funcName<<"-"<<i->status<<":"<<i->func_exe_counter<<";"<<std::endl;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
        }
    }

    void
    OrchestratorNodeInfoStorage::printDefaultNodeInfo(std::string nodeID)
    {
        std::cout<<"Print Default Node Info"<<std::endl;
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            std::cout<< "Name : "<<(itr->second).nodeName<<std::endl
            <<"CPU :"<<(itr->second).cpu.first<<std::endl
            <<"RAM :"<<(itr->second).ram.first<<std::endl
            <<"ROM :"<<(itr->second).rom.first<<std::endl;
            for(auto i = (itr->second.functions_current).begin(); i != ((itr->second).functions_current).end(); i++)
                std::cout<<"Function Info ="<<i->funcName<<"-"<<i->status<<":"<<i->func_exe_counter<<";"<<std::endl;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
        }
    }

    bool
    OrchestratorNodeInfoStorage::AddNodeToTable(std::string nodeID, computeNode nodeToAdd)
    {
        pair <map<std::string, computeNode>::iterator, bool> itr;
        itr = m_nodeInfoTable.insert(std::pair<std::string, computeNode>(nodeID,nodeToAdd));
        //std::cout<<"Node Table updated :"<<m_nodeInfoTable.size();
        return itr.second;
    }

    uint32_t
    OrchestratorNodeInfoStorage::getTableSize()
    {
        return m_nodeInfoTable.size();
    }

    Name
    OrchestratorNodeInfoStorage::getNodeName(std::string nodeID)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            return (itr->second).nodeName;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setNodeName(std::string nodeID,Name nameToSet)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            (itr->second).nodeName = nameToSet;
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    Ptr<ns3::ndn::inc::IncOrchestrationComputeNode>
    OrchestratorNodeInfoStorage::getNodePointer(std::string nodeID)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            return (itr->second).node_ptr;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setNodePointer(std::string nodeID,Ptr<ns3::ndn::inc::IncOrchestrationComputeNode> pointerToSet)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            (itr->second).node_ptr = pointerToSet;
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    uint32_t
    OrchestratorNodeInfoStorage::getNodeRAM(std::string nodeID, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)return (itr->second).ram.first;
            if (defaultvalue == false)return (itr->second).ram.second;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return 0;
        }

    }

    bool
    OrchestratorNodeInfoStorage::setNodeRAM(std::string nodeID, uint32_t ramToSet, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)(itr->second).ram.first = ramToSet;
            if (defaultvalue == false)(itr->second).ram.second = ramToSet;
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    uint32_t
    OrchestratorNodeInfoStorage::getNodeROM(std::string nodeID, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)return (itr->second).rom.first;
            if (defaultvalue == false)return (itr->second).rom.second;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return 0;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setNodeROM(std::string nodeID, uint32_t romToSet, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)(itr->second).rom.first=romToSet;
            if (defaultvalue == false)(itr->second).rom.second=romToSet;
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    uint32_t
    OrchestratorNodeInfoStorage::getNodeBusyCounter(std::string nodeID, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)return (itr->second).nodeBusyCounter.first;
            if (defaultvalue == false)return (itr->second).nodeBusyCounter.second;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return 0;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setNodeBusyCounter(std::string nodeID, uint32_t nbToSet, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)(itr->second).nodeBusyCounter.first=nbToSet;
            if (defaultvalue == false)(itr->second).nodeBusyCounter.second=nbToSet;
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }


    uint32_t
    OrchestratorNodeInfoStorage::getNodeCPU(std::string nodeID, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)return (itr->second).cpu.first;
            if (defaultvalue == false)return (itr->second).cpu.second;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return 0;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setNodeCPU(std::string nodeID, uint32_t cpuToSet, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)(itr->second).cpu.first=cpuToSet;
            if (defaultvalue == false)(itr->second).ram.second=cpuToSet;
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    std::vector<std::string>
    OrchestratorNodeInfoStorage::getNodeLinks(std::string nodeID, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)return (itr->second).links.first;
            if (defaultvalue == false)return (itr->second).links.second;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
        }
    }

        bool
    OrchestratorNodeInfoStorage::setNodeLinks(std::string nodeID, std::vector<std::string> linksToSet, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)(itr->second).links.first=linksToSet;
            if (defaultvalue == false)(itr->second).links.second=linksToSet;
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::addLinkToNode(std::string nodeID, string linkToAdd, bool defaultvalue=false)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if(defaultvalue==true)
            ((itr->second).links.first).push_back(linkToAdd);
            else
            ((itr->second).links.second).push_back(linkToAdd);
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    std::vector<std::string>
    OrchestratorNodeInfoStorage::getNodeRuntimes(std::string nodeID, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)return (itr->second).runtimes_supported.first;
            if (defaultvalue == false)return (itr->second).runtimes_supported.second;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setNodeRuntimes(std::string nodeID, std::vector<std::string>rtToSet, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if (defaultvalue == true)(itr->second).runtimes_supported.first=rtToSet;
            if (defaultvalue == false)(itr->second).runtimes_supported.second=rtToSet;
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::addRuntimeToNode(std::string nodeID, string rtToAdd, bool defaultvalue=false)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if(defaultvalue==true)((itr->second).runtimes_supported.first).push_back(rtToAdd);
            if(defaultvalue==false)((itr->second).runtimes_supported.second).push_back(rtToAdd);
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    std::vector<OrchestratorNodeInfoStorage::functionInfo>
    OrchestratorNodeInfoStorage::getFunctionsAtNode(std::string nodeID, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
            if(defaultvalue==true)return (itr->second).functions_initial;
            if(defaultvalue==false)return (itr->second).functions_current;
        }

        else{
            std::cout<<"Node not found"<<std::endl;
        }
    }

    bool
    OrchestratorNodeInfoStorage::addFunctionToNode(std::string nodeID, functionInfo funcToAdd, bool defaultvalue = false)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if(defaultvalue==true)((itr->second).functions_initial).push_back(funcToAdd);
            if(defaultvalue==false)((itr->second).functions_current).push_back(funcToAdd);
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    std::vector<OrchestratorNodeInfoStorage::dataInfo>
    OrchestratorNodeInfoStorage::getDataAtNode(std::string nodeID, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
            if(defaultvalue==true)return (itr->second).data_initial;
            if(defaultvalue==false)return (itr->second).data_current;
        }

        else{
            std::cout<<"Node not found"<<std::endl;
        }
    }

    bool
    OrchestratorNodeInfoStorage::addDataToNode(std::string nodeID, dataInfo dataToAdd, bool defaultvalue = false)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if(defaultvalue==true)((itr->second).data_initial).push_back(dataToAdd);
            if(defaultvalue==false)((itr->second).data_current).push_back(dataToAdd);
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setNodeDataList(std::string nodeID, std::vector<dataInfo> data_list, bool defaultvalue = false)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if(itr!=m_nodeInfoTable.end())
        {
            if(defaultvalue == true)
            {
                itr->second.data_initial.clear();
                for(auto iter = data_list.begin(); iter!= data_list.end(); iter++)
                    itr->second.data_initial.push_back(*iter);
                return true;
            }
            else if (defaultvalue == false)
            {
                itr->second.data_current.clear();
                for(auto iter = data_list.begin(); iter!= data_list.end(); iter++)
                    itr->second.data_current.push_back(*iter);
                return true;
            }
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::calculate_hop_distance()
    {
        boost::NdnGlobalRouterGraph graph;
        //std::cout<<"Node list size is "<< NodeList::GetNNodes()<<std::endl;
        //std::cout<<"m_nodeInfoTable size is"<<m_nodeInfoTable.size()<<std::endl;
        map<std::string, computeNode>::iterator itr1;
        for (itr1 = m_nodeInfoTable.begin(); itr1 != m_nodeInfoTable.end(); itr1++)
        {
            //ns3::Ptr<ns3::Node> src_node = NodeList::GetNode(i);
            //std::cout<<"first node in the table is "<<itr1->first<<std::endl;
            string src_node_name =itr1->first;
            src_node_name.erase(0,1);
            ns3::Ptr<ns3::Node> src_node = Names::Find<Node>(src_node_name);
            //std::cout<<"src node is "<<src_node->GetId();
            Ptr<GlobalRouter> source = src_node->GetObject<GlobalRouter>();
            if (source == 0)
            {
                std::cout<<"Node " << src_node->GetId() << " does not export GlobalRouter interface"<<std::endl;
                continue;
            }
            //std::cout<<"Reachability from Node: " << source->GetObject<Node>()->GetId() << " ("
              //                                      << Names::FindName(source->GetObject<Node>()) << ")"<<std::endl;
            boost::DistancesMap distances;
            dijkstra_shortest_paths(graph, source,
                              // predecessor_map (boost::ref(predecessors))
                              // .
                                distance_map(boost::ref(distances))
                                .distance_inf(boost::WeightInf)
                                .distance_zero(boost::WeightZero)
                                .distance_compare(boost::WeightCompare())
                                .distance_combine(boost::WeightCombine()));

            for (const auto& dist : distances)
            {
                ns3::Ptr<ns3::Node> dest_node = NodeList::GetNode(dist.first->GetId());
                Name dest_node_name = Names::FindName(dest_node);
                Name src_node_name = Names::FindName(src_node);
                map<std::string, computeNode>::iterator itr2;
                for(itr2 = m_nodeInfoTable.begin(); itr2 != m_nodeInfoTable.end(); itr2++)
                {
                    if(dest_node_name == (src_node_name))
                    {
                        ((itr1->second).hop_distance).insert(std::make_pair(dest_node_name.toUri(), 0));
                        break;
                    }
                    else if(dest_node_name == itr2->second.nodeName)
                    {
                       ((itr1->second).hop_distance).insert(std::make_pair(dest_node_name.toUri(), std::get<1>(dist.second)));
                       break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
        map<std::string, computeNode>::iterator mapIt;
        for(mapIt= m_nodeInfoTable.begin(); mapIt != m_nodeInfoTable.end(); mapIt++)
        {
            std::set<std::pair<std::string, int>>::iterator setIt;
            //for(setIt = mapIt->second.hop_distance.begin(); setIt != mapIt->second.hop_distance.end(); setIt++)
              //  std::cout<< "The hop distance from Node "<< mapIt->second.nodeName <<" to "<<(*setIt).first << " is " << (*setIt).second<<std::endl;
        }
        return true;
    }

    int
    OrchestratorNodeInfoStorage::get_distance(std::string src_node, std::string dest_node)
    {
        map<std::string, computeNode>::iterator mapIt;
        if(src_node == dest_node)
        return 0;
        for(mapIt = m_nodeInfoTable.begin(); mapIt != m_nodeInfoTable.end(); mapIt++)
        {
            if(src_node == (*mapIt).first)
            {
                std::set<std::pair<std::string, int>>::iterator setIt;
                for(setIt = mapIt->second.hop_distance.begin(); setIt != mapIt->second.hop_distance.end(); setIt++)
                {
                    if(dest_node == setIt->first)
                    {
                        return setIt->second;
                    }
                    else
                    {
                        continue;
                    }
                }
                return 0;
            }
        }

    }


    //Function Info
    OrchestratorNodeInfoStorage::functionInfo
    OrchestratorNodeInfoStorage::getFuncInfo(std::string nodeID, uint32_t index, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if(defaultvalue==true)return ((itr->second).functions_initial).at(index);
            if(defaultvalue==false)return((itr->second).functions_current).at(index);
            }
        else
            std::cout<<"Node not found"<<std::endl;
    }

    std::pair<bool, uint32_t>
    OrchestratorNodeInfoStorage::getFuncIndex(std::string nodeID, std::string funcName, bool defaultvalue)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end()){
            if(defaultvalue==true)
            {
                for(uint32_t i=0;i<(itr->second).functions_initial.size();i++)
                {
                    if(funcName.compare(((itr->second).functions_initial.at(i)).funcName.toUri())==0)
                        return std::make_pair(true,i);
                }
                return std::make_pair(false,0);
            }
            else
            {
                for(uint32_t i=0;i<(itr->second).functions_current.size();i++)
                {
                    if(funcName.compare(((itr->second).functions_current.at(i)).funcName.toUri())==0)
                        return std::make_pair(true, i);
                }
                return std::make_pair(false, 0);
            }
        }

    }

    bool
    OrchestratorNodeInfoStorage::setFuncInfo(std::string nodeID, uint32_t index, functionInfo funcToUpdate, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if (defaultvalue==true)
          {
            ((itr->second).functions_initial).at(index).funcName = funcToUpdate.funcName;
            ((itr->second).functions_initial).at(index).func_exe_counter = funcToUpdate.func_exe_counter;
            ((itr->second).functions_initial).at(index).func_interest_counter = funcToUpdate.func_interest_counter;
            ((itr->second).functions_initial).at(index).status = funcToUpdate.status;
            ((itr->second).functions_initial).at(index).exec_time = funcToUpdate.exec_time;
            ((itr->second).functions_initial).at(index).cores = funcToUpdate.cores;
            ((itr->second).functions_initial).at(index).ram = funcToUpdate.ram;
            ((itr->second).functions_initial).at(index).rom = funcToUpdate.rom;
            ((itr->second).functions_initial).at(index).size_bytes = funcToUpdate.size_bytes;
            return true;
          }
          else
          {
            ((itr->second).functions_current).at(index).funcName = funcToUpdate.funcName;
            ((itr->second).functions_current).at(index).func_exe_counter = funcToUpdate.func_exe_counter;
            ((itr->second).functions_current).at(index).func_interest_counter = funcToUpdate.func_interest_counter;
            ((itr->second).functions_current).at(index).status = funcToUpdate.status;
            ((itr->second).functions_current).at(index).exec_time = funcToUpdate.exec_time;
            ((itr->second).functions_current).at(index).cores = funcToUpdate.cores;
            ((itr->second).functions_current).at(index).ram = funcToUpdate.ram;
            ((itr->second).functions_current).at(index).rom = funcToUpdate.rom;
            ((itr->second).functions_current).at(index).size_bytes = funcToUpdate.size_bytes;
            (((itr->second).functions_current).at(index).input_data_list).clear();
            std::vector<std::string>::iterator itr1;
            for(itr1 = funcToUpdate.input_data_list.begin(); itr1!= funcToUpdate.input_data_list.end(); itr1++)
                (((itr->second).functions_current).at(index).input_data_list).push_back(*itr1);
            return true;
          }
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    Name
    OrchestratorNodeInfoStorage::getFuncName(std::string nodeID, uint32_t index, bool defaultvalue = true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)return ((itr->second).functions_initial).at(index).funcName;
          if(defaultvalue==false)return ((itr->second).functions_current).at(index).funcName;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return 0;
        }

    }

    bool
    OrchestratorNodeInfoStorage::setFuncName(std::string nodeID, uint32_t index, Name nameToSet, bool defaultvalue=true)
    {

        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)((itr->second).functions_initial).at(index).funcName = nameToSet;
          if(defaultvalue==false)((itr->second).functions_current).at(index).funcName = nameToSet;
          return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    uint32_t
    OrchestratorNodeInfoStorage::getFuncExeCounter(std::string nodeID, uint32_t index, bool defaultvalue=true)
    {

        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)return ((itr->second).functions_initial).at(index).func_exe_counter;
          if(defaultvalue==false)return ((itr->second).functions_current).at(index).func_exe_counter;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return 0;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setFuncExeCounter(std::string nodeID, uint32_t index, uint32_t counter, bool defaultvalue=true)
    {
         map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)((itr->second).functions_initial).at(index).func_exe_counter = 0;
          if(defaultvalue==false)((itr->second).functions_current).at(index).func_exe_counter = counter;
          return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    uint32_t
    OrchestratorNodeInfoStorage::getFuncInterestCounter(std::string nodeID, uint32_t index, bool defaultvalue=true)
    {

        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)return ((itr->second).functions_initial).at(index).func_interest_counter;
          if(defaultvalue==false)return ((itr->second).functions_current).at(index).func_interest_counter;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return 0;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setFuncInterestCounter(std::string nodeID, uint32_t index, uint32_t counter, bool defaultvalue=true)
    {
         map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)((itr->second).functions_initial).at(index).func_interest_counter = 0;
          if(defaultvalue==false)((itr->second).functions_current).at(index).func_interest_counter = counter;
          return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::getFuncStatus(std::string nodeID, uint32_t index, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)return ((itr->second).functions_initial).at(index).status;
          if(defaultvalue==false)return ((itr->second).functions_current).at(index).status;
        }
    }

    uint32_t
    OrchestratorNodeInfoStorage::getFuncCpu(std::string nodeID, uint32_t index, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)return ((itr->second).functions_initial).at(index).cores;
          if(defaultvalue==false)return ((itr->second).functions_current).at(index).cores;
        }
    }

    uint32_t
    OrchestratorNodeInfoStorage::getFuncRam(std::string nodeID, uint32_t index, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)return ((itr->second).functions_initial).at(index).ram;
          if(defaultvalue==false)return ((itr->second).functions_current).at(index).ram;
        }
    }

    uint32_t
    OrchestratorNodeInfoStorage::getFuncRom(std::string nodeID, uint32_t index, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)return ((itr->second).functions_initial).at(index).rom;
          if(defaultvalue==false)return ((itr->second).functions_current).at(index).rom;
        }
    }

    uint32_t
    OrchestratorNodeInfoStorage::getFuncSize(std::string nodeID, uint32_t index, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)return ((itr->second).functions_initial).at(index).size_bytes;
          if(defaultvalue==false)return ((itr->second).functions_current).at(index).size_bytes;
        }
    }

    std::vector<std::string>
    OrchestratorNodeInfoStorage::getFuncInputs(std::string nodeID, uint32_t index, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)return ((itr->second).functions_initial).at(index).input_data_list;
          if(defaultvalue==false)return ((itr->second).functions_current).at(index).input_data_list;
        }
    }


    bool
    OrchestratorNodeInfoStorage::setFuncStatus(std::string nodeID, uint32_t index, bool status, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)((itr->second).functions_initial).at(index).status = status;
          if(defaultvalue==false)((itr->second).functions_current).at(index).status = status;
          return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setFuncCpu(std::string nodeID, uint32_t index, uint32_t cores, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)((itr->second).functions_initial).at(index).cores = cores;
          if(defaultvalue==false)((itr->second).functions_current).at(index).cores = cores;
          return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setFuncRam(std::string nodeID, uint32_t index, uint32_t ram, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)((itr->second).functions_initial).at(index).ram = ram;
          if(defaultvalue==false)((itr->second).functions_current).at(index).ram = ram;
          return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setFuncRom(std::string nodeID, uint32_t index, uint32_t rom, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)((itr->second).functions_initial).at(index).rom = rom;
          if(defaultvalue==false)((itr->second).functions_current).at(index).rom = rom;
          return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setFuncSize(std::string nodeID, uint32_t index, uint32_t size, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if (itr != m_nodeInfoTable.end())
        {
          if(defaultvalue==true)((itr->second).functions_initial).at(index).size_bytes = size;
          if(defaultvalue==false)((itr->second).functions_current).at(index).size_bytes = size;
          return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }

    bool
    OrchestratorNodeInfoStorage::setFuncInputs(std::string nodeID, uint32_t index, std::vector<std::string>inputs, bool defaultvalue=true)
    {
        map<std::string, computeNode>::iterator itr;
        itr = m_nodeInfoTable.find(nodeID);
        if(itr!=m_nodeInfoTable.end())
        {
            for(auto iter = inputs.begin(); iter != inputs.end(); iter++)
            {
                if(defaultvalue==true)((itr->second).functions_initial).at(index).input_data_list.push_back(*iter);
                if(defaultvalue==false)((itr->second).functions_current).at(index).input_data_list.push_back(*iter);
            }
            return true;
        }
        else{
            std::cout<<"Node not found"<<std::endl;
            return false;
        }
    }
}}}
