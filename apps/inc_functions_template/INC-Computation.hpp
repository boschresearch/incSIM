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

#ifndef INC_COMPUTATION_H
#define  INC_COMPUTATION_H

#include "ns3/names.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/nstime.h"

/*
The base computation application.
All computation application derive from this base class.
It consists of methods to access the computation properties as well as a switch to enable or disable them.
A INC-Computation also contains a set of properties such as ram, rom, cup
*/

namespace ns3{
namespace ndn{
namespace inc{
    class INC_Computation: public Object{
        private:
            Name m_computationName;
            bool m_enableComputation;

        protected:
            //function properties
            uint32_t m_ram;
            uint32_t m_rom;
            uint32_t m_cpu;
            uint32_t m_exec_counter; //execution counter used for orchestration
            uint32_t m_miss_exec_counter; //counter for missed executions
            uint32_t m_node_busy_counter; //counter for node busy execution
            uint32_t m_param_number; //parameter number required by funcion
            uint32_t m_func_size;
            uint32_t m_result_size;
            uint32_t m_num_instructions;
            double m_exec_time;// dummy time used for simulation
            std::vector<std::string> m_runtime_requirements;
            std::string m_input_list;

        public:


            //constructor, deconstructor
            INC_Computation(uint32_t ram, uint32_t rom, uint32_t cpu);
            INC_Computation();
            virtual ~INC_Computation();

            //execution call with different return types
			virtual void Execute();
			virtual int ExecuteIntResult();

            // getters and setters:

            //name
            virtual Name getName();
            virtual void setName(Name);

            //enable status
            virtual bool GetEnableStatus();
            virtual void Enable();
            virtual void Disable();


            //execution counter
            virtual void IncreaseCounter();
            virtual uint32_t GetCounter();

            //execution counter
			virtual void AddMissExecCounter();
			virtual void ResetMissExecCounter();
			virtual uint32_t GetMissExecCounter();

            //Result size
            void SetResultSize(uint32_t value);
            uint32_t GetResultSize();

            //ram consumption
            virtual uint32_t GetRam();
            virtual void SetRam(u_int32_t value);

            //rom consumption
            virtual uint32_t GetRom();
            virtual void SetRom(u_int32_t value);

            //cpu consumption
            virtual uint32_t GetCpu();
            virtual void SetCpu(u_int32_t value);

            //execution time estimation
            virtual double GetExecTime();
            virtual void SetExecTime(double value);

            //number of instructions
            virtual uint32_t GetNumInstructions();
            virtual void SetNumInstructions(uint32_t value);

            //function sze in bytes
            virtual uint32_t GetFuncSize();
            virtual void SetFuncSize(uint32_t value);

            virtual void SetRuntimeEnvironment(std::vector<std::string> value);
            virtual std::vector<std::string> GetRuntimeEnvironment();

            virtual void SetParamNumber(u_int32_t value);
			virtual uint32_t GetParamNumber();

			virtual void SetInputList(std::string value);
            virtual void AddToInputList(std::string value);
			virtual std::string GetInputList();

    };
}}}
#endif
