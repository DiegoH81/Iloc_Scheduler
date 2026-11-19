#ifndef INSTRUCTION_MANAGER_H
#define INSTRUCTION_MANAGER_H

#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <unordered_map>
#include <random>
#include <limits>

#include "Instruction.h"
#include "FileReader.h"
#include "Helpers.h"
#include "Graph.h"

class ByPriority
{
public:
	ByPriority(std::vector<int>& in_priority)
		: priority(in_priority)
	{}

	bool operator()(int a, int b)
	{
		if ((priority[a] != priority[b]))
			return priority[a] > priority[b];

		return a < b;
	}

private:
	const std::vector<int>& priority;
	bool only_prio;
};


class InstructionManager
{
public:
	InstructionManager(std::filesystem::path in_path, int in_units, bool in_rename = false, bool in_random_ties = false):
		reader(in_path), units(in_units), instructions(), graph(),
		priority(), sorted_instructions(), next_register(1000),
		rename(in_rename), random_ties(in_random_ties),
		rng( std::random_device{}() )
	{}

	void schedule_normal(const std::string& in_data)
	{
		load_iloc(in_data);
		build_graph();
		compute_priority();
		schedule();
	}

	void schedule_backwards(const std::string& in_data)
	{
		load_iloc(in_data);
		build_graph();
		compute_priority_backward();
		schedule_backward();
	}

	void rbf(const std::string& in_data, int repetitions = 5)
	{
		random_ties = true;
		int min_length = std::numeric_limits<int>::max();

		std::vector<std::vector<int>> best_sorted;
		std::vector<int> best_start;

		for (int i = 0; i < (repetitions * 2); i++)
		{
			if (i % 2 == 0)
				schedule_normal(in_data);
			else
				schedule_backwards(in_data);

			int new_length = get_length();
			if (new_length < min_length)
			{
				min_length = new_length;
				best_sorted = sorted_instructions;

				best_start.clear();
				for (auto& ins : instructions)
					best_start.push_back(ins.start);
			}
		}

		sorted_instructions = best_sorted;
		for (int i = 0; i < instructions.size(); i++)
			instructions[i].start = best_start[i];
	}


	void print()
	{
        int length = get_length();

		for (int i = 0; i < length; i++)
		{
			std::cout << i + 1 << " :\t";
			if ((i >= sorted_instructions.size()) || sorted_instructions[i].empty())
				std::cout << "nop";
			else
			{
				for (int j = 0; j < sorted_instructions[i].size(); j++)
				{
					if (j > 0)
						std::cout << "; ";

					int idx = sorted_instructions[i][j];

					std::cout << instructions[idx];
				}
			}
			std::cout << "\n";
		}


		std::cout << "length: " << length << " cycles\n";
	}

private:
	Graph graph;
	FileReader reader;
	std::mt19937 rng;
	int units, next_register;
	bool rename, random_ties;
	std::vector<Instruction> instructions;
	std::vector<int> priority;
	std::vector<std::vector<int>> sorted_instructions;
	std::unordered_map<std::string, std::string> current_name;

	std::string get_name(const std::string& in_register)
	{
		if (!is_register(in_register) || !current_name.count(in_register))
			return in_register;

		return current_name[in_register];
	}

	void rename_registers(const std::string& op_code,
						  std::vector<std::string>& source,
						  std::vector<std::string>& dest)
	{
		for (auto& s : source)
			s = get_name(s);

		if (op_code == "storeAI")
		{
			dest[0] = get_name(dest[0]);
			return;
		}

		for (auto& d : dest)
		{
			if (d == "r0")
				continue;

			std::string new_name = "r" + std::to_string(next_register++);
			current_name[d] = new_name;
			d = new_name;
		}
	}

	void reset_instructions()
	{
		for (auto& i : instructions)
			i.start = -1;

		sorted_instructions.clear();
	}

	int get_length()
	{
		int to_return = 0;
		for (auto& ins : instructions)
			to_return = std::max(to_return, ins.start + ins.latency);

		return to_return - 1;
	}

	// Algorithm methods
	void load_iloc(const std::string& in_data)
	{
		if (!instructions.empty())
		{
			reset_instructions();
			return;
		}

		auto lines = help::get_lines_from_string(in_data);

		for (auto& l : lines)
		{
			help::replace(l, ',', ' ');
			auto tokens = help::parse_string(l);

			if (tokens.empty())
				continue;

			std::vector<std::string> source, dest;

			bool arrow_checked = false;
			for (int i = 1; i < tokens.size(); i++)
			{
				if (tokens[i] == "=>")
				{
					arrow_checked = true;
					continue;
				}

				if (!arrow_checked)
					source.push_back(tokens[i]);
				else
					dest.push_back(tokens[i]);
			}

			if (rename)
				rename_registers(tokens[0], source, dest);

			instructions.push_back(Instruction(tokens[0], source, dest, -1));
		}
	}

	void build_graph()
	{
		int n = instructions.size();
		graph.set_size(n);

		for (int j = 0; j < n; j++)
		{
			for (int i = j - 1; i >= 0; i--)
			{
				auto& I = instructions[i];
				auto& J = instructions[j];

				// Verdadera
				for (auto& ins : J.reads)
					if (std::find(I.writes.begin(), I.writes.end(), ins) != I.writes.end())
						graph.add_edge(i, j, I.latency);

				// Anti y salida
				for (auto& ins : J.writes)
				{
					if (std::find(I.reads.begin(), I.reads.end(), ins) != I.reads.end())
						graph.add_edge(i, j, 1);

					if (std::find(I.writes.begin(), I.writes.end(), ins) != I.writes.end())
						graph.add_edge(i, j, I.latency);
				}

				// Memoria
				if (I.is_memory() && J.is_memory())
				{
					if (I.is_load() && J.is_load())
						continue;

					if (memory_conflict(I, J))
					{
						if (I.is_store())
							graph.add_edge(i, j, I.latency);
						else
							graph.add_edge(i, j, 1);
					}
				}
			}
		}

	}

	void compute_priority()
	{
		int n = instructions.size();
		priority.assign(n, 0);

		for (int i = n - 1; i >= 0; i--)
		{
			int best = 0;
			for (auto& s : graph.successors[i])
				best = std::max(best, priority[s.first]);

			priority[i] = instructions[i].latency + best;
		}
	}

	void compute_priority_backward()
	{
		int n = instructions.size();
		priority.assign(n, 0);

		for (int i = 0; i < n; i++)
		{
			int best = 0;
			for (auto& p : graph.predecessors[i])
				best = std::max(best, priority[p.first]);

			priority[i] = instructions[i].latency + best;
		}
	}

	void schedule()
	{
		int cycle = 1;
		int counter = 0;
		int n = instructions.size();

		while (counter < n)
		{
			std::vector<int> ready;

			for (int i = 0; i < n; i++)
			{
				if (instructions[i].start != -1) // Already scheduled
					continue;

				bool condition_met = true;
				for (auto& p : graph.predecessors[i])
				{
					int p_start = instructions[p.first].start;
					if ((p_start == -1) || (p_start + p.second > cycle))
					{
						condition_met = false;
						break;
					}
				}

				if (condition_met)
					ready.push_back(i);
			}

			// Sort by priority			
			std::sort(ready.begin(), ready.end(), ByPriority(priority));

			if (random_ties)
			{
				int i = 0;
				while (i < ready.size())
				{
					int j = i;
					while (j < ready.size() && priority[ready[j]] == priority[ready[i]])
						j++;

					std::shuffle(ready.begin() + i, ready.begin() + j, rng);
					i = j;
				}
			}

			std::vector<int> temp;
			for (int k = 0; k < ready.size() && k < units; k++)
			{
				int r = ready[k];

				instructions[r].start = cycle;
				temp.push_back(r);
				counter++;
			}

			sorted_instructions.push_back(temp);
			cycle++;
		}
	}

	void schedule_backward()
	{
		int cycle = 1;
		int counter = 0;
		int n = instructions.size();

		while (counter < n)
		{
			std::vector<int> ready;

			for (int i = 0; i < n; i++)
			{
				if (instructions[i].start != -1) // Already scheduled
					continue;

				if (instructions[i].latency > cycle)
					continue;

				bool condition_met = true;
				for (auto& s : graph.successors[i])
				{
					int s_start = instructions[s.first].start;
					if ((s_start == -1) || (s_start + s.second > cycle))
					{
						condition_met = false;
						break;
					}
				}

				if (condition_met)
					ready.push_back(i);
			}

			// Sort by priority
			std::sort(ready.begin(), ready.end(), ByPriority(priority));
			if (random_ties)
			{
				int i = 0;
				while (i < ready.size())
				{
					int j = i;
					while (j < ready.size() && priority[ready[j]] == priority[ready[i]])
						j++;

					std::shuffle(ready.begin() + i, ready.begin() + j, rng);
					i = j;
				}
			}

			std::vector<int> temp;
			for (int k = 0; k < ready.size() && k < units; k++)
			{
				int r = ready[k];

				instructions[r].start = cycle;
				temp.push_back(r);
				counter++;
			}

			sorted_instructions.push_back(temp);
			cycle++;
		}

		int total = sorted_instructions.size();
		for (auto& ins : instructions)
			ins.start = total - ins.start + 1;

		std::reverse(sorted_instructions.begin(), sorted_instructions.end());

		while (!sorted_instructions.empty() && sorted_instructions.back().empty())
			sorted_instructions.pop_back();
	}

};

#endif