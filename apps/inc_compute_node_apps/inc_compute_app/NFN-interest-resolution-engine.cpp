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
 *			Liming Liu <fixed-term.liming.liu@de.bosch.com>
 * *****************************************************************************
 */

#include "NFN-interest-resolution-engine.hpp"
#include <ns3/log.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


NS_LOG_COMPONENT_DEFINE("NFN_InterestResolutionEngine");

namespace ns3{
namespace ndn{

    //A simple handler used when a function argument hits content store
	uint32_t data_miss_counter;
	void HitHandler(shared_ptr<Interest> interestArg){
		NS_LOG_INFO("[NFN Resolution Engine] Content store hit for argument:"<<interestArg->getName().toUri()<<std::endl);
	}

	//A simple handler used when a function argument misses content store
	void MissHandler(shared_ptr<Interest> interestArg, bool* flag_pointer ){
		NS_LOG_INFO("[NFN Resolution Engine] Content store miss for argument:"<<interestArg->getName().toUri()<<std::endl);
		*flag_pointer=false;
	}

	//A simple handler for doing nothing
	void voidHandler(){
		//do nothing
	}

	//A simple handler that adds a name to fetch list
	void
	AddToDecisionHandler(shared_ptr<Name> arg, shared_ptr<vector<Name>> ptr_fetch_decisions){
		NS_LOG_INFO("[NFN Resolution Engine] Add argument data to fetch decisions:"<<arg->toUri()<<std::endl);
		ptr_fetch_decisions->push_back(*arg);
		data_miss_counter++;
	}

	//return the function name of the interest
	//eg: /lambda/Function/func-one/Data/data1  will return Name("/func-one")
	shared_ptr<Name>
	NfnInterestResolutionEngine::ExtractFunctionName(shared_ptr<const Interest> interest){
		Name interest_name=interest->getName();
		Name func_prefix = Name("/Function");
		Name funcName;
		for(u_int32_t i=0;i<interest_name.size();i++){
			if(interest_name.getSubName(i, 1)==func_prefix){
				if(i<interest_name.size()-1){
					NS_LOG_INFO("[NFN Resolution Engine] Extract function name from: "<<interest_name.toUri()<< " as "<<interest_name.getSubName(i+1, 1).toUri());
					return make_shared<Name>(interest_name.getSubName(i+1, 1));
				}
			}
		}
		NS_LOG_INFO("[NFN Resolution Engine] Illegal interest: "<<interest_name.toUri()<<". No /Function name found");
		return NULL;
	}

	//return the data names of the interest
	//eg: /lambda/Function/func-one/Data/data1/data2  will return Name("/data1"),Name("/data2")
    std::vector<Name>
    NfnInterestResolutionEngine::ExtractDataNames(shared_ptr<const Interest> interest){
    	Name interest_name=interest->getName();
		vector<Name> data_names;
		bool isData=false;
		for(u_int32_t i=0;i<interest_name.size();i++){
			if(interest_name.getSubName(i, 1)==Name("/lambda")){
				continue; //skip
			}
			if(interest_name.getSubName(i, 1)==Name("/Data")){
				isData=true;
				continue;
			}
			if(interest_name.getSubName(i, 1)==Name("/Function")){
				isData=false; //enter function sub-expression
				continue;
			}
			if(isData){
				Name t_name=interest_name.getSubName(i, 1);

				/**
				 * !!Bug warning: unsolved runtime misbehavior
				 * The line underneath can recognize names like /%FE%01 but NOT /%FE-,/%FE., etc, which can appear in high-frequency cases
				 * the reason is still unclear.
				 */

				if(t_name.toUri().find("/%FE")!=string::npos){
					return data_names;
				}else{
					NS_LOG_INFO("[NFN Resolution Engine] Extract data name from: "<<interest_name.toUri()<< ". Add: "<<t_name.toUri());
					data_names.push_back(t_name); // add data name to list
				}
			}
		}
		return data_names;
    }

    std::pair<ns3::ndn::NfnInterestResolutionEngine::NRE_Decision,shared_ptr<vector<Name>>>
	NfnInterestResolutionEngine::GetFetchDecisions(shared_ptr<const Interest> interest, Ptr<ndn::inc::IncOrchestrationComputeNode> computeNode){
		data_miss_counter = 0;
		vector<Name> fetch_decisions;
		shared_ptr<vector<Name>> fetch_decisions_ptr=make_shared<vector<Name>>(fetch_decisions);
		std::pair<NRE_Decision,shared_ptr<vector<Name>>> result;

		Name interest_name=interest->getName();

		//basic check, interest should have at least 2 names
		if(interest_name.size()<2){
			NS_LOG_INFO("[NFN Resolution Engine-"<<computeNode->GetName()<<"] Illegal interest: "<<interest_name.toUri());
			result.first = NACK;
			result.second = fetch_decisions_ptr;
			return result;
		}

		//parse the interest, extract function name and parameter list
		shared_ptr<Name> func_name=ExtractFunctionName(interest);
		std::vector<Name> parameter_list=ExtractDataNames(interest);

		//verify function exists
		auto func=computeNode->GetFunction(func_name->toUri());
		if(!func){
			NS_LOG_INFO("[NFN Resolution Engine-"<<computeNode->GetName()<<"] Function does not exist "<<func_name->toUri()<< " in interest "<<interest_name.toUri());
			result.first=NACK;
			result.second = fetch_decisions_ptr;
			return result;
		}
		//check function enable status
		bool func_flag=func->GetEnableStatus();
		if(!func_flag){
			//not enabled locally, add /Function/(func_name) to fetch list
			Name fetch_func_name=Name("/Function").append(*func_name);
			fetch_decisions_ptr->push_back(fetch_func_name);
			NS_LOG_INFO("[NFN Resolution Engine-"<<computeNode->GetName()<<"] Add to fetch decisions "<<fetch_func_name.toUri());
		}


		//verify the number of input parameters is correct
		if(parameter_list.size()!=func->GetParamNumber()){

			NS_LOG_INFO("[NFN Resolution Engine-"<<computeNode->GetName()<<"] Illegal interest: " << interest->toUri()<<
					". Consumer should provide "<<func->GetParamNumber()<<
					" input parameters, but there are "<<parameter_list.size()<<
					" provided"<<std::endl);
			result.first=NACK;
			result.second = fetch_decisions_ptr;
			return result;
		}

		//check content store, search local cache of data arguments
		Ptr<L3Protocol> L3protocol = computeNode->GetNode()->GetObject<L3Protocol>();
		shared_ptr<nfd::Forwarder> forwarder = L3protocol->getForwarder();
		if(DataCacheAvailable==true)
		{
			for(auto t_param:parameter_list){
		   		shared_ptr<Name> argInterest = make_shared<Name>(Name("/Data").append(t_param));
		   		shared_ptr<Interest> interestArg = make_shared<Interest>(*argInterest);
				interestArg->setMustBeFresh(true);
		   		//lookup
		   		NS_LOG_INFO("[NFN Resolution Engine-"<<computeNode->GetName()<<"]: lookup local cs for data "<<argInterest->toUri());
		   		forwarder->getCs().find(*interestArg,
						   bind(&voidHandler), //if content store hit, do nothing
				   		   bind(&AddToDecisionHandler, argInterest, fetch_decisions_ptr));//if content store miss, add the name to fetch lsit
			}
		}
		else
		{
			NS_LOG_INFO("[ "<<computeNode->GetName()<<"]Inside Data store");
			for(auto t_param:parameter_list){
				if(computeNode->CheckProvidedDataList(t_param.toUri())==false)
				{
					shared_ptr<Name> argInterest = make_shared<Name>(Name("/Data").append(t_param));
		   			shared_ptr<Interest> interestArg = make_shared<Interest>(*argInterest);
		   			interestArg->setName(*argInterest);
					AddToDecisionHandler(argInterest, fetch_decisions_ptr);
				}
				else{
					//do nothing
				}
			}
		}
		NS_LOG_INFO("The data missing counter is : "<<data_miss_counter<<std::endl);
		NS_LOG_INFO("The number of input parameters are :"<<parameter_list.size()<<std::endl);

		bool check_result = computeNode->CheckExcludeList(func_name->toUri());
		if(((func_flag == false)&&(data_miss_counter == parameter_list.size())) || (check_result))
		{
			//Neither function nor data is available
			func->AddMissExecCounter();
			result.first=FORWARD;
			result.second = fetch_decisions_ptr;
			return result;
		}

		else if((func_flag == true) && (data_miss_counter == 0))
		{
			result.first=EXECUTE;
			result.second = fetch_decisions_ptr;
			return result;
		}
		result.first=FETCH;
		result.second = fetch_decisions_ptr;
		return result;
	}

  } // namespace ndn
} // namespace ns3
