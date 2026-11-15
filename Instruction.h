#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>
#include <vector>
#include <algorithm>
#include <ostream>

enum Type
{
    loadI,
    loadAI,
    storeAI,
    add,
    sub,
    mult,
    lshift,
    rshift
};

int get_latency(Type in_type)
{
    switch(in_type)
    {
        case Type::loadI:
        case Type::add:
        case Type::sub:
        case Type::lshift:
        case Type::rshift:
            return 1;
        case Type::mult:
            return 2;
        case Type::loadAI:
        case Type::storeAI:
            return 3;
        default:
            return -1;
    }
}

Type get_type(const std::string &in_type)
{
    if (in_type == "loadI")
        return Type::loadI;
    else if (in_type == "loadAI")
        return Type::loadAI;
    else if (in_type == "storeAI")
        return Type::storeAI;
    else if (in_type == "add")
        return Type::add;
    else if (in_type == "sub")
        return Type::sub;
    else if (in_type == "mult")
        return Type::mult;
    else if (in_type == "lshift")
        return Type::lshift;
    else if (in_type == "rshift")
        return Type::rshift;

}

inline bool is_register(const std::string& in_string)
{
    return in_string.size() > 1 && in_string[0] == 'r';
}

class Instruction
{
public:
    std::vector<std::string> reads, writes;
    std::vector<std::string> left, right;
    std::string base, offset, opcode;
    Type type;
    int start, latency;


    Instruction(std::string in_type,
                const std::vector<std::string>& in_left,
                const std::vector<std::string>& in_right,
                const int& in_start):
                opcode(in_type),
                left(in_left), right(in_right),
                type(get_type(in_type)),
                start(in_start), latency(get_latency(type))
    {
        switch (type)
        {
        case Type::loadI:
            writes.push_back(right[0]);
            break;

        case Type::loadAI:
            base = left[0];
            offset = left[1];
            add_read(left[0]);
            writes.push_back(right[0]);
            break;

        case Type::storeAI:
            base = right[0];
            offset = right[1];
            add_read(left[0]);
            add_read(right[0]);
            break;

        default:
            for (auto& s : left)
                if (is_register(s))
                    add_read(s);
            writes.push_back(right[0]);
            break;
        }
    }

    // Helper
    void add_read(const std::string& in_register)
    {
        if (std::find(reads.begin(), reads.end(), in_register) == reads.end())
            reads.push_back(in_register);
    }

    bool is_load() const { return type == Type::loadAI; }
    bool is_store() const { return type == Type::storeAI; }
    bool is_memory() const { return is_load() || is_store(); }


};

inline std::ostream& operator<<(std::ostream& os, const Instruction& in_ins)
{
    os << in_ins.opcode << " ";

    for (int i = 0; i < in_ins.left.size(); i++)
    {
        if (i > 0)
            os << ",";
        os << in_ins.left[i];
    }

    if (!in_ins.right.empty())
    {
        os << " => ";
        for (int i = 0; i < in_ins.right.size(); i++)
        {
            if (i > 0)
                os << ",";
            os << in_ins.right[i];
        }
    }

    return os;
}

bool memory_conflict(const Instruction& I, const Instruction& J)
{
    if (I.base != J.base)
        return true;
    return I.offset == J.offset;
}

#endif