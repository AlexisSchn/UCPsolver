/**
 * @file 
 *  Implementation of the Instance class
*/

//** incudes

//* standart 
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

//* scip

//* user includes 

// data
#include "InstanceUCP.h"


//** namespaces
using namespace std;



InstanceUCP::InstanceUCP(char* filename)
{
    int error_code;
    error_code = read_data_from_file(filename);
    if(error_code != 0)
    {
        cerr << "Unable to read the data :/" << endl;
    }
}


InstanceUCP::~InstanceUCP()
{
}



int InstanceUCP::read_data_from_file(char *filename)
{   


    ifstream file(filename);
    if (!file)
    {
        return 1;
    }

    /* we go over the whole file, word by word */
    string key;
    int value;
    string dummy;

    while(file)
    {
        file >> key;

        if(key == "n")
        {
            file >> dummy;
            file >> m_units_number;
        }

        else if( key == "T")
        {
            file >> dummy;
            file >> m_time_steps_number;
        }

        else if( key == "c0")
        {
            file >> dummy;
            file >> dummy;
            for(int i_unit = 0; i_unit < m_units_number; i_unit ++)
            {
                file >> value;
                m_costs_startup.push_back(value);
            }
            file >> dummy;
        }

        else if( key == "cf")
        {
            file >> dummy;
            file >> dummy;
            for(int i_unit = 0; i_unit < m_units_number; i_unit ++)
            {
                file >> value;
                m_costs_fixed.push_back(value);
            }
            file >> dummy;
        }

        else if( key == "cp")
        {
            file >> dummy;
            file >> dummy;
            for(int i_unit = 0; i_unit < m_units_number; i_unit ++)
            {
                file >> value;
                m_costs_proportionnal.push_back(value);
            }
            file >> dummy;
        }

        else if( key == "Init")
        {
            file >> dummy;
            file >> dummy;
            for(int i_unit = 0; i_unit < m_units_number; i_unit ++)
            {
                file >> value;
                m_initial_state.push_back(value);
            }
        }

        else if( key == "D")
        {
            file >> dummy;
            file >> dummy;
            for(int i_time = 0; i_time < m_time_steps_number; i_time++)
            {
                file >> value;
                m_demand.push_back(value);
            }
            file >> dummy;
        }

        else if( key == "Pmin")
        {
            file >> dummy;
            file >> dummy;
            for(int i_unit = 0; i_unit < m_units_number; i_unit ++)
            {
                file >> value;
                m_production_min.push_back(value);
            }
            file >> dummy;
        }
        
        else if( key == "Pmax")
        {
            file >> dummy;
            file >> dummy;
            for(int i_unit = 0; i_unit < m_units_number; i_unit ++)
            {
                file >> value;
                m_production_max.push_back(value);
            }
            file >> dummy;
        }

        else if( key == "L")
        {
            file >> dummy;
            file >> dummy;
            for(int i_unit = 0; i_unit < m_units_number; i_unit ++)
            {
                file >> value;
                m_min_uptime.push_back(value);
            }
            file >> dummy;
        }

        else if( key == "l")
        {
            file >> dummy;
            file >> dummy;
            for(int i_unit = 0; i_unit < m_units_number; i_unit ++)
            {
                file >> value;
                m_min_downtime.push_back(value);
            }
            file >> dummy;
        }

        else
        {
            getline(file, dummy);
        }

    }

    return 0;
}



void InstanceUCP::print_instance()
{
    cout << "Short printing of the instance... \n";
    cout << "Number of units : " << m_units_number << "\n";
    cout << "Number of time steps : " << m_time_steps_number << "\n";
}




//* gets


int InstanceUCP::get_units_number()
{
    return(m_units_number);
}

int InstanceUCP::get_time_steps_number()
{
    return(m_time_steps_number);
}

vector<int> InstanceUCP::get_costs_fixed()
{
    return(m_costs_fixed);
}

vector<int> InstanceUCP::get_costs_proportionnal()
{
    return(m_costs_proportionnal);
}

vector<int> InstanceUCP::get_costs_startup()
{
    return(m_costs_startup);
}

vector<int> InstanceUCP::get_demand()
{
    return(m_demand);
}

vector<int> InstanceUCP::get_production_max()
{
    return(m_production_max);
}

vector<int> InstanceUCP::get_production_min()
{
    return(m_production_min);
}

vector<int> InstanceUCP::get_min_downtime()
{
    return(m_min_downtime);
}

vector<int> InstanceUCP::get_min_uptime()
{
    return(m_min_uptime);
}

vector<int> InstanceUCP::get_initial_state()
{
    return(m_initial_state);
}