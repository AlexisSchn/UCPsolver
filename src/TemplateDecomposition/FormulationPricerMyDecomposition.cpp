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
#include "MyDecomposition/FormulationPricerMyDecomposition.h"


//** Namespaces
using namespace std;



FormulationPricerMyDecomposition::FormulationPricerMyDecomposition(InstanceUCP *instance, 
    SCIP *scip,
    block_number
    ) :
    FormulationPricer( instance, scip ), 
    m_block_number( block_number )
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
SCIP_RETCODE FormulationPricerMyDecomposition::create_variables()
{
    ostringstream current_var_name;
    int number_of_time_steps = m_instance_ucp->get_time_steps_number();


    //* create the variable

    //! fill
    

    return( SCIP_OKAY );

}


/* create all the constraints, and add them to the scip object and formulation object */
SCIP_RETCODE FormulationPricerMyDecomposition::create_constraints()
{
    ostringstream current_cons_name;
    int number_of_time_steps = m_instance_ucp->get_time_steps_number();

    //! fill here, create the constraints

    return( SCIP_OKAY );
}


SCIP_RETCODE change_reduced_costs( 
    std::vector<std::vector<SCIP_Real>> reduced_costs_somthing
)
{

    double coefficient( 0. );
    // modify coefficient
    SCIP_CALL(SCIPchgVarObj(m_scip_pricer, variable, coefficient));
    
    return( SCIP_OKAY );
}



ProductionPlan* FormulationPricerMyDecomposition::get_production_plan_from_solution()
{
    SCIP_SOL *solution = SCIPgetBestSol( m_scip_pricer );
    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());

    // We start by creating an empty plan
    vector< vector < double > > up_down_plan;
    vector< vector < double > > switch_plan;
    vector< vector < double > > quantity_plan;
    up_down_plan.resize(number_of_units);
    switch_plan.resize(number_of_units);
    quantity_plan.resize(number_of_units);

    // create the right plan for the solution

    //! fill

    // create the plan object

    ProductionPlan* new_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    new_plan->compute_cost();

    return( new_plan );
}





// * gets 


int FormulationPricerMyDecomposition::get_unit_number()
{
    return( m_unit_number );
}

