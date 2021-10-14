/**
 * @file
 * 
*/

//** Includes

//* Standart
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>


//* SCIP
#include <scip/scipdefplugins.h>
#include <scip/scip.h>


//* User

// general
#include "DataClasses/InstanceUCP.h"
#include "DataClasses/ProductionPlan.h"

// Decomposition
#include "Decomposition/FormulationPricer.h"

// Unit Decomposition
#include "TimeDecomposition2pmax/FormulationPricerTimeDecomposition2pmax.h"


//** Namespaces
using namespace std;

 
 
FormulationPricerTimeDecomposition2pmax::FormulationPricerTimeDecomposition2pmax(InstanceUCP *instance, 
    SCIP *scip,
    int block_number,
    vector<vector<SCIP_Real>> reduced_costs_switch,
    vector<vector<SCIP_Real>> reduced_costs_min_uptime,
    vector<vector<SCIP_Real>> reduced_costs_min_downtime,
    vector<vector<SCIP_Real>> reduced_costs_pmaxpmin,
    vector<SCIP_Real> reduced_costs_demand
    ) :
    FormulationPricer( instance, scip ),
    m_block_number(block_number),
    m_reduced_costs_switch( reduced_costs_switch ),
    m_reduced_costs_min_uptime( reduced_costs_min_uptime ),
    m_reduced_costs_min_downtime( reduced_costs_min_downtime ),
    m_reduced_costs_pmaxpmin( reduced_costs_pmaxpmin),
    m_reduced_costs_demand( reduced_costs_demand )
{
    //* create the variables
    SCIP_RETCODE retcode(SCIP_OKAY);
    retcode = create_variables();
    if ( retcode != SCIP_OKAY)
    {
        SCIPprintError( retcode );
    }

    //* create the constraints
    retcode = create_constraints();
    if ( retcode != SCIP_OKAY)
    {
        SCIPprintError( retcode );
    }

}


/* create all the variable and add them to the object */
SCIP_RETCODE FormulationPricerTimeDecomposition2pmax::create_variables()
{
    ostringstream current_var_name;
    int number_of_time_steps = m_instance_ucp->get_time_steps_number();
    int number_of_units = m_instance_ucp->get_units_number();

    // Variables x
    vector<int> fixed_costs( m_instance_ucp->get_costs_fixed() ); 
    vector<int> proportionnal_costs( m_instance_ucp->get_costs_proportionnal());
    vector<int> min_uptime( m_instance_ucp->get_min_uptime() );
    vector<int> min_downtime( m_instance_ucp->get_min_downtime() );
    vector<int> production_max( m_instance_ucp->get_production_max() );
    vector<int> production_min( m_instance_ucp->get_production_min() );


    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        current_var_name.str("");
        current_var_name << "x_" << i_unit;
        SCIP_VAR* variable_x_i_t;
 
        // coefficient calculation
        double coefficient( 0. );
        coefficient += fixed_costs[i_unit];
 
        // switch reduced costs
        coefficient -= m_reduced_costs_switch[i_unit][m_block_number];
        if( m_block_number < number_of_time_steps -1 )
        {
            coefficient += m_reduced_costs_switch[i_unit][m_block_number + 1];
        }
 
        // min uptime / downtime reduced costs
        if( m_block_number >= min_uptime[i_unit] - 1)
        {
            coefficient += m_reduced_costs_min_uptime[i_unit][m_block_number - min_uptime[i_unit] + 1];
        }
        if( m_block_number <= number_of_time_steps - min_downtime[i_unit] - 1 )
        {
            coefficient -= m_reduced_costs_min_downtime[i_unit][m_block_number];
        }

        // Pmin Pmax reduced costs
        coefficient -= ( production_max[i_unit] - production_min[i_unit] ) * m_reduced_costs_pmaxpmin[i_unit][m_block_number];
        
        // demand reduced costs
        coefficient -= production_max[i_unit] * m_reduced_costs_demand[m_block_number];

        // Pmax or Pmin Cost
        coefficient += production_max[i_unit] * proportionnal_costs[i_unit];
        // coefficient += production_max[i_unit] * proportionnal_costs[i_unit];

    
        // create the variable
        SCIPcreateVarBasic(  m_scip_pricer,
            &variable_x_i_t,                    // pointer 
            current_var_name.str().c_str(),     // name
            0.,                                 // lowerbound
            1.,                                 // upperbound
            coefficient ,                       // coeff in obj function
            SCIP_VARTYPE_BINARY                 // type
        );
        SCIP_CALL(SCIPaddVar(m_scip_pricer, variable_x_i_t));
        m_variables_x.push_back(variable_x_i_t);
    }

    
    return( SCIP_OKAY );

}


/* create all the constraints, and add them to the scip object and formulation object */
SCIP_RETCODE FormulationPricerTimeDecomposition2pmax::create_constraints()
{
    ostringstream current_cons_name;
    int number_of_time_steps = m_instance_ucp->get_time_steps_number();
    int number_of_units = m_instance_ucp->get_units_number();
    vector<int> production_max = m_instance_ucp->get_production_max();
    //* demand constraint
    SCIP_CONS* cons_demand_t;
    current_cons_name.str("");
    current_cons_name << "cons_demand";
    // creating the constraint
    SCIP_CALL(SCIPcreateConsBasicLinear(m_scip_pricer, 
        &cons_demand_t,                     /* constraint pointer */ 
        current_cons_name.str().c_str(),    /* constraint name */
        0,                                  /* number of variable added */
        nullptr,                            /* array of variable */
        nullptr,                            /* array of coefficient */
        m_instance_ucp->get_demand()[m_block_number],         /* LHS */
        +SCIPinfinity(m_scip_pricer)));              /* RHS */        
    // adding the variable
    for(int i_unit = 0; i_unit < number_of_units; i_unit++)
    {
        SCIP_CALL( SCIPaddCoefLinear(m_scip_pricer,
            cons_demand_t,
            m_variables_x[i_unit],  /* variable to add */
            production_max[i_unit]));                               /* coefficient */
    }
    SCIP_CALL( SCIPaddCons(m_scip_pricer, cons_demand_t));
    m_constraints_demand = cons_demand_t;
    


    return( SCIP_OKAY );
}


ProductionPlan* FormulationPricerTimeDecomposition2pmax::get_production_plan_from_solution()
{
    SCIP_SOL *solution = SCIPgetBestSol( m_scip_pricer );
    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    vector<int> production_max( m_instance_ucp->get_production_max() );
    vector<int> production_min( m_instance_ucp->get_production_min() );

    // We start by creating an empty plan
    vector< vector < double > > up_down_plan;
    vector< vector < double > > switch_plan;
    vector< vector < double > > quantity_plan;
    up_down_plan.resize(number_of_units);
    switch_plan.resize(number_of_units);
    quantity_plan.resize(number_of_units);

    // create the right plan for the solution
 
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
 
        up_down_plan[i_unit].resize(number_of_time_steps, 0);
        switch_plan[i_unit].resize(number_of_time_steps, 0);
        quantity_plan[i_unit].resize(number_of_time_steps, 0);

        up_down_plan[i_unit][m_block_number] += SCIPgetSolVal( m_scip_pricer, solution, m_variables_x[i_unit]);
        quantity_plan[i_unit][m_block_number] += production_max[i_unit] * up_down_plan[i_unit][m_block_number];
        // quantity_plan[i_unit][m_block_number] += production_max * up_down_plan[i_unit][m_block_number];
    }

    // create the plan object
 
    ProductionPlan* new_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    new_plan->compute_cost();

    return( new_plan );
}

 

// * gets 


int FormulationPricerTimeDecomposition2pmax::get_unit_number()
{
    return( m_block_number );
}

