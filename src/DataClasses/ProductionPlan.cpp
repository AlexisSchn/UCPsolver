
/**
 *  @file ProductionPlan.cpp
 *  Implement the class ProductionPlan 
*/



//** Includes

//* Standart
#include <vector>
#include <iostream>


//* SCIP
#include <scip/scipdefplugins.h>
#include <scip/scip.h>


//* User

// general
#include "DataClasses/InstanceUCP.h"
#include "DataClasses/ProductionPlan.h"

//** Namespaces
using namespace std; 





ProductionPlan::ProductionPlan( InstanceUCP* instance_ucp, 
        std::vector < std::vector  < double > > up_down_plan, 
        std::vector < std::vector  < double > > switch_plan, 
        std::vector < std::vector  < double > > quantity_plan 
    )
{
    m_instance_ucp = instance_ucp;
    m_up_down_plan = up_down_plan;
    m_switch_plan = switch_plan;
    m_quantity_plan = quantity_plan;
    compute_cost();
}




ProductionPlan::~ProductionPlan()
{
}


void ProductionPlan::compute_cost()
{
    m_cost = 0;

    int number_units( m_instance_ucp-> get_units_number() );
    int number_time_steps( m_instance_ucp-> get_time_steps_number() );
    vector<int> costs_fixed( m_instance_ucp->get_costs_fixed() );
    vector<int> costs_proportionnal( m_instance_ucp->get_costs_proportionnal() );
    vector<int> costs_startup( m_instance_ucp->get_costs_startup() );

    for( int i_unit = 0; i_unit < number_units; i_unit ++)
    {
        for( int i_time_step = 0; i_time_step < number_time_steps; i_time_step ++)
        {
            m_cost += m_up_down_plan[i_unit][i_time_step] * costs_fixed[i_unit] ;
            m_cost += m_switch_plan[i_unit][i_time_step] * costs_startup[i_unit] ;
            m_cost += m_quantity_plan[i_unit][i_time_step] * costs_proportionnal[i_unit] ;
        }
    }

    return;
}


void ProductionPlan::show()
{
    cout << "Printing the production plan : ";
    
    int number_units( m_instance_ucp-> get_units_number() );
    int number_time_steps( m_instance_ucp-> get_time_steps_number() );

    for( int i_unit = 0; i_unit < number_units; i_unit ++)
    {
        cout << "\nUnit " << i_unit << " :";
        for( int i_time_step = 0; i_time_step < number_time_steps; i_time_step++ )
        {
            cout << " " << m_up_down_plan[i_unit][i_time_step];
        }
    }

    cout << "\nSwitch plan : " ;
    for( int i_unit = 0; i_unit < number_units; i_unit ++)
    {
        cout << "\nUnit " << i_unit << " :";
        for( int i_time_step = 0; i_time_step < number_time_steps; i_time_step++ )
        {
            cout << " " << m_switch_plan[i_unit][i_time_step];
        }
    }

    cout << "\nProduction values : ";
    for( int i_unit = 0; i_unit < number_units; i_unit ++)
    {
        cout << "\nUnit " << i_unit << " :";
        for( int i_time_step = 0; i_time_step < number_time_steps; i_time_step++ )
        {
            cout << " " << m_quantity_plan[i_unit][i_time_step];
        }
    }

    cout << "\nCost of the plan : " << m_cost << endl;

}


//* Gets


double ProductionPlan::get_cost()
{
    return( m_cost );
}


vector< vector< double > > ProductionPlan::get_up_down_plan()
{
    return( m_up_down_plan );
}


vector< vector< double > > ProductionPlan::get_switch_plan()
{
    return( m_switch_plan );
}


vector< vector< double > > ProductionPlan::get_quantity_plan()
{
    return( m_quantity_plan );
}



