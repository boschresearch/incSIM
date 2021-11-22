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

#include "message_handler.hpp"
#include "ns3/core-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/node-info-storage.hpp"
#include "ns3/string.h"
#include "ns3/incSIM-module.h"
#include <vector>
#include <boost/algorithm/string.hpp>


NS_LOG_COMPONENT_DEFINE ("OrchestrationMessageHandler");
namespace ns3{
namespace ndn{
namespace inc{

	NS_OBJECT_ENSURE_REGISTERED(OrchestrationMessageHandler);

	TypeId
    OrchestrationMessageHandler::GetTypeId (void)
    {
      static TypeId tid = TypeId ("ns3::ndn::inc::OrchestrationMessageHandler")
        .SetParent<Object> ()
        .AddConstructor<OrchestrationMessageHandler>();

      return tid;
    }


	OrchestrationMessageHandler::OrchestrationMessageHandler(){
			//do nothing
	}

	OrchestrationMessageHandler::~OrchestrationMessageHandler(){
			//do nothing
	}

	void
	OrchestrationMessageHandler::HandleMessage(std::string actionToDo, std::string content)
	{
		if(actionToDo.compare("/BootstrapInfo")==0)
		{
			BootstrapPhaseHandler(content);
		}
		else if (actionToDo.compare("/NodeStatusFetch")==0)
		{
			NodeStatusFetchHandler(content);
		}
		else
		{
			//do nothing
		}
	}

	void
	OrchestrationMessageHandler::BootstrapPhaseHandler(std::string content)
	{
		OrchestratorNodeInfoStorage::computeNode newNode;
		std::vector<std::string> tokens;
		boost::split(tokens, content, boost::is_any_of(";"));
			//----------------Parsing the content-------------------//
			for(uint32_t i=0; i< tokens.size();i++)
			{
				std::vector<std::string> values;
				boost::split(values,tokens[i],boost::is_any_of("="));
					if((values[0]).compare("name")==0)
					{
						newNode.nodeName = Name(values[1]);
					}
					else if((values[0]).compare("processor_core")==0)
					{
						(newNode.cpu).first = std::stoi(values[1]);
						(newNode.cpu).second = std::stoi(values[1]);
					}
					else if((values[0]).compare("RAM")==0)
					{
						(newNode.ram).first = std::stoi(values[1]);
						(newNode.ram).second = std::stoi(values[1]);
					}
					else if((values[0]).compare("ROM")==0)
					{
						(newNode.rom).first = std::stoi(values[1]);
						(newNode.rom).second = std::stoi(values[1]);
					}
					else if((values[0]).compare("links")==0)
					{
						std::vector<std::string> links;
						boost::split(links, values[1], boost::is_any_of(","));
						(newNode.links).first = links;
						(newNode.links).second = links;
					}
					else if((values[0]).compare("runtimes")==0)
					{
						std::vector<std::string> runtimes;
						boost::split(runtimes, values[1], boost::is_any_of(","));
						(newNode.runtimes_supported).first = runtimes;
						(newNode.runtimes_supported).second = runtimes;
					}
					else if((values[0]).compare("node_busy_counter")==0)
					{
						(newNode.nodeBusyCounter).first = 0;
						(newNode.nodeBusyCounter).second = 0;
					}
					else if((values[0]).compare("data")==0)
					{
						std::vector<std::string> data_list;
						if(!(values[1].compare("null")==0))
						{
							boost::split(data_list,values[1], boost::is_any_of(","));
							for(auto iter = data_list.begin(); iter != data_list.end(); iter++)
							{
								std::vector<std::string>data_each;
								boost::split(data_each, *iter, boost::is_any_of(":"));
								OrchestratorNodeInfoStorage::dataInfo new_data;
								new_data.dataName = data_each.at(0);
								new_data.size_bytes = std::stoi(data_each.at(1));
								newNode.data_initial.push_back(new_data);
								newNode.data_current.push_back(new_data);
							}
						}
						else
						{
							//do nothing
						}
					}
					else if((values[0]).compare("functions")==0)
					{
						std::vector<std::string> functions_all;
						boost::split(functions_all, values[1], boost::is_any_of("/")); //splitting list of functions to individual functions
						auto iter=functions_all.begin();
						for (std::advance(iter, 1); iter != functions_all.end(); iter++)
							{
								OrchestratorNodeInfoStorage::functionInfo func;
								std::vector<std::string> functions_each;
								boost::split(functions_each, *iter, boost::is_any_of(":")); //spliting individual functions to name and status
								functions_each[0].insert(0,1,'/');
								func.funcName = functions_each[0];
								func.status = (functions_each[1]=="1")?true:false;
								func.func_exe_counter = std::stoi(functions_each[2]);
								func.func_interest_counter = std::stoi(functions_each[3]);
								func.cores = std::stoi(functions_each[4]);
								func.ram = std::stoi(functions_each[5]);
								func.rom = std::stoi(functions_each[6]);
								func.size_bytes = std::stoi(functions_each[7]);
								std::vector<std::string> inputs;
								if(!(functions_each[8].compare("null")==0))
								{
									boost::split(inputs, functions_each[8], boost::is_any_of(","));
									for(auto it = inputs.begin(); it!= inputs.end(); it++)
										func.input_data_list.push_back(*it);
								}
								newNode.functions_initial.push_back(func);
								newNode.functions_current.push_back(func);
							}
					}
			}

				std::string UUID = newNode.nodeName.toUri();
				m_storage_handler.AddNodeToTable(UUID, newNode);
				//m_storage_handler.printDefaultNodeInfo(UUID);
	}

	void
	OrchestrationMessageHandler::NodeStatusFetchHandler(std::string content)
	{
		std::string UUID;
		std::vector<std::string> tokens;
			boost::split(tokens, content, boost::is_any_of(";"));
			//----------------Parsing the content-------------------//
			for(uint32_t i=0; i< tokens.size();i++)
			{
				std::vector<std::string> values;
				boost::split(values,tokens[i],boost::is_any_of("="));
					if((values[0]).compare("name")==0)
					{
						UUID = values[1];
					}
					else if((values[0]).compare("processor_core")==0)
					{
						m_storage_handler.setNodeCPU(UUID,std::stoi(values[1]),false);
					}
					else if((values[0]).compare("RAM")==0)
					{
						m_storage_handler.setNodeRAM(UUID, std::stoi(values[1]), false);
					}
					else if((values[0]).compare("ROM")==0)
					{
						m_storage_handler.setNodeROM(UUID, std::stoi(values[1]), false);
					}
					else if((values[0]).compare("links")==0)
					{
						std::vector<std::string> links;
						boost::split(links, values[1], boost::is_any_of(","));
						m_storage_handler.setNodeLinks(UUID, links, false);
					}
					else if((values[0]).compare("runtimes")==0)
					{
						std::vector<std::string> runtimes;
						boost::split(runtimes, values[1], boost::is_any_of(","));
						m_storage_handler.setNodeRuntimes(UUID, runtimes, false);
					}
					else if((values[0]).compare("node_busy_counter")==0)
					{
						m_storage_handler.setNodeBusyCounter(UUID, std::stoi(values[1]),false);
					}
					else if ((values[0].compare("data")==0))
					{
						if(values[1].compare("null")==0)
						{
							//do nothing
						}
						else
						{
							std::vector<OrchestratorNodeInfoStorage::dataInfo> data_list;
							std::vector<std::string> data;
							boost::split(data, values[1], boost::is_any_of(","));
							for(auto iter = data.begin(); iter != data.end(); iter++)
							{
								std::vector<std::string> each_data;
								boost::split(each_data, *iter, boost::is_any_of(":"));
								OrchestratorNodeInfoStorage::dataInfo new_data;
								new_data.dataName = each_data.at(0);
								new_data.size_bytes = std::stoi(each_data.at(1));
								data_list.push_back(new_data);
							}
							m_storage_handler.setNodeDataList(UUID,data_list, false);
						}
					}
					else if((values[0]).compare("functions")==0)
					{
						std::vector<std::string> functions_all;
						boost::split(functions_all, values[1], boost::is_any_of("/")); //splitting list of functions to individual functions
						auto iter=functions_all.begin();
						for (std::advance(iter, 1); iter != functions_all.end(); iter++)
							{
								OrchestratorNodeInfoStorage::functionInfo func;
								std::vector<std::string> functions_each;
								boost::split(functions_each, *iter, boost::is_any_of(":")); //spliting individual functions to name and status
								functions_each[0].insert(0,1,'/');
								std::pair<bool,uint32_t> index = m_storage_handler.getFuncIndex(UUID,functions_each[0],false);
								if(index.first==true)
								{
									m_storage_handler.setFuncName(UUID,index.second, functions_each[0], false);
									bool status = (functions_each[1]=="1")?true:false;
									m_storage_handler.setFuncStatus(UUID,index.second,status,false);
									m_storage_handler.setFuncExeCounter(UUID, index.second, std::stoi(functions_each[2]), false);
									m_storage_handler.setFuncInterestCounter(UUID, index.second, std::stoi(functions_each[3]), false);
									m_storage_handler.setFuncCpu(UUID, index.second, std::stoi(functions_each[4]),false);
									m_storage_handler.setFuncRam(UUID, index.second, std::stoi(functions_each[5]),false);
									m_storage_handler.setFuncRom(UUID, index.second, std::stoi(functions_each[6]), false);
									m_storage_handler.setFuncSize(UUID, index.second, std::stoi(functions_each[7]), false);
									std::vector<std::string> inputs;
									boost::split(inputs, functions_each[8], boost::is_any_of(","));
									m_storage_handler.setFuncInputs(UUID, index.second, inputs, false);
								}
							}
					}

			}
		//m_storage_handler.printCurrentNodeInfo(UUID);

	}


}
}
}
