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
#include "OverlappingDecomposition/FormulationPricerOverlappingDecompositionTime.h"


//** Namespaces
using namespace std;

 

FormulationPricerOverlappingDecompositionTime::FormulationPricerOverlappingDecompositionTime(InstanceUCP *instance, 
        SCIP *scip,
        int block_number,
        double coefficient_repartition_fixed,
        double coefficient_repartition_prop
    ) : FormulationPricer(instance, scip),
    m_block_number( block_number ),
    m_coefficient_repartition_fixed( coefficient_repartition_fixed ),
    m_coefficient_repartition_prop( coefficient_repartition_prop )
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
SCIP_RETCODE FormulationPricerOverlappingDecompositionTime::create_variables()
{
    ostringstream current_var_name;
    int number_of_time_steps = m_instance_ucp->get_time_steps_number();
    int number_of_units = m_instance_ucp->get_units_number();
 
    // Variables x
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        current_var_name.str("");
        current_var_name << "x_" << i_unit;
        SCIP_VAR* variable_x_i_t;
 
        // coefficient calculation
        double coefficient( 0. );

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

    // Variables p

    vector<int> production_max( m_instance_ucp->get_production_max() );
    vector<int> costs_prop( m_instance_ucp->get_costs_proportionnal() );

    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIP_VAR* variable_p_i_t;
        current_var_name.str("");
        current_var_name << "p_" << i_unit << "_" << m_block_number ;

        // Creation of the variable
        SCIP_CALL( SCIPcreateVarBasic( m_scip_pricer,
            &variable_p_i_t,                // pointer
            current_var_name.str().c_str(), // name
            0,                     // lowerbound
            + SCIPinfinity( m_scip_pricer ),                              // upperbound
            costs_prop[i_unit],              // coeff in obj function
            SCIP_VARTYPE_CONTINUOUS));      // type

        // Adding the variable to the problem and the var matrix
        SCIP_CALL( SCIPaddVar(m_scip_pricer, variable_p_i_t));
        m_variables_p.push_back(variable_p_i_t);
    }
    
    return( SCIP_OKAY );

}


/* create all the constraints, and add them to the scip object and formulation object */
SCIP_RETCODE FormulationPricerOverlappingDecompositionTime::create_constraints()
{
    ostringstream current_cons_name;
    int number_of_time_steps = m_instance_ucp->get_time_steps_number();
    int number_of_units = m_instance_ucp->get_units_number();

    

    //* production constraints

    vector<int> maximum_production( m_instance_ucp->get_production_max() );
    vector<int> minimum_production( m_instance_ucp->get_production_min() );

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
            m_variables_p[i_unit],  /* variable to add */
            1.));                               /* coefficient */
        
        SCIP_CALL( SCIPaddCoefLinear(m_scip_pricer,
            cons_demand_t,
            m_variables_x[ i_unit ],  /* variable to add */
            minimum_production[ i_unit ]));                               /* coefficient */
    }
    SCIP_CALL( SCIPaddCons(m_scip_pricer, cons_demand_t));


    // p < Pmax - Pmin
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIP_CONS* cons_pmin_i_t;
        current_cons_name.str("");
        current_cons_name << "cons_pmin_" << i_unit << "_" << m_block_number;

        // create the constraint
        SCIP_CALL(SCIPcreateConsBasicLinear( m_scip_pricer, 
            &cons_pmin_i_t,                      /** constraint pointer */ 
            current_cons_name.str().c_str(),             /** constraint name */
            0,                                  /** number of variable added */
            nullptr,                            /** array of variable */
            nullptr,                            /** array of coefficient */
            0,                /** LHS */
            SCIPinfinity(m_scip_pricer)));                                /** RHS */

        // add the variables

        SCIP_CALL( SCIPaddCoefLinear(m_scip_pricer,
            cons_pmin_i_t,
            m_variables_x[i_unit],                           /** variable to add */
            maximum_production[i_unit] - minimum_production[i_unit] ));     /* coefficient */
        SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
            cons_pmin_i_t,
            m_variables_p[i_unit],                           /** variable to add */
            - 1));                                    /** coefficient */

        // add the constraint
        SCIP_CALL( SCIPaddCons(m_scip_pricer, cons_pmin_i_t));
    }


    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationPricerOverlappingDecompositionTime::change_reduced_costs( 
    vector< vector < SCIP_Real > > reduced_costs_variable_splitting
)
{
    vector<int> fixed_costs( m_instance_ucp->get_costs_fixed() ); 
    vector<int> prop_costs( m_instance_ucp->get_costs_proportionnal() );
    int number_of_units( m_instance_ucp->get_units_number() );
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    vector<int> production_min( m_instance_ucp->get_production_min() );

    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        // coefficient calculation
        double coefficient( 0. );

        coefficient += reduced_costs_variable_splitting[i_unit][m_block_number];
        coefficient += m_coefficient_repartition_fixed * fixed_costs[ i_unit ];
        coefficient += ( 1 - m_coefficient_repartition_prop ) * prop_costs[i_unit] * production_min[i_unit];


        SCIP_CALL(SCIPchgVarObj(m_scip_pricer, m_variables_x[i_unit], coefficient));
    }
}


ProductionPlan* FormulationPricerOverlappingDecompositionTime::get_production_plan_from_solution()
{
    SCIP_SOL *solution = SCIPgetBestSol( m_scip_pricer );
    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    vector<int> production_minimum( m_instance_ucp->get_production_min() );
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
        quantity_plan[i_unit][m_block_number] += SCIPgetSolVal( m_scip_pricer, solution, m_variables_p[i_unit]);
        quantity_plan[i_unit][m_block_number] += SCIPgetSolVal( m_scip_pricer, solution, m_variables_x[i_unit]) * production_minimum[i_unit];
    }

    // create the plan object
 
    ProductionPlan* new_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    new_plan->compute_cost();
    // new_plan->show();

    return( new_plan );
}


// * gets 


int FormulationPricerOverlappingDecompositionTime::get_block_number()
{
    return( m_block_number );
}

