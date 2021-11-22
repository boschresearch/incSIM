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

#ifndef MESSAGE_HANDLER_H_
#define MESSAGE_HANDLER_H_
#include "ns3/core-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/node-info-storage.hpp"
#include "ndn-cxx/name.hpp"

namespace ns3{
namespace ndn{
namespace inc{
	/**
	 * A helper class assists ndn/udp orchestrators on decision making.
	 * Helps decoupling orchestration logic from communication tasks.
	 */
    class OrchestrationMessageHandler:public Object
    {

		 public:
		 OrchestrationMessageHandler();
		 ~OrchestrationMessageHandler();
		 static TypeId GetTypeId (void);
		 /**
		 * A message handler function that takes packet content from compute
		 * nodes as input and output the operational command orchestrators should reply.
		 **/

		 void
		 HandleMessage(std::string Msgname, std::string content);

		 void
		 BootstrapPhaseHandler(std::string content);

		 void
		 NodeStatusFetchHandler(std::string content);

		 private:
			OrchestratorNodeInfoStorage m_storage_handler;

    };
  }//namespace inc
 } // namespace ndn
} // namespace ns3


#endif
