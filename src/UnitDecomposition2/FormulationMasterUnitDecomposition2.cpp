/**
 * 
 *  Implements the class FormulationMasterUnitDecomposition2UnitDecomposition2 
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
#include "UnitDecomposition2/FormulationMasterUnitDecomposition2.h"

//** Relaxation
using namespace std;




FormulationMasterUnitDecomposition2::FormulationMasterUnitDecomposition2( InstanceUCP* instance, SCIP* scip_master):
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



FormulationMasterUnitDecomposition2::~FormulationMasterUnitDecomposition2()
{
}



SCIP_RETCODE FormulationMasterUnitDecomposition2::add_column( ProductionPlan* plan_of_new_column, bool initialization, int block_number ) 
{
    int number_time_step( m_instance_ucp->get_time_steps_number() );

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

    if( initialization ) 
    {
        SCIPaddVar( m_scip_master, new_scip_variable);
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
                m_convexity_constraint[block_number],
                new_scip_variable,  /* variable to add */
                1.);         /* coefficient */        
    
    // demand constraints : only for the block 
    vector<double> quantity_plan = plan_of_new_column->get_quantity_plan()[block_number];
    for( int i_time_step = 0; i_time_step < number_time_step; i_time_step++ )
    {
        double quantity_plan_t( quantity_plan[i_time_step]);
        SCIPaddCoefLinear(m_scip_master,
            m_complicating_constraints[i_time_step],
            new_scip_variable,  /* variable to add */
            quantity_plan_t     /* coefficient */
        );                                    
    }

    return( SCIP_OKAY );
}



SCIP_RETCODE FormulationMasterUnitDecomposition2::create_variables()
{
    return( SCIP_OKAY );
}



SCIP_RETCODE FormulationMasterUnitDecomposition2::create_constraints()
{
    // creating the constraints
    ostringstream current_cons_name;

    // convexity constraints
    int number_of_units( m_instance_ucp->get_units_number() );
    m_convexity_constraint.resize( number_of_units );

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
        m_convexity_constraint[i_unit] = convexity_constraint_i;
    }


    // Complicating constraints
    int number_time_step( m_instance_ucp->get_time_steps_number() );
    m_complicating_constraints.resize( number_time_step );
    vector<int> demand( m_instance_ucp->get_demand() );
    for( int i_time_step = 0; i_time_step < number_time_step; i_time_step ++)
    {
        SCIP_CONS* demand_constraint_t;

        current_cons_name.str("");
        current_cons_name << "demand_constraint_" << i_time_step;
        SCIPcreateConsLinear( m_scip_master, &demand_constraint_t, current_cons_name.str().c_str(), 0, NULL, NULL,
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
        m_complicating_constraints[i_time_step] = demand_constraint_t;
    }

    return( SCIP_OKAY );
}



SCIP_RETCODE FormulationMasterUnitDecomposition2::create_and_add_first_columns()
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
                quantity_plan[i_unit_2].resize(number_of_time_steps, maximum_production[i_unit_2]);
                
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



ProductionPlan* FormulationMasterUnitDecomposition2::get_production_plan_from_solution()
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
    for(int i_column = 0; i_column < number_columns; i_column ++)
    {
        // get the plan with all the corresponding informations, and the coefficient
        VariableMaster* current_variable = m_vector_columns[i_column];
        ProductionPlan* current_plan = current_variable->get_production_plan();
        vector< vector< double > > current_up_down_plan( current_plan->get_up_down_plan() );
        vector< vector< double > > current_switch_plan( current_plan->get_switch_plan() );
        vector< vector< double > > current_quantity_plan( current_plan->get_quantity_plan() );
        SCIP_Real coefficient_column( SCIPgetSolVal( m_scip_master, solution, current_variable->get_variable_pointer()));

        // add the value to the main plan
        for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
        {
            for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step++ )
            {
                up_down_plan[i_unit][i_time_step] += current_up_down_plan[i_unit][i_time_step]*coefficient_column;
                switch_plan[i_unit][i_time_step] += current_switch_plan[i_unit][i_time_step]*coefficient_column;
                quantity_plan[i_unit][i_time_step] += current_quantity_plan[i_unit][i_time_step]*coefficient_column; 
            }
        }

    }
 
    // we can now create the plan
    ProductionPlan* production_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    production_plan->compute_cost();

    return( production_plan );
}



//* gets


SCIP_CONS** FormulationMasterUnitDecomposition2::get_convexity_constraint(int number_unit)
{
    return( &m_convexity_constraint[number_unit] );
}


SCIP_CONS** FormulationMasterUnitDecomposition2::get_complicating_constraints(int number_time_step)
{
    return( &m_complicating_constraints[number_time_step] );
}

int FormulationMasterUnitDecomposition2::get_columns_number()
{
    return( m_vector_columns.size());
}

