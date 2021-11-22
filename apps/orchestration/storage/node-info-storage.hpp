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

#ifndef INC_NODE_INFO_STORAGE_H
#define INC_NODE_INFO_STORAGE_H

#include "ndn-cxx/name.hpp"
#include "ns3/ndnSIM-module.h"
#include "ns3/inc-orchestration-compute-node.h"
#include "ns3/nstime.h"
#include <map>
#include <vector>
#include <string>

/*
The node-info-storage stores the information pertaining to the nodes in the network topology.
This information is obtained from the nodes via the APIs that communication application uses to update the information.
It also provides handles/APIs to access complete and specific information from individual/all nodes in the network
for the OAM application to make informed decisions.
*/

namespace ns3{
namespace ndn{
namespace inc{
    class OrchestratorNodeInfoStorage: public Object{
        public:
            //function properties
            struct functionInfo{
                Name funcName;
                double exec_time;
                uint32_t func_exe_counter;
                uint32_t func_interest_counter;
                uint32_t cores;
                uint32_t ram;
                uint32_t rom;
                uint32_t size_bytes;
                std::vector<std::string> input_data_list;
                bool status;
            };

            struct dataInfo{
                Name dataName;
                uint32_t size_bytes;
            };

            struct computeNode{
                Name nodeName;
                Ptr<ns3::ndn::inc::IncOrchestrationComputeNode> node_ptr;
                std::pair<uint32_t, uint32_t> ram;
                std::pair<uint32_t, uint32_t> rom;
                std::pair<uint32_t, uint32_t> cpu;
                std::pair<uint32_t, uint32_t> nodeBusyCounter;
                std::pair<std::vector<std::string>, std::vector<std::string>> links;
                std::pair<std::vector<std::string>, std::vector<std::string>> runtimes_supported;
                std::vector<ns3::ndn::inc::OrchestratorNodeInfoStorage::dataInfo> data_initial;
                std::vector<ns3::ndn::inc::OrchestratorNodeInfoStorage::dataInfo> data_current;
                std::vector<ns3::ndn::inc::OrchestratorNodeInfoStorage::functionInfo> functions_initial;
                std::vector<ns3::ndn::inc::OrchestratorNodeInfoStorage::functionInfo> functions_current;
                std::set<std::pair<std::string, int>> hop_distance;
            };


        private:
            static
            std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode> m_nodeInfoTable;
        public:
            std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_iterator cbegin();
            std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_iterator cend();
            std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_reverse_iterator crbegin();
            std::map<std::string, ns3::ndn::inc::OrchestratorNodeInfoStorage::computeNode>::const_reverse_iterator crend();
            static TypeId GetTypeId (void);

            //constructor, deconstructor
            OrchestratorNodeInfoStorage();
            ~OrchestratorNodeInfoStorage();


            // getters and setters:

            //Node info
            computeNode getNodeInfo(std::string);
            void printCurrentNodeInfo(std::string);
            void printDefaultNodeInfo(std::string);
            uint32_t getTableSize();
            bool AddNodeToTable(std::string,computeNode);
            Name getNodeName(std::string nodeID);
            bool setNodeName(std::string nodeID,Name);
            Ptr<ns3::ndn::inc::IncOrchestrationComputeNode> getNodePointer(std::string nodeID);
            bool setNodePointer(std::string nodeID, Ptr<ns3::ndn::inc::IncOrchestrationComputeNode> value);

            uint32_t getNodeRAM(std::string, bool);
            bool setNodeRAM(std::string, uint32_t, bool);

            uint32_t getNodeROM(std::string, bool);
            bool setNodeROM(std::string, uint32_t, bool);

            uint32_t getNodeBusyCounter(std::string, bool);
            bool setNodeBusyCounter(std::string, uint32_t, bool);

            uint32_t getNodeCPU(std::string, bool);
            bool setNodeCPU(std::string, uint32_t, bool);

            std::vector<std::string> getNodeLinks(std::string, bool);
            bool setNodeLinks(std::string,std::vector<std::string>, bool);
            std::vector<dataInfo> getNodeDataList(std::string, bool);
            bool setNodeDataList(std::string,std::vector<dataInfo>, bool);

            bool addLinkToNode(std::string,string, bool);

            std::vector<std::string> getNodeRuntimes(std::string, bool);
            bool setNodeRuntimes(std::string,std::vector<std::string>, bool);
            bool addRuntimeToNode(std::string,string, bool);

            std::vector<functionInfo> getFunctionsAtNode(std::string nodeID, bool);
            bool addFunctionToNode(std::string nodeID, functionInfo funcToAdd, bool);

            bool
            addDataToNode(std::string nodeID, dataInfo dataToAdd, bool);
            std::vector<OrchestratorNodeInfoStorage::dataInfo>
            getDataAtNode(std::string nodeID, bool defaultvalue);

            bool
            calculate_hop_distance();

            int
            get_distance(std::string src_node, std::string dest_node);

            //int
            //get_hop_distance(string source_node, string destination_node);

            //Function Info
            functionInfo getFuncInfo(std::string nodeID, uint32_t index, bool);
            bool setFuncInfo(std::string nodeID, uint32_t index, functionInfo funcToUpdate, bool);
            std::pair<bool, uint32_t> getFuncIndex(std::string nodeID, std::string funcName, bool);
            Name getFuncName(std::string nodeID, uint32_t index, bool);
            bool setFuncName(std::string nodeID, uint32_t index, Name nameToSet, bool);
            uint32_t getFuncExeCounter(std::string nodeID, uint32_t index, bool);
            bool setFuncExeCounter(std::string nodeID, uint32_t index, uint32_t counter, bool);
            uint32_t getFuncInterestCounter(std::string nodeID, uint32_t index, bool);
            bool setFuncInterestCounter(std::string nodeID, uint32_t index, uint32_t counter, bool);
            bool getFuncStatus(std::string nodeID, uint32_t index, bool);
            bool setFuncStatus(std::string nodeID, uint32_t index, bool status, bool);
            uint32_t getFuncCpu(std::string nodeID, uint32_t index, bool);
            bool setFuncCpu(std::string nodeID, uint32_t index, uint32_t cores, bool);
            bool setFuncRam(std::string nodeID, uint32_t index, uint32_t ram, bool);
            uint32_t getFuncRam(std::string nodeID, uint32_t index, bool);
            bool setFuncRom(std::string nodeID, uint32_t index, uint32_t rom, bool);
            uint32_t getFuncRom(std::string nodeID, uint32_t index, bool);
            bool setFuncSize(std::string nodeID, uint32_t index, uint32_t size, bool);
            uint32_t getFuncSize(std::string nodeID, uint32_t index, bool);
            std::vector<std::string> getFuncInputs(std::string nodeID, uint32_t index, bool);
            bool setFuncInputs(std::string nodeID, uint32_t index, std::vector<std::string> inputs, bool);
    };
}}}
#endif
