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
*      Robert Bosch GmbH - initial API and functionality
*          Uthra Ambalavanan <uthra.ambalavanan@de.bosch.com>
* *****************************************************************************
*/


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/netanim-module.h"
#include "ns3/incSIM-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

namespace ns3 {
 namespace ndn {
   namespace inc {
   /*
   A simulation scenario with Hybrid orchestration. When orchestrationSwitch is false, the network performs as  plain NFN.
   With the switch set to true, a centralized orchestrator periodically polls the compute nodes for their status information.
   Based on this information, the orchestrator performs function enabling and disabling decisions at the compute nodes.
   These decisions improvise the completion time and load utilization at the edge nodes.
   */
   NS_LOG_COMPONENT_DEFINE ("inc.ndnOrchestrationSimulation");
   int main(int argc, char* argv[])
   {
     double interval = 20;
     bool orchestrationSwitch=false;
     bool nfn_load_distribution_scenario = true;
     bool wrongParam = false;
     bool homogeneous_nodes = false;
     uint32_t cloud_scale = 1;
     uint32_t num_consumer_nodes = 20;
     uint32_t num_data = 20;
     uint32_t t1_nodes = 1;
     uint32_t t2_nodes = 2;
     uint32_t t3_nodes = 4;
     uint32_t num_compute_nodes = t1_nodes+t2_nodes+t3_nodes+2;
     uint32_t num_of_func = 80;
     uint32_t seed = 1;
     int input_range = 2;
     int freq_range =9;
     int simulation_time = 200;
     std::string topology_path = "./src/incSIM/examples/topologies/";
     std::string trace_path = "./Traces/";
     std::string orch_traceFile = "orch_traceFile.txt";
     std::string orch_node_traceFile = "compute_node_orch_traceFile.txt";
     std::string compute_traceFile = "compute_traceFile.txt";
     std::string consumer_traceFile = "consumer_traceFile.txt";
     std::string app_delay_traceFile = "app_delay_traceFile.txt";
     std::string queue_traceFile = "queue_traceFile.txt";
     std::string resource_utilization_traceFile = "Node_ResUtilization.txt";
     std::string strategy = "NFN";
     // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
     CommandLine cmd;
     cmd.AddValue("consumer-nodes","no of edge consumer nodes", num_consumer_nodes);
     cmd.AddValue("t1-nodes", "no of tier 1 compute capable nodes",t1_nodes);
     cmd.AddValue("t2-nodes", "no of tier 2 compute capable nodes",t2_nodes);
     cmd.AddValue("t3-nodes", "no of tier 3 compute capable nodes",t3_nodes);
     cmd.AddValue("cloud-config", "cloud configuration setting", cloud_scale);
     cmd.AddValue("functions", "no of functions", num_of_func);
     cmd.AddValue("data", "no of data", num_data);
     cmd.AddValue("input range", "range of no of data inputs per function upper limit", input_range);
     cmd.AddValue("freq range", "range of no of request frequency per function upper limit", freq_range);
     cmd.AddValue("seed", "A seed iput for random number generator", seed);
     cmd.AddValue("interval", "interval of periodic orchestration requests", interval);
     cmd.AddValue("orchestration-switch", "turn orchestration switch on/off", orchestrationSwitch);
     cmd.AddValue("wrong-parameter", "use wrong number of parameters (only for demo purpose)", wrongParam);
     cmd.AddValue("topology-path","folder where the topology file can be accessed from", topology_path);
     cmd.AddValue("trace-path", "folder where the trace files are stored", trace_path);
     cmd.AddValue("sim-time", "simulation time in seconds", simulation_time);
     cmd.AddValue("inc_strategy", "the inc strategy switch", strategy);

     cmd.Parse(argc, argv);
     Time::SetResolution (Time::NS);

     //-----------------------------------------------------------------------------//
     //------------------------Logging within applications--------------------------//
     //-----------------------------------------------------------------------------//

     /* Other LogComponents in this scenario to be used:
      * inc.ConsumerApp, inc.INCConsumerBaseApp,IncNdnAnnotatedTopologyReader,
      * IncOrchestrationComputeNode, inc.IncConsumerTracer, NfnProducerApp
      * ndn-cxx.nfd.Forwarder, NFN_InterestResolutionEngine, inc.parserNodeInfo
      * ndn.GlobalRoutingHelper*/
     LogComponentEnable("inc.ndnOrchestrationSimulation", LOG_LEVEL_INFO);

     //-----------------------------------------------------------------------------//
     //-----------------------------Initiate Topology-------------------------------//
     //-----------------------------------------------------------------------------//

     num_data = num_consumer_nodes;
     num_compute_nodes = t1_nodes + t2_nodes + t3_nodes + 2;
     std::cout << "num_data is " << num_data << std::endl;
     std::cout << "num_compute_nodes is " << num_compute_nodes << std::endl;
     DataGen generator(seed, (topology_path.append("inc_topology.txt")));
     if(nfn_load_distribution_scenario == true)
     {
       generator.generate_data(num_consumer_nodes, t1_nodes, t2_nodes, t3_nodes,
         cloud_scale, num_of_func, num_data, std::pair<int, int>(0,input_range),
         std::pair<int, int>(7, freq_range), simulation_time, true,
         nfn_load_distribution_scenario = true, homogeneous_nodes);
     }
     else
     {
       generator.generate_data(num_consumer_nodes, t1_nodes, t2_nodes, t3_nodes,
         cloud_scale, num_of_func, num_data, std::pair<int,int>(0,input_range),
         std::pair<int, int>(7,freq_range), simulation_time, true,
         nfn_load_distribution_scenario = false, homogeneous_nodes);
     }

     IncNdnAnnotatedTopologyReader topologyReader("", 1);
     topologyReader.SetFileName(topology_path);

     std::pair<NodeContainer,NodeContainer> NC_nodes;
     //get NodeContainer from topology reader
     NC_nodes = topologyReader.ReadTopology();
     //get list of compute nodes and consumer nodes from topology reader
     std::vector<Ptr<IncOrchestrationComputeNode>> computeNodes = topologyReader.GetComputeNodes();
     std::vector<Ptr<Node>> consumerNodes = topologyReader.GetConsumerNodes();

     //get initial function distribution from topology reader
     std::unordered_map<std::string,std::vector<std::string>> initial_function_status_map=
     topologyReader.GetInitialFunctionStatusMap();

     for(uint32_t i = 0; i < num_compute_nodes; i++){
       Ptr<IncOrchestrationComputeNode> t_compute_node = computeNodes.at(i);
       std::cout << "node" << i
       <<" "<<"name="<< t_compute_node->GetName()
       <<" "<<"processor_core=" << t_compute_node->GetProcessorCore()
       <<" "<<"processor_type=" << t_compute_node->GetProcessorClockSpeed()
       <<" "<<"ram=" << t_compute_node->GetRam()
       <<" "<<"rom=" << t_compute_node->GetRom()
       <<" "<<"supported_runtimes=" << t_compute_node->GetSupportedRuntimes()
       << std::endl;

       t_compute_node->SetIncStragety(strategy);
     }

     // //print initial function status map
     std::cout << std::endl;
     std::cout << "read in initial function status:" << std::endl;
     for (auto it = initial_function_status_map.begin();
       it != initial_function_status_map.end(); ++it )
     {
       for(uint32_t i=0;i<it->second.size();i++)
       {
         std::cout << " " << it->first << " " << it->second.at(i) << std::endl;
       }
     }
     std::cout <<std::endl;

     //-----------------------------------------------------------------------------//
     //------------------------------------NDN--------------------------------------//
     //-----------------------------------------------------------------------------//
     //Install NDN stack on all icn nodes
     ndn::StackHelper ndnHelper;
     ndnHelper.setPolicy("nfd::cs::lru");
     ndnHelper.setCsSize(1000);

     ndnHelper.InstallAll();
     topologyReader.ApplyOspfMetric();

     ndn::StrategyChoiceHelper::Install(NC_nodes.first,"","/localhost/nfd/strategy/Multicast");
     ndn::StrategyChoiceHelper::Install(NC_nodes.second,"", "/localhost/nfd/strategy/Multicast");

     ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
     ndnGlobalRoutingHelper.Install(NC_nodes.first);
     ndnGlobalRoutingHelper.Install(NC_nodes.second);
     //-----------------------------------------------------------------------------//
     //--------------------------storing compute node pointers----------------------//
     //-----------------------------------------------------------------------------//


     parserNodeInfo node_info_parser(topology_path);
     std::vector<compute_node_struct> compute_node_info;
     compute_node_info = node_info_parser.update_compute_info(computeNodes);
     std::cout<<"size of compute_node_info is " << compute_node_info.size() << std::endl;
     std::vector<consumer_node_struct> consumer_node_info;
     consumer_node_info = node_info_parser.update_consumer_info(consumerNodes);

     //-----------------------------------------------------------------------------//
     //-----------------------Function Initial setup--------------------------------//
     //-----------------------------------------------------------------------------//

     std::vector<std::string> enable_functions;
     for (uint32_t i = 1; i < num_compute_nodes; i++)
     {
       std::string prefix = "/lambda/Function/";
       compute_node_info[i].function_prefix.push_back(prefix);
       std::cout<<"compute_node_info[i].Name is " << compute_node_info[i].Name << std::endl;
       //get initial function status for node
       if(initial_function_status_map.find(computeNodes.at(i)->GetName())
         != initial_function_status_map.end())
       {
         enable_functions = initial_function_status_map[computeNodes.at(i)->GetName()];
         for(uint32_t j = 0; j < enable_functions.size(); ++j)
         {
           std::string t_prefix = "/Function/";
           computeNodes.at(i)->EnableFunction("/" + enable_functions.at(j));
           t_prefix.append(enable_functions.at(j));
           compute_node_info[i].function_prefix.push_back(t_prefix);
         }
       }
     }


     //-----------------------------------------------------------------------------//
     //-----------------------NDN Orchestrator Bootstraping-------------------------//
     //-----------------------------------------------------------------------------//
     if(orchestrationSwitch)
     {
       for(uint32_t i = 2; i < num_compute_nodes; i++)
       {
         ndn::AppHelper OrchestratorProducer("ns3::ndn::inc::NdnOrchestrationComputeNodeApp");
         OrchestratorProducer.SetPrefix(compute_node_info.at(i).node_prefix);
         OrchestratorProducer.SetAttribute("PayloadSize", StringValue("1024"));
         OrchestratorProducer.SetAttribute("ComputeNodePointer",PointerValue(computeNodes.at(i)));
         OrchestratorProducer.Install(NC_nodes.first.Get(i));
       }


       for(uint32_t i = 2; i < num_compute_nodes;i++)
       {
         ndn::AppHelper OrchestratorHelper_BootStrap("ns3::ndn::inc::NdnOrchestrationCommunicationApp");
         OrchestratorHelper_BootStrap.SetPrefix(compute_node_info.at(i).node_prefix+"/BootstrapInfo");
         OrchestratorHelper_BootStrap.SetAttribute("ExecutionTime", TimeValue(Seconds(1.0)));
         OrchestratorHelper_BootStrap.Install(NC_nodes.first.Get(0));
       }
     }

     //-----------------------------------------------------------------------------//
     //-------------------------Function Execution Requests-------------------------//
     //-----------------------------------------------------------------------------//
     int t_i=0;
     for(auto itr = consumer_node_info.begin();
       itr != consumer_node_info.end(); ++itr)
     {
       int j = 0;
       for(auto it= itr-> function_prefix.begin();
         it != itr->function_prefix.end(); ++it)
       {
         ndn::AppHelper consumerHelper ("ns3::ndn::inc::ConsumerApp");
         consumerHelper.SetPrefix(*it);
         consumerHelper.SetAttribute("Interval", StringValue(itr->request_frequency.at(j)));
         consumerHelper.SetAttribute("LifeTime", StringValue(itr->lifetime.at(j)));
         consumerHelper.SetAttribute("ComputeNode", BooleanValue(false));
         ApplicationContainer ConsumerApp = consumerHelper.Install(NC_nodes.second.Get(t_i));
         ConsumerApp.Start(Seconds(stoi(itr->start_time.at(j))));
         ConsumerApp.Stop(Seconds(stoi(itr->stop_time.at(j))));
         j++;
       }
       t_i++;
     }

     for(uint32_t i = 0;i < num_compute_nodes; i++)
     {
       if((i == 0) && (orchestrationSwitch == true))
       {
         ndn::AppHelper producerHelper("ns3::ndn::inc::NfnProducerApp");
         producerHelper.SetPrefix("/Orchestrate");
         producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
         producerHelper.SetAttribute("ComputeNodePointer", PointerValue(computeNodes.at(0)));
         producerHelper.Install(NC_nodes.first.Get(0));
       }
       else
       {
         for(auto itr = compute_node_info[i].function_prefix.begin();
           itr != compute_node_info[i].function_prefix.end(); ++itr)
         {
           ndn::AppHelper producerHelper("ns3::ndn::inc::NfnProducerApp");
           NS_LOG_INFO("NFNProducerApp for prefix " << *itr << " at node " << computeNodes.at(i)->GetName());
           producerHelper.SetPrefix(*itr);
           producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
           producerHelper.SetAttribute("ComputeNodePointer", PointerValue(computeNodes.at(i)));
           producerHelper.Install(NC_nodes.first.Get(i));
         }
       }
     }

     //---------------------------------------------------------//
     //----------------------Data Producer----------------------//
     //---------------------------------------------------------//

     int i=0;
     for(auto it = consumer_node_info.begin();
       it != consumer_node_info.end(); it++)
     {
       int j=0;
       for(auto itr = it->data_prefix_provided.begin();
         itr != it->data_prefix_provided.end(); ++itr)
       {
         ndn::AppHelper dataProducerHelper("ns3::ndn::inc::DataProducer");
         dataProducerHelper.SetPrefix(*itr);
         dataProducerHelper.SetAttribute("Freshness", TimeValue(Seconds(7.0)));
         dataProducerHelper.SetAttribute("PayloadSize", UintegerValue(stoi(it->data_size.at(j))));
         dataProducerHelper.Install(NC_nodes.second.Get(i));
         j++;
       }
       i++;
     }
     //-----------------------------------------------------------------------------//
     //--------------------------Periodic Node Status Fetch-------------------------//
     //-----------------------------------------------------------------------------//

     if(orchestrationSwitch)
     {
       for(uint32_t i = 2; i < num_compute_nodes; i++)
       {
         ndn::AppHelper OrchestratorHelper_NodeStatusFetch("ns3::ndn::inc::NdnOrchestrationCommunicationApp");
         OrchestratorHelper_NodeStatusFetch.SetPrefix(compute_node_info.at(i).node_prefix+"/NodeStatusFetch");
         OrchestratorHelper_NodeStatusFetch.SetAttribute("Periodic", BooleanValue(true));
         OrchestratorHelper_NodeStatusFetch.SetAttribute("Interval", TimeValue(Seconds(interval)));
         OrchestratorHelper_NodeStatusFetch.SetAttribute("ExecutionTime", TimeValue(Seconds(10.0)));
         OrchestratorHelper_NodeStatusFetch.Install(NC_nodes.first.Get(0));
       }

     //-----------------------------------------------------------------------------//
     //--------------------------Function Counter Requests--------------------------//
     //-----------------------------------------------------------------------------//


       ndn::AppHelper OrchestratorHelper_FunctionCounter("ns3::ndn::inc::OrchestrationManagementApp");
       OrchestratorHelper_FunctionCounter.SetAttribute("OrchestrationStrategy", StringValue("Function Switch"));
       OrchestratorHelper_FunctionCounter.SetAttribute("Periodic", BooleanValue(true));
       OrchestratorHelper_FunctionCounter.SetAttribute("Interval", TimeValue(Seconds(interval)));
       OrchestratorHelper_FunctionCounter.SetAttribute("ExecutionTime", TimeValue(Seconds(12.0)));
       OrchestratorHelper_FunctionCounter.SetAttribute("CommunicationModel", StringValue("In-Band"));
       OrchestratorHelper_FunctionCounter.Install(NC_nodes.first.Get(0));
     }

     //-----------------------------------------------------------------------------//
     //-------------------Add prefix origins to ndn::GlobalRouter-------------------//
     //-----------------------------------------------------------------------------//
     if(orchestrationSwitch)
     {
       for(uint32_t i = 0; i < num_compute_nodes; i++)
       {
         NS_LOG_INFO("Adding prefix to origin : " << compute_node_info.at(i).node_prefix);
         ndnGlobalRoutingHelper.AddOrigin(compute_node_info.at(i).node_prefix, (NC_nodes.first.Get(i)));
       }
     }


     for(uint32_t i=1; i<num_compute_nodes;i++)
     {
       for(auto itr = compute_node_info[i].function_prefix.begin();
         itr != compute_node_info[i].function_prefix.end(); ++itr)
       {
         NS_LOG_INFO("Adding prefix " << *itr << " to origin node : " << compute_node_info.at(i).Name);
         ndnGlobalRoutingHelper.AddOrigin(*itr, NC_nodes.first.Get(i));
       }
     }


     for(uint32_t i = 0 ; i < num_consumer_nodes;i ++)
     {
       for(auto itr = consumer_node_info[i].data_prefix_provided.begin();
         itr != consumer_node_info[i].data_prefix_provided.end(); ++itr)
       {
         NS_LOG_INFO("Adding prefix " << *itr << " to origin node : " << consumer_node_info.at(i).Name);
         ndnGlobalRoutingHelper.AddOrigin(*itr, NC_nodes.second.Get(i));
       }
     }


     //-----------------------------------------------------------------------------//
     //-------------------------Calculate and install FIBs--------------------------//
     //-----------------------------------------------------------------------------/
     ndn::GlobalRoutingHelper::CalculateRoutes ();

     //-----------------------------------------------------------------------------//
     //----------------------------------Simulation---------------------------------//
     //-----------------------------------------------------------------------------//

     std::string consumer_trace_path;
     consumer_trace_path.append(trace_path + consumer_traceFile);
     NS_LOG_INFO(consumer_trace_path);

     std::string compute_trace_path;
     compute_trace_path.append(trace_path + compute_traceFile);
     NS_LOG_INFO(compute_trace_path);

     std::string orch_trace_path;
     orch_trace_path.append(trace_path + orch_traceFile);
     NS_LOG_INFO(orch_trace_path);

     std::string orch_node_trace_path;
     orch_node_trace_path.append(trace_path + orch_node_traceFile);
     NS_LOG_INFO(orch_node_trace_path);

     std::string app_delay_trace_path;
     app_delay_trace_path.append(trace_path + app_delay_traceFile);
     NS_LOG_INFO(app_delay_trace_path);

     std::string node_resource_utilization_trace_path;
     node_resource_utilization_trace_path.append(trace_path + resource_utilization_traceFile);
     NS_LOG_INFO(node_resource_utilization_trace_path);

     inc::IncConsumerTracer::Install(NC_nodes.second, consumer_trace_path, Seconds(1));
     inc::IncAppDelayTracer::Install(NC_nodes.second, app_delay_trace_path);

     NodeContainer t_compute_nodes;
     for(uint32_t i = 1; i < num_compute_nodes; i++)
     {
       t_compute_nodes.Add(NC_nodes.first.Get(i));
     }
     inc::IncComputeTracer::Install(t_compute_nodes, compute_trace_path, Seconds(1));
     inc::IncComputeNodeTracer::Install(t_compute_nodes, node_resource_utilization_trace_path, Seconds(1));

     if(orchestrationSwitch)
     {
       inc::IncOrchestratorCommunicationTracer::Install(NC_nodes.first.Get(0), orch_trace_path, Seconds(1));
       inc::IncOrchestratorTracerNode::Install(t_compute_nodes, orch_node_trace_path, Seconds(1));
     }

     Simulator::Stop (Seconds (simulation_time));
     AnimationInterface anim ("animation_ndn_orchestration-scaled.xml");
     Simulator::Run ();
     Simulator::Destroy ();


     return 0;
    }
   }
 }
}

int
main(int argc, char* argv[])
{
 return ns3::ndn::inc::main(argc, argv);
}
