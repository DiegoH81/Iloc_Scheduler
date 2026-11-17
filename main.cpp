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
    bool rename = false, backward = false, use_rbf = false, random_ties = false;

    for (int i = 2; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-r")
            rename = true;
        if (arg == "-b")
            backward = true;
        if (arg == "-rbf")
            use_rbf = true;
        if (arg == "-rand")
            random_ties = true;
    }

    std::stringstream buffer;
    buffer << std::cin.rdbuf();

    InstructionManager manager(".", units, rename, random_ties);

    if (use_rbf)
        manager.rbf(buffer.str());
    else if (backward)
        manager.schedule_backwards(buffer.str());
    else
        manager.schedule_normal(buffer.str());

    manager.print();
}