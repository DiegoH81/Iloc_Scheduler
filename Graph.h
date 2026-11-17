#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

using Data = std::pair<int, int>;

class Graph
{
public:
	std::vector<std::vector<Data>> predecessors, successors;

	Graph() :
		predecessors(),
		successors()
	{}

	Graph(int in_size) :
		predecessors(in_size, std::vector<Data>()),
		successors(in_size, std::vector<Data>()),
		size(in_size)
	{}

	void set_size(int in_size)
	{
		predecessors = std::vector<std::vector<Data>>(in_size, std::vector<Data>());
		successors = std::vector<std::vector<Data>>(in_size, std::vector<Data>());
	}

	void add_edge(int in_source, int in_dest, int in_delay)
	{
		successors[in_source].push_back({ in_dest, in_delay });
		predecessors[in_dest].push_back({ in_source, in_delay });
	}

	void clear()
	{
		if (size > 0)
		{
			successors.clear();
			predecessors.clear();
			set_size(size);
		}
	}
private:
	int size;
};


#endif