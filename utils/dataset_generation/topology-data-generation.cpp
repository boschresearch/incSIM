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

#include "topology-data-generation.hpp"

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
    std::cout<<"Inside Constructor"<<std::endl;
    m_fileToWrite = fileToWrite;
    m_seed = seed;
}

DataGen::~DataGen(){};

void
DataGen::generate_data(uint32_t num_consumer_nodes, uint32_t num_compute_nodes, uint32_t num_func, std::pair<int,int> input_range, std::pair<int,int> frequency_range, uint32_t sim_time, bool zipf, bool nfn_load_distribution_scenario)
{
    std::cout<<"The seed is "<<m_seed<<std::endl;
    bool static_value = true;

    std::vector<std::string> cores = {"1","2","4","6","8","10"};
    std::vector<std::string> clock_speed = {"200M","450M","600M", "700M", "850M", "1000M"};
    std::vector<std::string> ram = {"2","4","8","16","32","64"};
    std::vector<std::string> rom = {"16","32","64","128","256","500","1000","2000"};
    std::vector<std::string> runtimes = {"docker","python","java","ART", "GCC"};
    std::vector<std::string> links = {"l1","l2","l3"};
    std::vector<std::string> exec_time = {"1.0","2.0","4.0","6.0","8.0"};
    std::vector<std::string> data_packet_size = {"1000","10000","100000","1000000","10000000","100000000","1000000000","10000000000"};
    std::vector<std::string> freshness = {"0","0.1","0.5","1.0","5.0","10.0","50.0","100.0"};
    srand(m_seed);
    fstream fout;
    fout.open(m_fileToWrite, ios::out);
    if(fout)
    {
        //--------------router section----------------------//
        fout<<"router"<<"\n"; //section router
        fout<<"orchestrator"<<"\t"<<"2"<<"\t"<<"25"<<"\t"<<cores.at(4)<<"\t"<<clock_speed.at(5)<<"\t"<<ram.at(5)<<"\t"<<rom.at(7)<<"\t"<<(links.at(0)+','+links.at(1)+','+links.at(2))<<"\t"<<(runtimes.at(0)+','+runtimes.at(1)+','+runtimes.at(2))<<"\n";
        for(uint32_t i=1;i<num_compute_nodes; i++)
        {
            std::string compute = "compute_node_";
            nodeProperties compute_node;
            compute_node.name = compute.append(to_string(i));
            if(static_value == false)
            {
                compute_node.cores = cores.at(rand()%cores.size());
                compute_node.ram = ram.at(rand()%ram.size());
                compute_node.rom = rom.at(rand()%rom.size());
            }
            else
            {
                compute_node.cores = "4";
                compute_node.ram="16";
                compute_node.rom = "64";
            }
            m_node_list.push_back(compute_node);

            fout<<compute_node.name<<"\t"<<"1"<<"\t"<<to_string((i-1)*5)<<"\t"<<compute_node.cores<<"\t"<<clock_speed.at(rand()%clock_speed.size())<<"\t"<<compute_node.ram<<"\t"
            <<compute_node.rom<<"\t"<<(links.at(1)+','+links.at(2))<<"\t"<<(runtimes.at(0)+','+runtimes.at(1)+','+runtimes.at(2))<<"\n";
        }

        for(int32_t i=0;i<num_consumer_nodes; i++)
        {
            std::string consumer = "consumer_";
            fout<<(consumer.append(to_string(i)))<<"\t"<<"-1"<<"\t"<<to_string(i-2)<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\t"<<"NaN"<<"\n";
        }
        std::cout<<"After Router Section"<<std::endl;
        //-------------Links section-----------------------//
        fout<<"\n";
        fout<<"links"<<"\n"; //section links
        for(uint32_t i = 0; i<m_node_list.size();i++)
        {
            fout<<"orchestrator"<<"\t"<<(m_node_list.at(i).name)<<"\t"<<"100Mbps"<<"\t"<<"50"<<"\t"<<"10ms"<<"\t"<<"50"<<"\n";
        }

        for(uint32_t i=0; i<m_node_list.size()-1;i++)
        {
            std::string compute_node_1 = "compute_node_";
            std::string compute_node_2 = "compute_node_";
            fout<<compute_node_1.append(to_string(i+1))<<"\t"<<compute_node_2.append(to_string(i+2))<<"\t"<<"100Mbps"<<"\t"<<"0"<<"\t"<<"10ms"<<"\t"<<"50"<<"\n";
        }

        uint32_t q = num_consumer_nodes / (m_node_list.size());
        for(uint32_t i = 0;i<m_node_list.size();i++)
        {
            for(uint32_t j=0;j<q;j++)
            {
                std::string consumer = "consumer_";
                uint32_t consumer_suffix = ((j) + ((i)*(q)));
                fout<<(m_node_list.at(i).name)<<"\t"<<(consumer.append(to_string(consumer_suffix)))<<"\t"<<"100Mbps"<<"\t"<<"1"<<"\t"<<"10ms"<<"\t"<<"50"<<"\n";
            }
        }
        for( uint32_t i = 0; i<m_node_list.size();i++)
        {
            std::string consumer = "consumer_";
            uint32_t consumer_suffix = (i)+(q * (m_node_list.size()));
            if(consumer_suffix < num_consumer_nodes)
                fout<<(m_node_list.at(i).name)<<"\t"<<(consumer.append(to_string(consumer_suffix)))<<"\t"<<"100Mbps"<<"\t"<<"1"<<"\t"<<"10ms"<<"\t"<<"50"<<"\n";
            else
                break;
        }
        std::cout<<"After Links Section"<<std::endl;
        //-------------functions section--------------------//
        fout<<"\n";
        fout<<"functions"<<"\n"; //section functions
        generate_func_list(num_func, input_range);
        generate_provided_data(num_consumer_nodes);
        generate_start_time(num_consumer_nodes, sim_time);
        generate_stop_time(num_consumer_nodes, sim_time);

        for(uint32_t i=0;i<num_func;i++)
        {
            if(static_value == false)
            {
                m_func_list.at(i).ram=ram.at(rand()%3);
                m_func_list.at(i).rom=rom.at(rand()%3);
                m_func_list.at(i).cores=cores.at(rand()%3);
                m_func_list.at(i).exec_time = exec_time.at(rand()%6);
                m_func_list.at(i).func_size = data_packet_size.at(rand()%8);
                m_func_list.at(i).result_size = data_packet_size.at(rand()%5);
            }
            else
            {
                m_func_list.at(i).exec_time = "3.0"; //currently set to a constant vaue
                m_func_list.at(i).func_size = "2000"; //currently set to a constant value
                m_func_list.at(i).result_size ="500"; //currently set to a constant value
                m_func_list.at(i).cores="1";
                m_func_list.at(i).ram="2";
                m_func_list.at(i).rom="16";
            }
            fout<<(m_func_list.at(i).name)<<"\t"<<(m_func_list.at(i).input_signature)<<"\t"<<runtimes.at(0)<<"\t"<<m_func_list.at(i).ram<<"\t"<<m_func_list.at(i).rom<<"\t"<<m_func_list.at(i).cores<<"\t"<<m_func_list.at(i).exec_time
            <<"\t"<<m_func_list.at(i).func_size<<"\t"<<m_func_list.at(i).result_size<<"\n";
        }
        std::cout<<"After functions Section"<<std::endl;
        //-----------initial function placement section------//
        fout<<"\n";
        fout<<"initial function status"<<"\n"; // section intial placement to compute nodes
        //std::cout<<"initial function status"<<std::endl;
        int func_max = 2; // each func will be placed in max 5 different nodes
        int func_min = 1; // each func will be placed in min 1 node
        int random_value;
        for(uint32_t i=0;i<num_func;i++)
        {
            std::vector<int> rand_list;
            int func_repl_rand = rand()%(func_max-func_min+1)+func_min;
            random_value = m_node_list.size();
            for(int j = 0; j<func_repl_rand; j++)
            {
                do{
                    //random_value = rand()%m_node_list.size();
                    random_value--;
                    //random_value = m_node_list.size()-1;
                } while(
                   (stoi(m_node_list.at(random_value).cores) < stoi(m_func_list.at(i).cores))
                || (stoi(m_node_list.at(random_value).ram) < stoi(m_func_list.at(i).ram))
                || (stoi(m_node_list.at(random_value).rom) < stoi(m_func_list.at(i).rom))
                );
                if (std::find(rand_list.begin(), rand_list.end(), random_value) == rand_list.end())
                {
                    fout<<(m_node_list.at(random_value).name)<<"\t"<<(m_func_list.at(i).name)<<"\n";
                    rand_list.push_back(random_value);
                }
            }
        }
        std::cout<<"initial function placement done!"<<std::endl;

        //----------consumer section---------------------//
        fout<<"\n";
        fout<<"consumer section"<<"\n"; // define consumer request behavior

        std::vector<double> l_prob_vector = createZipfMandelbrotDist(num_func, 0.7, 0.7);
        generate_int_lifetime(num_func, frequency_range);
        //for(int k = 0; k < (m_node_list.size()/2); k++)
        //{
            for(uint32_t i=0;i<num_consumer_nodes;i++)
            {
                std::string consumer = "consumer_";
                int j;

                if(zipf==true)
                {
                    if(nfn_load_distribution_scenario == true)
                    {
                        j = ((i % (num_consumer_nodes/m_node_list.size())) ); //+ (2*k*(num_consumer_nodes/m_node_list.size())));
                    }
                    else
                    {
                        j = i;
                    }
                    uint32_t index = getNextZipfMandelbrotRandomNumber(l_prob_vector, num_func);
                    index = index-1;
                    string inputs = generate_input_list((m_func_list.at(index)).num_inputs, num_func);
                    fout<<(consumer.append(to_string(j)))<<"\t"<<(m_func_list.at(index)).name<<"\t"<<inputs<<"\t"<<m_freq.at(index)<<"\t"<<(m_func_list.at(index)).lifetime<<"\t"
                    <<m_startTime.at(j)<<"\t"<<m_stopTime.at(j)<<"\n";
                }
                else
                {
                    if(nfn_load_distribution_scenario == true)
                    {
                        j = i % (num_consumer_nodes/m_node_list.size());
                    }
                    else
                    {
                        j= i;
                    }
                    string inputs = generate_input_list((m_func_list.at(i)).num_inputs, num_func);
                    fout<<(consumer.append(to_string(j)))<<"\t"<<(m_func_list.at(j)).name<<"\t"<<inputs<<"\t"<<m_freq.at(j)<<"\t"<<(m_func_list.at(j)).lifetime<<"\t"
                    <<m_startTime.at(j)<<"\t"<<m_stopTime.at(j)<<"\n";
                }
            }
        //}
         //usual consumer request at other consumer nodes (connected to other compute nodes) during nfn_load_distribution_scenario.
         if(nfn_load_distribution_scenario == true)
        {
            for(uint32_t i = (num_consumer_nodes/m_node_list.size()); i<num_consumer_nodes; i++)
            {
                std::string consumer = "consumer_";
                if(zipf==true)
                {
                    uint32_t index = getNextZipfMandelbrotRandomNumber(l_prob_vector, num_func);
                    index = index-1;
                    string inputs = generate_input_list((m_func_list.at(index)).num_inputs, num_func);
                    fout<<(consumer.append(to_string(i)))<<"\t"<<(m_func_list.at(index)).name<<"\t"<<inputs<<"\t"<<m_freq.at(index)<<"\t"<<(m_func_list.at(index)).lifetime<<"\t"
                    <<m_startTime.at(i)<<"\t"<<m_stopTime.at(i)<<"\n";

                }
                else
                {
                    string inputs = generate_input_list((m_func_list.at(i)).num_inputs, num_func);
                    fout<<(consumer.append(to_string(i)))<<"\t"<<(m_func_list.at(i)).name<<"\t"<<inputs<<"\t"<<m_freq.at(i)<<"\t"<<(m_func_list.at(i)).lifetime<<"\t"
                    <<m_startTime.at(i)<<"\t"<<m_stopTime.at(i)<<"\n";
                }
            }
        }
        std::cout<<"consumer section done"<<std::endl;

        //------------initial data section---------------------//
        fout<<"\n";
        fout<<"initial data section"<<"\n";
        for(uint32_t i=0;i<num_consumer_nodes; i++)
        {
            std::string consumer = "consumer_";
            m_provided_data.at(i).data_size= "1000"; //currently set to a constant value
            m_provided_data.at(i).freshness = "0";
            fout<<consumer.append(to_string(i))<<"\t"<<(m_provided_data.at(i)).name<<"\t"<<m_provided_data.at(i).data_size<<"\t"<<m_provided_data.at(i).freshness<<"\n";
        }
        std::cout<<"initial data section done"<<std::endl;
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
DataGen::generate_input_list(int num_inputs, uint32_t num_consumer_nodes)
{
    int data_max = num_consumer_nodes/2;
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
        //m_func_list.at(i).lifetime =  to_string(std::stoi(m_func_list.at(i).exec_time)+5);
        m_func_list.at(i).lifetime = to_string(10);
        //int min = std::stoi(m_func_list.at(i).exec_time)+2;
        int min = 5;
        int max = frequency_range.second;
        m_freq.push_back(to_string(rand()%(max-min+1)+min));
    }
}

void
DataGen::generate_provided_data(uint32_t num_consumer_nodes)
{

    int min = 0; int max = num_consumer_nodes/2; //Number of different provided data
    //int max = 2;
    for(uint32_t i=0;i< num_consumer_nodes; i++)
    {
        dataProperties new_data;
        do{
            new_data.name = "Operand_";
            (new_data.name).append(to_string(rand()%(max-min+1)+min)+',');
            auto it = find_if(begin(m_provided_data), end(m_provided_data), [=] (dataProperties const& dp)
            {
             return (dp.name == new_data.name);
            });

            if((it == end(m_provided_data)) || (m_provided_data.size() >= ((max-min)+1)))
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
