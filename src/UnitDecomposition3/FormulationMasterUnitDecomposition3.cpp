/**
 * 
 *  Implements the class FormulationMasterUnitDecomposition3
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
#include "Decomposition/FormulationMaster.h"
#include "Decomposition/VariableMaster.h"

// Unit Decomposition
#include "UnitDecomposition3/FormulationMasterUnitDecomposition3.h"

//** Relaxation
using namespace std;




FormulationMasterUnitDecomposition3::FormulationMasterUnitDecomposition3( InstanceUCP* instance, SCIP* scip_master):
    FormulationMaster(instance, scip_master)
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
    //*  add the first columns (one per block)
    retcode = create_and_add_first_columns();
    if ( retcode != SCIP_OKAY)
    {
        SCIPprintError( retcode );
    }
}



FormulationMasterUnitDecomposition3::~FormulationMasterUnitDecomposition3()
{
    SCIPsetMessagehdlrQuiet( m_scip_master, 1);
    SCIPfree(&m_scip_master);
}

SCIP_RETCODE FormulationMasterUnitDecomposition3::add_column( ProductionPlan* plan_of_new_column, bool initialization, int block_number ) 
{
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    int number_of_units( m_instance_ucp->get_units_number());

    //* create the scip variable
    string column_name = "column_" + to_string(m_vector_columns.size()); 
    SCIP_VAR* new_scip_variable;

    SCIPcreateVar(  m_scip_master,
        &new_scip_variable,                            // pointer 
        column_name.c_str(),                            // name
        0.,                                     // lowerbound
        +SCIPinfinity(m_scip_master),            // upperbound
        plan_of_new_column->get_cost(),          // coeff in obj function
        SCIP_VARTYPE_CONTINUOUS,
        false, false, NULL, NULL, NULL, NULL, NULL
    );
    if( initialization)
    {
        SCIPaddVar( m_scip_master, new_scip_variable );
    }
    else
    {
        SCIPaddPricedVar(m_scip_master, new_scip_variable, 1.);
    }
    
    //* create the column
    VariableMaster* new_column = new VariableMaster( new_scip_variable, plan_of_new_column );
    new_column->add_block_number( block_number );
    m_vector_columns.push_back( new_column );


    //* add column to constraints
    // convexity
    SCIPaddCoefLinear(m_scip_master,
                m_constraints_convexity[block_number],
                new_scip_variable,  /* variable to add */
                1.
    );         /* coefficient */        

    // demand : don't add it

    // production constraints
    double production_max( m_instance_ucp->get_production_max()[block_number]);
    double production_min( m_instance_ucp->get_production_min()[block_number]);
    vector<double> up_down_plan_unit( plan_of_new_column->get_up_down_plan()[block_number]);

    for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step++ )
    {
        // pmax
        SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
            m_constraints_pmax[block_number][i_time_step],
            new_scip_variable,                                          /* variable to add */
            + production_max * up_down_plan_unit[i_time_step] ));       /* coefficient */
        // pmin
        SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
            m_constraints_pmin[block_number][i_time_step],
            new_scip_variable,                                          /* variable to add */
            - production_min * up_down_plan_unit[i_time_step] ));       /* coefficient */
    }

    return( SCIP_OKAY );

}



SCIP_RETCODE FormulationMasterUnitDecomposition3::create_variables()
{

    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    int number_of_units( m_instance_ucp->get_units_number());
    ostringstream current_var_name;

    // Variables p

    vector<int> prod_max( m_instance_ucp->get_production_max() );
    vector<int> cost_prop( m_instance_ucp->get_costs_proportionnal() );
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++ )
    {
        vector< SCIP_VAR* > variables_p_i;
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_VAR* variable_p_i_t;
            current_var_name.str("");
            current_var_name << "p_" << i_unit << "_" << i_time_step ;

            // Creation of the variable
            SCIP_CALL( SCIPcreateVarBasic( m_scip_master,
                &variable_p_i_t,                // pointer
                current_var_name.str().c_str(), // name
                0,                     // lowerbound
                prod_max[i_unit],                              // upperbound
                cost_prop[i_unit],              // coeff in obj function
                SCIP_VARTYPE_CONTINUOUS));      // type

            // Adding the variable to the problem and the var matrix
            SCIP_CALL( SCIPaddVar(m_scip_master, variable_p_i_t));
            variables_p_i.push_back( variable_p_i_t );
        }

        m_variable_p.push_back( variables_p_i );
    }

    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationMasterUnitDecomposition3::create_constraints()
{
    // creating the constraints
    ostringstream current_cons_name;

    // convexity constraints
    int number_of_units( m_instance_ucp->get_units_number() );
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );

    m_constraints_convexity.resize( number_of_units );

    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIP_CONS* convexity_constraint_i;

        current_cons_name.str("");
        current_cons_name << "convexity_constraint_" << i_unit;
        SCIPcreateConsLinear( m_scip_master, &convexity_constraint_i, current_cons_name.str().c_str(), 0, NULL, NULL,
                    1.0,   // lhs 
                    1.0,   // rhs 
                    true,  /* initial */
                    false, /* separate */
                    true,  /* enforce */
                    true,  /* check */
                    true,  /* propagate */
                    false, /* local */
                    true,  /* modifiable */
                    false, /* dynamic */
                    false, /* removable */
                    false  /* stickingatnode */ );
        SCIPaddCons( m_scip_master, convexity_constraint_i );
        m_constraints_convexity[i_unit] = convexity_constraint_i;
    }



    // Demand constraints
    m_constraints_demand.resize( number_of_time_steps );
    vector<int> demand( m_instance_ucp->get_demand() );
    for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
        SCIP_CONS* demand_constraint_t;

        current_cons_name.str("");
        current_cons_name << "demand_constraint_" << i_time_step;
        SCIPcreateConsLinear( m_scip_master, 
                &demand_constraint_t, 
                current_cons_name.str().c_str(), 
                0, 
                NULL, 
                NULL,
			    demand[i_time_step],   // lhs 
			    + SCIPinfinity( m_scip_master ),   // rhs 
			    true,  /* initial */
			    false, /* separate */
			    true,  /* enforce */
			    true,  /* check */
			    true,  /* propagate */
			    false, /* local */
			    true,  /* modifiable */
			    false, /* dynamic */
			    false, /* removable */
			    false  /* stickingatnode */ );
        SCIPaddCons( m_scip_master, demand_constraint_t );


        for(int i_unit = 0; i_unit < number_of_units; i_unit++)
        {
            SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
                demand_constraint_t,
                m_variable_p[i_unit][i_time_step],  /* variable to add */
                1.));                               /* coefficient */
        }
        SCIPaddCons(m_scip_master, demand_constraint_t);
        m_constraints_demand.push_back(demand_constraint_t);

        m_constraints_demand[i_time_step] = demand_constraint_t;
    }



    
    //  * production constraints

    // p < pmax 
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        vector< SCIP_CONS*> cons_pmax_i;
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_CONS* cons_pmax_i_t;
            current_cons_name.str("");
            current_cons_name << "cons_pmax_" << i_unit << "_" << i_time_step;

            // create the constraint
            SCIP_CALL( SCIPcreateConsLinear( m_scip_master, 
                &cons_pmax_i_t,                      /** constraint pointer */ 
                current_cons_name.str().c_str(),             /** constraint name */
                0,                                  /** number of variable added */
                nullptr,                            /** array of variable */
                nullptr,                            /** array of coefficient */
                0,                /** LHS */
                SCIPinfinity(m_scip_master),                                  /** RHS */
                true,  /* initial */
			    false, /* separate */
			    true,  /* enforce */
			    true,  /* check */
			    true,  /* propagate */
			    false, /* local */
			    true,  /* modifiable */
			    false, /* dynamic */
			    false, /* removable */
			    false  /* stickingatnode */ )
            );

            // add the variable p, x is added when columns are added
            SCIPaddCoefLinear( m_scip_master,
                cons_pmax_i_t,
                m_variable_p[i_unit][i_time_step],                           /** variable to add */
                - 1.                                    /** coefficient */
            );
            // add the constraint
            SCIP_CALL( SCIPaddCons(m_scip_master, cons_pmax_i_t));
            cons_pmax_i.push_back(cons_pmax_i_t);
        }
        m_constraints_pmax.push_back(cons_pmax_i);

    }

    // pmin < p
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        vector< SCIP_CONS* > cons_pmin_i;
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {   
            SCIP_CONS* cons_pmin_i_t;
            current_cons_name.str("");
            current_cons_name << "cons_pmin_" << i_unit << "_" << i_time_step;

            // create the constraint
            SCIP_CALL(SCIPcreateConsLinear( m_scip_master, 
                &cons_pmin_i_t,                      /** constraint pointer */ 
                current_cons_name.str().c_str(),             /** constraint name */
                0,                                  /** number of variable added */
                nullptr,                            /** array of variable */
                nullptr,                            /** array of coefficient */
                0,                /** LHS */
                SCIPinfinity(m_scip_master),
                true,  /* initial */
			    false, /* separate */
			    true,  /* enforce */
			    true,  /* check */
			    true,  /* propagate */
			    false, /* local */
			    true,  /* modifiable */
			    false, /* dynamic */
			    false, /* removable */
			    false  /* stickingatnode */ )
            );                                /** RHS */

            // add the variables
            SCIP_CALL( SCIPaddCoefLinear( m_scip_master,
                cons_pmin_i_t,
                m_variable_p[i_unit][i_time_step],                           /** variable to add */
                1. ));                                    /** coefficient */

            // add the constraint
            SCIP_CALL( SCIPaddCons(m_scip_master, cons_pmin_i_t));
            cons_pmin_i.push_back(cons_pmin_i_t);
        }
        m_constraints_pmin.push_back(cons_pmin_i);
    }

    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationMasterUnitDecomposition3::create_and_add_first_columns()
{

    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    vector<int> maximum_production( m_instance_ucp->get_production_max() );
    vector<int> initial_state( m_instance_ucp->get_initial_state() );
    ostringstream current_column_name;

    //* for each unit, we create a master variable where the unit is working at max production for each time step

    for(int i_unit = 0; i_unit < number_of_units; i_unit++)
    {
        // we start by creating the correct plan

        vector< vector < double > > up_down_plan;
        vector< vector < double > > switch_plan;
        vector< vector < double > > quantity_plan;

        up_down_plan.resize(number_of_units);
        switch_plan.resize(number_of_units);
        quantity_plan.resize(number_of_units);

        for( int i_unit_2 = 0; i_unit_2 < number_of_units; i_unit_2++ )
        {
            if( i_unit_2 == i_unit )
            {
                up_down_plan[i_unit_2].resize(number_of_time_steps, 1.);
                switch_plan[i_unit_2].resize(number_of_time_steps, 0.);
                quantity_plan[i_unit_2].resize(number_of_time_steps, 0.);
                
                if( initial_state[i_unit_2] == 0)
                {
                    switch_plan[i_unit_2][0] = 1;
                }
            }
            else
            {
                up_down_plan[i_unit_2].resize(number_of_time_steps, 0.);
                switch_plan[i_unit_2].resize(number_of_time_steps, 0.);
                quantity_plan[i_unit_2].resize(number_of_time_steps, 0.);
            }
        }

        // then we create the production plan and the master variable

        ProductionPlan* production_plan_block = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
        add_column( production_plan_block, true, i_unit );

    }

    return( SCIP_OKAY );

}


ProductionPlan* FormulationMasterUnitDecomposition3::get_production_plan_from_solution()
{
    int number_columns( m_vector_columns.size());
    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());

    SCIP_SOL *solution = SCIPgetBestSol( m_scip_master );

  
    // We start by creating full zeros plans
    vector< vector < double > > up_down_plan;
    vector< vector < double > > switch_plan;
    vector< vector < double > > quantity_plan;
    up_down_plan.resize(number_of_units);
    switch_plan.resize(number_of_units);
    quantity_plan.resize(number_of_units);
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        up_down_plan[i_unit].resize(number_of_time_steps, 0);
        switch_plan[i_unit].resize(number_of_time_steps, 0);
        quantity_plan[i_unit].resize(number_of_time_steps, 0);
    }

    // for each column, we add its plan value to the base plan, times it's coefficient
    // for updown and switch plan here !
    for(int i_column = 0; i_column < number_columns; i_column ++)
    {
        // get the plan with all the corresponding informations, and the coefficient
        VariableMaster* current_variable = m_vector_columns[i_column];
        ProductionPlan* current_plan = current_variable->get_production_plan();
        vector< vector< double > > current_up_down_plan( current_plan->get_up_down_plan() );
        vector< vector< double > > current_switch_plan( current_plan->get_switch_plan() );
        SCIP_Real coefficient_column( SCIPgetSolVal( m_scip_master, solution, current_variable->get_variable_pointer()));

        // add the value to the main plan
        for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
        {
            for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step++ )
            {
                up_down_plan[i_unit][i_time_step] += current_up_down_plan[i_unit][i_time_step]*coefficient_column;
                switch_plan[i_unit][i_time_step] += current_switch_plan[i_unit][i_time_step]*coefficient_column;
            }
        }

    }

    // for quantity plan : we take it from the solution directly

    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            quantity_plan[i_unit][i_time_step] +=  SCIPgetSolVal( m_scip_master, solution, m_variable_p[i_unit][i_time_step]);
        }
    }
 
    // we can now create the plan
    ProductionPlan* production_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    production_plan->compute_cost();

    return( production_plan );
}


//* gets

SCIP_CONS** FormulationMasterUnitDecomposition3::get_convexity_constraint(int number_unit)
{
    return( &m_constraints_convexity[number_unit] );
}


SCIP_CONS** FormulationMasterUnitDecomposition3::get_constraint_pmax(int number_unit, int number_time_step)
{
    return( &m_constraints_pmax[number_unit][number_time_step] );
}


SCIP_CONS** FormulationMasterUnitDecomposition3::get_constraint_pmin(int number_unit, int number_time_step)
{
    return( &m_constraints_pmin[number_unit][number_time_step] );
}


int FormulationMasterUnitDecomposition3::get_columns_number()
{
    return(m_vector_columns.size());
}
