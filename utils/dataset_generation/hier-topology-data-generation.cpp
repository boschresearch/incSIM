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

#include "hier-topology-data-generation.hpp"

#include <fstream>

#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/log.h"
#include <boost/algorithm/string.hpp>
#include <random>

NS_LOG_COMPONENT_DEFINE("inc.consumerNodeDataGen");

namespace ns3 {
namespace ndn {
namespace inc{

DataGen::DataGen(uint32_t seed,std::string fileToWrite)
{
    //std::cout<<"Inside Constructor"<<std::endl;
    m_fileToWrite = fileToWrite;
    m_seed = seed;
}

DataGen::~DataGen(){};

void
DataGen::generate_data(uint32_t num_consumer_nodes, uint32_t t1_nodes, uint32_t t2_nodes, uint32_t t3_nodes, uint32_t cloud_scale, uint32_t num_func, uint32_t num_data, std::pair<int,int> input_range, std::pair<int,int> frequency_range, uint32_t sim_time, bool zipf, bool nfn_load_distribution_scenario, bool homogeneous_nodes)
{
    //std::cout<<"The seed is "<<m_seed<<std::endl;
    bool static_value = true;
    int num_compute_nodes = t1_nodes + t2_nodes + t3_nodes + 2;
    std::vector<std::string> cores = {"1","2","4","6","8","10", "200"};
    std::vector<std::string> clock_speed_mips = {"1000","2000","4000","8000","10000","40000"};
    std::vector<std::string> ram_mb = {"2","4","8","16","32","64","2048","6144","8192","40960"};
    std::vector<std::string> rom_gb = {"16","32","64","128","256","500","1000","2000"};
    std::vector<std::string> runtimes = {"docker","python","java","ART", "GCC"};
    std::vector<std::string> links = {"l1","l2","l3"};
    std::vector<std::string> exec_time_s = {"1.0","2.0","4.0","6.0","8.0"};
    std::vector<std::string> data_packet_size_KB = {"1","10","100","500","1000","10000","100000","1000000","10000000"};
    std::vector<std::string> freshness_s = {"0","0.1","0.5","1.0","5.0","10.0","50.0","100.0"};
    std::vector<std::string> queue_size = {"5", "10", "50", "100"};
    srand(m_seed);
    fstream fout;
    fout.open(m_fileToWrite, ios::out);
    if(fout)
    {
        //--------------router section----------------------//
        fout<<"router"<<"\n"; //section router
        fout<<"orchestrator"<<"\t"<<"100"<<"\t"<<"30"<<"\t"<<cores.at(5)<<"\t"<<"4000"<<"\t"<<"40960"<<"\t"<<"2000"<<"\t"<<"10"<<"\t"<<(links.at(0)+','+links.at(1)+','+links.at(2))<<"\t"<<(runtimes.at(0)+','+runtimes.at(1)+','+runtimes.at(2))<<"\n";
        for(uint32_t i=1;i<num_compute_nodes; i++)
        {
            std::string compute = "compute_node_";
            nodeProperties compute_node;
            compute_node.name = compute.append(to_string(i-1));
            if(homogeneous_nodes==true)
            {
                compute_node.clock_speed_mips = "8000";
                compute_node.cores= "100";
                compute_node.ram_mb = "10000";
                compute_node.rom_gb = "10000";
                compute_node.queue = "100";
            }
            else
            {
                if(i==1)
                {
                    if (cloud_scale == 1)
                    {
                        //cloud server
                        compute_node.clock_speed_mips = "40000";
                        compute_node.cores= "1000";
                        compute_node.ram_mb = "81920";
                        compute_node.rom_gb = "10000";
                        compute_node.queue = "1000";
                        compute_node.lati = "500";
                        compute_node.longi = "1000";
                    }
                    else if (cloud_scale == 2)
                    {
                        //cloud server
                        compute_node.clock_speed_mips = "40000";
                        compute_node.cores= "2000";
                        compute_node.ram_mb = "163840";
                        compute_node.rom_gb = "10000";
                        compute_node.queue = "2000";
                        compute_node.lati = "500";
                        compute_node.longi = "1000";
                    }
                    else if (cloud_scale == 3)
                    {
                        //cloud server
                        compute_node.clock_speed_mips = "40000";
                        compute_node.cores= "4000";
                        compute_node.ram_mb = "327680";
                        compute_node.rom_gb = "20000";
                        compute_node.queue = "4000";
                        compute_node.lati = "500";
                        compute_node.longi = "1000";
                    }
                }
                else if (i==2)
                {
                    if (cloud_scale == 1)
                    {
                        //ISP gateway
                        compute_node.clock_speed_mips = "20000";
                        compute_node.cores = "500";
                        compute_node.ram_mb = "40960";
                        compute_node.rom_gb = "5000";
                        compute_node.queue = "500";
                        compute_node.lati = "500";
                        compute_node.longi = "800";
                    }
                    else if (cloud_scale == 2)
                    {
                        compute_node.clock_speed_mips = "20000";
                        compute_node.cores = "1000";
                        compute_node.ram_mb = "81920";
                        compute_node.rom_gb = "5000";
                        compute_node.queue = "1000";
                        compute_node.lati = "500";
                        compute_node.longi = "800";
                    }
                    else if (cloud_scale == 3)
                    {
                        compute_node.clock_speed_mips = "40000";
                        compute_node.cores = "2000";
                        compute_node.ram_mb = "163840";
                        compute_node.rom_gb = "5000";
                        compute_node.queue = "2000";
                        compute_node.lati = "500";
                        compute_node.longi = "800";
                    }
                }
                else if( (i > 2) && (i <= (t2_nodes+2)) )
                {
                    compute_node.clock_speed_mips = "8000";
                    compute_node.cores = "40";
                    compute_node.ram_mb="8192";
                    compute_node.rom_gb = "800";
                    compute_node.queue = "40";
                    compute_node.lati = to_string((1000/t2_nodes)*(i-2));
                    compute_node.longi = "600";
                }
                else if( i > (t2_nodes+t1_nodes+1)) //remaining in tier 3
                {
                    compute_node.clock_speed_mips = "4000";
                    compute_node.cores = "20",
                    compute_node.ram_mb = "6144",
                    compute_node.rom_gb = "400",
                    compute_node.queue = "20";
                    compute_node.lati = to_string((1000/t3_nodes)* (i-(t2_nodes+2)));
                    compute_node.longi = "400";
                }
            }
            if(i==1)
            {
                compute_node.lati = "500";
                compute_node.longi = "1000";
            }
            else if (i==2)
            {
                compute_node.lati = "500";
                compute_node.longi = "800";
            }
            else if( (i > 2) && (i <= (t2_nodes+2)) )
            {
                compute_node.lati = to_string((1000/t2_nodes)*(i-2));
                compute_node.longi = "600";
            }
            else if( i > (t2_nodes+t1_nodes+1)) //remaining in tier 3
            {
                compute_node.lati = to_string((1000/t3_nodes)* (i-(t2_nodes+2)));
                compute_node.longi = "400";
            }
            m_node_list.push_back(compute_node);

            fout<<compute_node.name<<"\t"<<compute_node.longi<<"\t"<<compute_node.lati<<"\t"<<compute_node.cores<<"\t"<<compute_node.clock_speed_mips<<"\t"<<compute_node.ram_mb<<"\t"
            <<compute_node.rom_gb<<"\t"<<compute_node.queue<<"\t"<<(links.at(1)+','+links.at(2))<<"\t"<<(runtimes.at(0)+','+runtimes.at(1)+','+runtimes.at(2))<<"\n";
        }

        for(int32_t i=0;i<num_consumer_nodes; i++)
        {
            std::string consumer = "consumer_";
            fout<<(consumer.append(to_string(i)))<<"\t"<<"20"<<"\t"<<i<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\n";
        }



        NS_LOG_INFO("After Router Section"<<std::endl);
        //-------------Links section-----------------------//
        fout<<"\n";
        fout<<"links"<<"\n"; //section links

        for(uint32_t i = 1; i<m_node_list.size();i++)
        {
            fout<<"orchestrator"<<"\t"<<(m_node_list.at(i).name)<<"\t"<<"10000Mbps"<<"\t"<<"50"<<"\t"<<"5ms"<<"\t"<<"1000"<<"\n"; //orchestrator with other compute nodes except cloud
        }
        //link between compute nodes
        int l,k;
        for(uint32_t i=0; i<=t2_nodes+1 ;i++)
        {
            if(i==0)
            {
                std::string compute_node_1 = "compute_node_";
                std::string compute_node_2 = "compute_node_";
                fout<<compute_node_1.append(to_string(i))<<"\t"<<compute_node_2.append(to_string(i+1))<<"\t"<<"10000Mbps"<<"\t"<<"1"<<"\t"<<"200ms"<<"\t"<<"1000"<<"\n";
            }
            if(i==1)
            {
                l = 2;
                while(l<=t2_nodes+1)
                {
                    std::string compute_node_1 = "compute_node_";
                    std::string compute_node_2 = "compute_node_";
                    fout<<compute_node_1.append(to_string(i))<<"\t"<<compute_node_2.append(to_string(l))<<"\t"<<"10000Mbps"<<"\t"<<"1"<<"\t"<<"25ms"<<"\t"<<"1000"<<"\n";
                    l++;
                }
            }
            else if (i>1)
            {
                if(i==2)
                {
                    k=t2_nodes+2;
                }
                std::string compute_node_1 = "compute_node_";
                std::string compute_node_2 = "compute_node_";
                fout<<compute_node_1.append(to_string(i))<<"\t"<<compute_node_2.append(to_string(k))<<"\t"<<"10000Mbps"<<"\t"<<"1"<<"\t"<<"5ms"<<"\t"<<"1000"<<"\n";
                k++;
                compute_node_1 = "compute_node_";
                compute_node_2 = "compute_node_";
                fout<<compute_node_1.append(to_string(i))<<"\t"<<compute_node_2.append(to_string(k))<<"\t"<<"10000Mbps"<<"\t"<<"1"<<"\t"<<"5ms"<<"\t"<<"1000"<<"\n";
                k++;
            }
        }
        uint32_t q = num_consumer_nodes / t3_nodes;
        for(uint32_t i = 0 ; i<t3_nodes;i++)
        {
            for(uint32_t j=0;j<q;j++)
            {
                std::string consumer = "consumer_";
                uint32_t consumer_suffix = ((j) + ((i)*(q)));
                fout<<(m_node_list.at(t2_nodes+2+i).name)<<"\t"<<(consumer.append(to_string(consumer_suffix)))<<"\t"<<"250Mbps"<<"\t"<<"1"<<"\t"<<"2ms"<<"\t"<<"10"<<"\n";
            }
        }
        for( uint32_t i = 0; i<t3_nodes;i++)
        {
            std::string consumer = "consumer_";
            uint32_t consumer_suffix = (i)+(q * (t3_nodes));
            if(consumer_suffix < num_consumer_nodes)
                fout<<(m_node_list.at(t2_nodes+2+i).name)<<"\t"<<(consumer.append(to_string(consumer_suffix)))<<"\t"<<"250Mbps"<<"\t"<<"1"<<"\t"<<"2ms"<<"\t"<<"10"<<"\n";
            else
                break;
        }

        NS_LOG_INFO("After Links Section"<<std::endl);
        //-------------functions section--------------------//
        fout<<"\n";
        fout<<"functions"<<"\n"; //section functions
        generate_func_list(num_func, input_range);
        generate_provided_data(num_data);
        generate_start_time(num_consumer_nodes, sim_time);
        generate_stop_time(num_consumer_nodes, sim_time);

        for(uint32_t i=0;i<num_func;i++)
        {
            if(static_value == false)
            {
                m_func_list.at(i).exec_time = exec_time_s.at(rand()%5);
                m_func_list.at(i).num_instructions = clock_speed_mips.at(rand()%6);
                m_func_list.at(i).ram_mb=ram_mb.at(rand()%3);
                m_func_list.at(i).rom_gb=rom_gb.at(rand()%3);
                m_func_list.at(i).cores=cores.at(rand()%3);
                m_func_list.at(i).func_size_KB = data_packet_size_KB.at(rand()%8);
                m_func_list.at(i).result_size_KB = data_packet_size_KB.at(rand()%5);
            }
            else
            {
                m_func_list.at(i).exec_time = "3.0";
                m_func_list.at(i).num_instructions = "4000";
                m_func_list.at(i).func_size_KB = "10000"; //currently set to a constant value
                m_func_list.at(i).result_size_KB ="500"; //currently set to a constant value
                m_func_list.at(i).cores="1";
                m_func_list.at(i).ram_mb="100";
                m_func_list.at(i).rom_gb="16";
            }
            fout<<(m_func_list.at(i).name)<<"\t"<<(m_func_list.at(i).input_signature)<<"\t"<<runtimes.at(0)<<"\t"<<m_func_list.at(i).ram_mb<<"\t"<<m_func_list.at(i).rom_gb<<"\t"<<m_func_list.at(i).cores<<"\t"<<m_func_list.at(i).exec_time
            <<"\t"<<m_func_list.at(i).num_instructions<<"\t"
            <<m_func_list.at(i).func_size_KB<<"\t"<<m_func_list.at(i).result_size_KB<<"\n";
        }


        NS_LOG_INFO("After functions Section"<<std::endl);
        //-----------initial function placement section------//
        fout<<"\n";
        fout<<"initial function status"<<"\n"; // section intial placement to cloud server
        //std::cout<<"initial function status"<<std::endl;
        for(uint32_t i=0;i<num_func;i++)
        {
            fout<<"compute_node_0"<<"\t"<<(m_func_list.at(i).name)<<"\n";
        }
        NS_LOG_INFO("initial function placement done!"<<std::endl);

        //----------consumer section---------------------//
        fout<<"\n";
        fout<<"consumer section"<<"\n"; // define consumer request behavior
        NS_LOG_INFO("consumer section"<<std::endl);

        std::vector<double> l_prob_vector = createZipfMandelbrotDist(num_func, 0.7, 0.7);
        generate_int_lifetime(num_func, frequency_range);
        int load_index;
        //std::cout<<"The load index is "<<load_index<<std::endl;
        for(int k = 0; k < 3; k++)
        {
            load_index = rand()%((num_consumer_nodes/20)+1);
            for(uint32_t i=0;i<num_consumer_nodes/2;i++)
            {
                std::string consumer = "consumer_";
                int j;

                if(zipf==true)
                {
                    if(nfn_load_distribution_scenario == true)
                    {
                        j = ((i % (num_consumer_nodes/t3_nodes)) + (load_index*(num_consumer_nodes/t3_nodes)));
                        if(j>=num_consumer_nodes)
                            continue;
                    }
                    else
                    {
                        j = i;
                    }
                    uint32_t index = getNextZipfMandelbrotRandomNumber(l_prob_vector, num_func);
                    index = index-1;
                    string inputs = generate_input_list((m_func_list.at(index)).num_inputs, num_data);
                    fout<<(consumer.append(to_string(j)))<<"\t"<<(m_func_list.at(index)).name<<"\t"<<inputs<<"\t"<<m_freq.at(index)<<"\t"<<(m_func_list.at(index)).lifetime<<"\t"
                    <<m_startTime.at(j)<<"\t"<<m_stopTime.at(j)<<"\n";
                }
                else
                {
                    if(nfn_load_distribution_scenario == true)
                    {
                        j = (i % (num_consumer_nodes/t3_nodes)+ (load_index*k*(num_consumer_nodes/t3_nodes)));
                        if(j>=num_consumer_nodes)
                            continue;
                    }
                    else
                    {
                        j= i;
                    }
                    string inputs = generate_input_list((m_func_list.at(i)).num_inputs, num_data);
                    fout<<(consumer.append(to_string(j)))<<"\t"<<(m_func_list.at(j)).name<<"\t"<<inputs<<"\t"<<m_freq.at(j)<<"\t"<<(m_func_list.at(j)).lifetime<<"\t"
                    <<m_startTime.at(j)<<"\t"<<m_stopTime.at(j)<<"\n";
                }
            }
        }

         //usual consumer request at other consumer nodes (connected to other compute nodes) during nfn_load_distribution_scenario.
          if(nfn_load_distribution_scenario == true)
        {
            for(uint32_t i=0 ; i<num_consumer_nodes; i++)
            {
                std::string consumer = "consumer_";
                if(zipf==true)
                {
                    uint32_t index = getNextZipfMandelbrotRandomNumber(l_prob_vector, num_func);
                    index = index-1;
                    string inputs = generate_input_list((m_func_list.at(index)).num_inputs, num_data);
                    fout<<(consumer.append(to_string(i)))<<"\t"<<(m_func_list.at(index)).name<<"\t"<<inputs<<"\t"<<m_freq.at(index)<<"\t"<<(m_func_list.at(index)).lifetime<<"\t"
                    <<m_startTime.at(i)<<"\t"<<m_stopTime.at(i)<<"\n";

                }
                else
                {
                    string inputs = generate_input_list((m_func_list.at(i)).num_inputs, num_data);
                    fout<<(consumer.append(to_string(i)))<<"\t"<<(m_func_list.at(i)).name<<"\t"<<inputs<<"\t"<<m_freq.at(i)<<"\t"<<(m_func_list.at(i)).lifetime<<"\t"
                    <<m_startTime.at(i)<<"\t"<<m_stopTime.at(i)<<"\n";
                }
            }
        }
        NS_LOG_INFO("consumer section done"<<std::endl);

        //------------initial data section---------------------//
        fout<<"\n";
        fout<<"initial data section"<<"\n";
        int cons_index = 0;
        for(uint32_t i=0;i<m_provided_data.size(); i++)
        {
            std::string consumer = "consumer_";
            string data_size_KB = "500";
            string freshness_s = "2";
            fout<<consumer.append(to_string(cons_index))<<"\t"<<m_provided_data.at(i).name<<"\t"<<data_size_KB<<"\t"<<freshness_s<<"\n";
            cons_index++;
            if(cons_index==num_consumer_nodes)
            {
                cons_index = 0;
            }
        }
        NS_LOG_INFO("initial data section done"<<std::endl);
    }
    fout.close();
}

void
DataGen::generate_func_list(uint32_t num_func, std::pair<int, int>input_range)
{
    int func_min = input_range.first;
    int func_max = input_range.second;
    for(uint32_t i=0; i< num_func; i++)
    {
        std::string func = "func-";
        funcProperties function;
        function.name = func.append(to_string(i));
        do{
        function.num_inputs = rand()%(func_max-func_min+1)+func_min;
        }while(function.num_inputs==0);
        if(function.num_inputs == 0)
        {
            function.input_signature = "null";
        }
        else
        {
            for(int j = 1; j<=function.num_inputs; j++)
            {
                std::string operand = "Operand_";
                operand.append(to_string(j%2));
                function.input_signature.append(operand+',');
            }
            function.input_signature.pop_back();
        }
        m_func_list.push_back(function);
    }
}

std::string
DataGen::generate_input_list(int num_inputs, uint32_t num_data)
{
    int data_max = num_data;
    int data_min = 0;
    std::string input_per_func;
    if(num_inputs==0)
    {
        input_per_func.append("null");
        return input_per_func;
    }
    else
    {
        std::string operand;
        for(int j=1; j<=num_inputs;j++)
        {
            do{
                operand = "Operand_";
                int random_value = rand()%(data_max-data_min+1)+data_min;
                std::string value = to_string(random_value);
                operand.append(value);
            }while(input_per_func.find(operand) != input_per_func.npos);
            input_per_func.append(operand+','); // update inputs corresponding to number of inputs
        }
        input_per_func.pop_back();
        return input_per_func;
    }
}

void
DataGen::generate_int_lifetime(uint32_t num_func, std::pair<int, int>frequency_range)
{
    for(uint32_t i=0;i<num_func; i++)
    {
        std::vector<std::string> freq_list = {"5","7.5","10","12.5","15"};
        m_func_list.at(i).lifetime = "20";
        std::string freq = freq_list.at(rand()%5);
        m_freq.push_back(freq);
        //int min = frequency_range.first;
        //int max = frequency_range.second;
        //m_freq.push_back(to_string(rand()%(max-min+1)+min));
    }
}

void
DataGen::generate_provided_data(uint32_t num_data)
{

    int min = 0; int max = num_data; //Number of different provided data
    //int max = 2;
    for(uint32_t i=0;i<num_data; i++)
    {
        dataProperties new_data;
        do{
            new_data.name = "Operand_";
            (new_data.name).append(to_string(rand()%(max-min+1)+min));
            auto it = find_if(begin(m_provided_data), end(m_provided_data), [=] (dataProperties const& dp)
            {
             return (dp.name == new_data.name);
            });

            if(it == end(m_provided_data))
            {
                m_provided_data.push_back(new_data);
                break;
            }
        }while(true);
    }
}

void
DataGen::generate_start_time(uint32_t num_consumer_nodes, uint32_t sim_time)
{
    int min = 7;
    int max = sim_time/3;
    for(uint32_t i=0; i<num_consumer_nodes; i++)
    {
        m_startTime.push_back(to_string(rand()%(max-min+1)+min));
    }
}

void
DataGen::generate_stop_time(uint32_t num_consumer_nodes, uint32_t sim_time)
{
    int min = (2 * sim_time)/3;
    int max = sim_time-2;
    for(uint32_t i=0; i<num_consumer_nodes; i++)
    {
        m_stopTime.push_back(to_string(rand()%(max-min+1)+min));
    }
}

std::vector<double>
DataGen::createZipfMandelbrotDist(uint32_t f_N, double f_q, double f_s) {
	std::vector<double> l_prob_vector = std::vector<double>(f_N + 1);

	// calculate some values based on the zipf mandelbrot distribution and store it in an array
	for (uint32_t i = 1; i <= f_N; i++) {
		l_prob_vector[i] = l_prob_vector[i - 1] + 1.0 / std::pow(i + f_q, f_s);
	}

	// calculate the cumulative probability by printing it to the console
	for (uint32_t i = 1; i <= f_N; i++) {
		l_prob_vector[i] = l_prob_vector[i] / l_prob_vector[f_N];
		//std::cout<<"Cumulative probability [" << i << "]=" << l_prob_vector[i]<<std::endl;
	}
	return l_prob_vector;
}

uint32_t
DataGen::getNextZipfMandelbrotRandomNumber(std::vector<double> dist_vector, uint32_t basis) {
    m_seqRNG = CreateObject<UniformRandomVariable>();
	uint32_t content_index = 1; 	// set to 1 of the set [1, m_N]
	double p_sum = 0;				// prob value is

	double p_random = m_seqRNG->GetValue();
	while (p_random == 0) {
		p_random = m_seqRNG->GetValue();
	}

	// if (p_random == 0)
	NS_LOG_LOGIC("p_random=" << p_random);
	for (uint32_t i = 1; i <= basis; i++) {
		p_sum = dist_vector[i]; // m_Pcum[i] = m_Pcum[i-1] + p[i], p[0] = 0;   e.g.: p_cum[1] = p[1],
		// p_cum[2] = p[1] + p[2]
		if (p_random <= p_sum) {
			content_index = i;
			break;
		} // if
	}   // for

	// content_index = 1;
	NS_LOG_INFO("RandomNumber=" << content_index);

	return content_index;
}

} //namespace inc
} //namespace ndn
} //namespace ns3
