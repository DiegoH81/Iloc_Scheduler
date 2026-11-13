#ifndef INSTRUCTION_H
#endif INSTRUCTION_H

#include <string>
#include <vector>

enum Types
{
    loadI,
    loadAI,
    storeAI,
    add,
    sub,
    mult,
    lshift,
    rshift,
    nop
}

int get_latency(Types in_type)
{
    switch(in_type)
    {
        case Types::loadI:
        case Types::add:
        case Types::sub:
        case Types::lshift:
        case Types::rshift:
        case Types::nop:
            return 1;
        case Types::mult:
            return 2;
        case Types::loadAI:
        case Types::storeAI:
            return 3;
        case default:
            return -1;
    }
}

class Instruction
{
public:
    Instruction(const std::vector<std::string>& in_data,
                const Types in_type, const int& in_start):
                data(in_data), start(in_start), duration(get_latency(in_type))
    {}

private:
    std::vector<std::string> data;
    int start, duration;
};

#endif