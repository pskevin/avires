#include "../models/event.h"
#include "../models/address.h"
#include "./access.h"

#include <iostream>
#include <fstream>
#include <string>

using std::cerr;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

namespace Event
{
    void Pool::WriteAll(string dir)
    {
        LogPoint();
        WriteAddress(dir);
        // Open all files
        Type *type;
        for (auto const &tpair : types_)
        {
            type = tpair.second;
            LogMessage("Type %s", type->ID());
            LogMessage("Count %lld", type->count);

            // Vector of sources in order of registration
            vector<string> sources(type->sources_.size());
            for (auto const &spair : type->sources_)
            {
                sources[spair.second] = spair.first;
            }
            
            string file_name = dir + "/" + type->ID() + ".out";
            ofstream file(file_name.c_str(), ios::out | ios::trunc);
            if (file.is_open())
            {
                for (uint32_t i = 0; i < sources.size(); i++)
                {
                    LogMessage("Source %s", sources[i]);
                    if (i != 0)
                        file << ",";
                    file << sources[i];
                }
                file << endl;

                LogMessage("Size of data %lld", type->data_.size());
                for (auto line : type->data_)
                {
                    for (uint32_t i = 0; i < sources.size(); i++) // line.size <= sources.size
                    {
                        auto lpair = line->find(i); // i is the index mapping source to data

                        if (i != 0)
                            file << ",";
                        if (lpair != line->end())
                            file << lpair->second;
                    }
                    file << endl;
                }
            }
            else
            {
                cerr << "Cannot open file " << file_name << endl;
            }
            file.close();
            LogPoint();
        }
    }

    void Pool::WriteAddress(string dir)
    {
        ofstream file((dir + "/addrs.out").c_str(), ios::out | ios::trunc);
        file << "vaddr,paddr" << endl;
        for (Address *addr : addrs_)
        {
            file << addr->Virtual() << "," << addr->Physical() << endl;
        }
        file.close();
    }
}