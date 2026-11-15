#include <iostream>
#include <sstream>
#include <string>
#include "InstructionManager.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Use: ./sched < # units > < Iloc code > \n";
        return 1;
    }

    int units = std::stoi(argv[1]);

    std::stringstream buffer;
    buffer << std::cin.rdbuf();

    InstructionManager manager(".", units);
    manager.load_iloc(buffer.str());
    manager.build_graph();
    manager.compute_priority();
    manager.schedule();
    manager.print();
}