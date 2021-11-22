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

#include "INC-Computation.hpp"

namespace ns3{
namespace ndn{
namespace inc{

    //Constructor
    INC_Computation::INC_Computation()
    {
    	INC_Computation::setName("/func");
        m_enableComputation=false;
        m_exec_counter=0;
        m_exec_time=0.0;
        m_miss_exec_counter=0;
        m_func_size = 0;
        m_input_list = "null";
        //Do Nothing
    }

    //Constructor
    INC_Computation::INC_Computation(uint32_t ram, uint32_t rom, uint32_t cpu)
    {
    	INC_Computation::setName("/func");
    	m_enableComputation=false;
    	m_exec_counter=0;
    	m_exec_time=0.0;
    	m_miss_exec_counter=0;
        m_ram=ram;
        m_rom=rom;
        m_cpu=cpu;
        m_func_size = 0;
        m_input_list="null";
    }

    void
    INC_Computation::Execute(){
    	this->IncreaseCounter();
    }

    int
    INC_Computation::ExecuteIntResult(){
    	this->IncreaseCounter();
        return 0;
    }

    //Set the name of the computation application.
    void
    INC_Computation::setName(Name name)
    {
        m_computationName = name;

    }

    //Get the name of the computation application.
    Name
    INC_Computation::getName()
    {
        return m_computationName;
    }

    //Enable the computation application.
    void
    INC_Computation::Enable()
    {
    	ResetMissExecCounter();//reset missed execution counter
        m_enableComputation = true;

    }

    //Disable the computation application.
    void
    INC_Computation::Disable()
    {
        m_enableComputation= false;
    }

    //Get the current enable status of the computation application.
    bool
    INC_Computation::GetEnableStatus()
    {
        return m_enableComputation;
    }

    //Destructor
    INC_Computation::~INC_Computation(){
        //Do nothing
    }

    //the following are getters and setters:

    uint32_t INC_Computation::GetRam(){
        return this->m_ram;
    }

    void INC_Computation::SetRam(uint32_t value){
        this->m_ram=value;
    }

    uint32_t INC_Computation::GetRom(){
        return this->m_rom;
    }

    void  INC_Computation::SetRom(uint32_t value){
        this->m_rom=value;
    }

    uint32_t INC_Computation:: GetCpu(){
        return this->m_cpu;
    }

    void  INC_Computation::SetCpu(uint32_t value){
        this->m_cpu=value;
    }

    void INC_Computation::SetFuncSize(uint32_t value){
        this->m_func_size = value;
    }

    uint32_t INC_Computation::GetFuncSize(){
        return this->m_func_size;
    }

    void INC_Computation::IncreaseCounter(){
    	this->m_exec_counter++;
    }

    uint32_t INC_Computation::GetCounter(){
    	return this->m_exec_counter;
    }

    double INC_Computation::GetExecTime(){
    	return this->m_exec_time;
    }

    void INC_Computation::SetExecTime(double value){
    	this->m_exec_time=value;
    }

    uint32_t INC_Computation::GetNumInstructions(){
    	return this->m_num_instructions;
    }

    void INC_Computation::SetNumInstructions(uint32_t value){
    	this->m_num_instructions=value;
    }

    void INC_Computation::SetResultSize(uint32_t value){
        this->m_result_size = value;
    }

    uint32_t INC_Computation::GetResultSize(){
        return this->m_result_size;
    }

    void INC_Computation::SetRuntimeEnvironment(std::vector<std::string> value){
    	this->m_runtime_requirements=value;
    }

    std::vector<std::string> INC_Computation::GetRuntimeEnvironment(){
    	return this->m_runtime_requirements;
    }

    void INC_Computation::SetParamNumber(uint32_t value){
    	this->m_param_number=value;
    }

    uint32_t INC_Computation::GetParamNumber(){
    	return this->m_param_number;
    }

   void INC_Computation::AddMissExecCounter(){
	    if(!m_enableComputation){
	    	this->m_miss_exec_counter = this->m_miss_exec_counter+1;
	    }
   }

	void INC_Computation::ResetMissExecCounter(){
		this->m_miss_exec_counter=0;
	}

	uint32_t INC_Computation::GetMissExecCounter(){
		return this->m_miss_exec_counter;
	}

	void INC_Computation::SetInputList(std::string value){
        this->m_input_list.clear();
		this->m_input_list=value;
	}
	std::string INC_Computation::GetInputList(){
		return this->m_input_list;
	}

    void INC_Computation::AddToInputList(std::string value){
        if(!(this->m_input_list.compare("null")==0))
        {
            std::size_t found = this->m_input_list.find(value);
            if(found == this->m_input_list.npos)
            {
                this->m_input_list.append(","+value);
            }
        }
        else
        {
            this->m_input_list.clear();
            this->m_input_list.append(value);
        }
    }


}}}
