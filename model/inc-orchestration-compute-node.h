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

#ifndef INC_ORCHESTRATION_COMPUTE_NODE_H
#define INC_ORCHESTRATION_COMPUTE_NODE_H

#include "ns3/node.h"
#include <unordered_map>
#include "ns3/INC-Computation.hpp"


namespace ns3{
    namespace ndn{
        namespace inc{
    /**
     *
     * \brief A class that couples a ns3::Node with its extra information,
     * you can think of it as a container for a Node which also carries the information related to the Node
     *
     */
    class IncOrchestrationComputeNode: public Object{
        public:

           static TypeId GetTypeId (void);

           uint32_t GetUUID();
           void SetUUID(uint32_t value);

           uint32_t GetProcessorCore();
           void SetProcessorCore(uint32_t value);

           uint32_t GetProcessorClockSpeed();
           void SetProcessorClockSpeed(uint32_t value);

           uint32_t GetRam();
           void SetRam(uint32_t value);

           uint32_t GetRom();
           void SetRom(uint32_t value);

           uint32_t GetQueueSize();
           void SetQueueSize(uint32_t value);
           bool IncrementQueueFill();
           bool DecrementQueueFill();
           bool IsQueueFull();
           bool IsQueueEmpty();

           std::string GetLinks();
           void SetLinks(std::string value);

           std::string GetProvidedData();
           void AddProvidedData(std::string value);
           bool CheckProvidedDataList(std::string value);

           std::string GetName();
           void SetName(std::string value);

           std::string GetSupportedRuntimes();
           void SetSupportedRuntimes(std::string value);

           Ptr<Node> GetNode();
           void SetNode(Ptr<Node> value);

           //function management
           virtual void
           SetFunctionMap(std::unordered_map<std::string,Ptr<INC_Computation>> map);

           virtual std::unordered_map<std::string,Ptr<INC_Computation>>
           GetFunctionMap(void);

           virtual void
           AddNewFunction(std::string name, Ptr<INC_Computation> function);

           virtual Ptr<INC_Computation>
           GetFunction(std::string name);

           virtual bool
           EnableFunction(std::string func_name);

           virtual bool
           EnableDataForCompute(std::string data_name);

           virtual bool
           DisableFunction(std::string function_name);

           void
           AddToExcludeList(std::string function_name);

           void
           RemoveFromExcludeList(std::string function_name);

           bool
           CheckExcludeList(std::string function_name);


           IncOrchestrationComputeNode();
           IncOrchestrationComputeNode(uint32_t systemId);
           virtual ~IncOrchestrationComputeNode();

           void
           AddNodeBusyCounter();

           uint32_t
           GetNodeBusyCounter();

           void
           ResetNodeBusyCounter();

           void
		   notifyFibFunctionEnable(std::string func_name);

		   void
		   notifyFibFunctionDisable(std::string func_name);

           void
		   notifyFibDataEnable(std::string data_name);

		   void
		   notifyFibDataDisable(std::string data_name);

		  shared_ptr<Face>
		  GetProducerAppFace();

		  void
		   SetProducerAppFace(shared_ptr<Face> face);

		  std::string
		  GetIncStrategy();

		  void
		  SetIncStragety(std::string inc_strategy);

          typedef void (*EmptyQueueCallback)(double emptyTime);
          typedef void (*FullQueueCallback)(double fullTime);

        private:

           //void InitialFunctionRegister();

            Ptr<Node> m_node;
            uint32_t m_uuid;
            uint32_t m_processor_core;
            uint32_t m_processor_speed;
            uint32_t m_ram;
            uint32_t m_rom;
            uint32_t m_nodeBusyCounter;
            uint32_t m_queue_size;
            uint32_t m_queue_fill;
            std::string m_links;
            std::string m_supported_runtimes;
            std::string m_name;
            std::vector<std::string> m_FuncExcludeList;
            std::string m_inc_strategy;
            std::string m_provided_data;
            double m_q_empty_time;
            bool m_empty_timer_started;
            bool m_full_timer_started;
            double m_q_full_time;
            //function map used for function management
            std::unordered_map<std::string,Ptr<INC_Computation>> m_func_map;

            //the app face used for NFD manipulation
            shared_ptr<Face> m_producer_app_face;

            TracedCallback<double> m_emptyQueueTimer;
            TracedCallback<double> m_fullQueueTimer;
    };
        }
    }
}
#endif
