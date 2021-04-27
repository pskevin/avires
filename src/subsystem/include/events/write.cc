#include "../models/event.h"
#include "../models/address.h"
#include "./access.h"

#include <iostream>
#include <fstream>
#include <string>

using std::endl;
using std::ios;
using std::ofstream;
using std::string;

namespace Event
{
    void Pool::WriteAll(string file_prefix)
    {
        LogPoint();
        WriteAddress(file_prefix);
        // Open all files
        Type *type;
        for (auto const &tpair : types_)
        {
            type = tpair.second;
            LogMessage("Type %s", type->ID());

            // if(type->ID() == "access")
            // {
                LogMessage("Count %lld", type->count);
            // }

            // Vector of sources in order of registration
            vector<string> sources(type->sources_.size());
            for (auto const &spair : type->sources_)
            {
                sources[spair.second] = spair.first;
            }

            ofstream file((file_prefix + "_" + type->ID() + ".out").c_str(), ios::out | ios::trunc);
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
            file.close();
            LogPoint();
        }
    }

    void Pool::WriteAddress(string file_prefix)
    {
        ofstream file((file_prefix + "_addrs.out").c_str(), ios::out | ios::trunc);
        file << "vaddr,paddr" << endl;
        for (Address *addr : addrs_)
        {
            file << addr->Virtual() << "," << addr->Physical() << endl;
        }
        file.close();
    }
}