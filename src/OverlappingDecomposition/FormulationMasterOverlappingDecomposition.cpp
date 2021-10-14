
/**
 * 
 *  Implements the class FormulationMasterOverlappingDecomposition
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

// OverlappingDecomposition
#include "OverlappingDecomposition/FormulationMasterOverlappingDecomposition.h"

//** Relaxation
using namespace std;



//* class functions

FormulationMasterOverlappingDecomposition::FormulationMasterOverlappingDecomposition( 
    InstanceUCP* instance, 
    SCIP* scip_master,
    bool use_inequality,
    double coefficient_repartition_fixed,
    double coefficient_repartition_prop
):
    FormulationMaster(instance, scip_master),
    m_use_inequality( use_inequality ),
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
    //*  add the first columns (one per block)
    retcode = create_and_add_first_columns();
    if ( retcode != SCIP_OKAY)
    {
        SCIPprintError( retcode );
    }
}

  

FormulationMasterOverlappingDecomposition::~FormulationMasterOverlappingDecomposition()
{
    SCIPsetMessagehdlrQuiet( m_scip_master, 1);
    SCIPfree(&m_scip_master );
}



//* Initilization of the problem

SCIP_RETCODE FormulationMasterOverlappingDecomposition::create_variables()
{


    //* create the variables

    // No variables to create. they are all lambdas created later...


    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationMasterOverlappingDecomposition::create_constraints()
{
    // creating the constraints
    ostringstream current_cons_name;

    int number_of_units( m_instance_ucp->get_units_number() );
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );

    // variable splitting 
    SCIP_Real rhs_value;
    if( m_use_inequality == false )
    {
        rhs_value = 0.;
    }
    if( m_use_inequality == true )
    {
        rhs_value = + SCIPinfinity( m_scip_master );
    }
 
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++ )
    {
        vector<SCIP_Cons*> constraint_variable_splitting_i; 
        for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_CONS* constraint_variable_splitting_i_t;
            current_cons_name.str("");
            current_cons_name << "convexity_constraint_" << i_unit << "_" << i_time_step;
            SCIPcreateConsLinear( m_scip_master,  &constraint_variable_splitting_i_t, current_cons_name.str().c_str(), 0, NULL, NULL,
                        0.,   // lhs 
                        rhs_value,   // rhs 
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
            SCIPaddCons( m_scip_master, constraint_variable_splitting_i_t );
            constraint_variable_splitting_i.push_back( constraint_variable_splitting_i_t);
        }
        m_constraints_variable_splitting.push_back( constraint_variable_splitting_i );
    }


    // convexity unit
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIP_CONS* convexity_constraint_i;

        current_cons_name.str("");
        current_cons_name << "convexity_constraint_unit_" << i_unit;
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
        m_constraints_convexity_unit.push_back( convexity_constraint_i);
    }


    // convexity time
    for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
        SCIP_CONS* convexity_constraint_t;

        current_cons_name.str("");
        current_cons_name << "convexity_constraint_time_" << i_time_step;
        SCIPcreateConsLinear( m_scip_master, &convexity_constraint_t, current_cons_name.str().c_str(), 0, NULL, NULL,
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
        SCIPaddCons( m_scip_master, convexity_constraint_t );
        m_constraints_convexity_time.push_back( convexity_constraint_t);
    }


    return( SCIP_OKAY );
}
 

SCIP_RETCODE FormulationMasterOverlappingDecomposition::create_and_add_first_columns()
{

    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    vector<int> initial_states( m_instance_ucp->get_initial_state());
    vector<int> maximum_production( m_instance_ucp->get_production_max() );
    ostringstream current_column_name;


    //* for unit decomposition
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
                
                if( initial_states[i_unit_2] == 0)
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
        add_column_unit( production_plan_block, true, i_unit );

    }



    //* for time
    vector<int> production_max( m_instance_ucp->get_production_max() );
    for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
        // we start by creating the correct plan
        vector< vector < double > > up_down_plan;
        vector< vector < double > > switch_plan;
        vector< vector < double > > quantity_plan;

        up_down_plan.resize(number_of_units);
        switch_plan.resize(number_of_units);
        quantity_plan.resize(number_of_units);

        for( int i_unit = 0; i_unit < number_of_units; i_unit++ )
        {

            up_down_plan[i_unit].resize(number_of_time_steps, 0);
            switch_plan[i_unit].resize(number_of_time_steps, 0);
            quantity_plan[i_unit].resize(number_of_time_steps, 0.);

            up_down_plan[i_unit][i_time_step] = 1;
            quantity_plan[i_unit][i_time_step] = production_max[i_unit];

        }

        // then we create the production plan and the master variable

        ProductionPlan* production_plan_block = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
        add_column_time( production_plan_block, true, i_time_step );
    }


    
    

    return( SCIP_OKAY );

}




//* usefull functions


SCIP_RETCODE FormulationMasterOverlappingDecomposition::add_column_unit( 
    ProductionPlan* plan_of_new_column, 
    bool initialization, 
    int block_number ) 
{
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    int number_of_units( m_instance_ucp->get_units_number());
    vector<double> up_down_plan( plan_of_new_column->get_up_down_plan()[block_number] );
    vector<double> switch_plan( plan_of_new_column->get_switch_plan()[block_number] );
    int cost_startup( m_instance_ucp->get_costs_startup()[block_number]);
    int cost_fixed( m_instance_ucp->get_costs_fixed()[block_number] );
    int cost_prop( m_instance_ucp->get_costs_proportionnal()[block_number] );
    int production_minimum( m_instance_ucp->get_production_min()[ block_number ] );

    //* calculate the coefficient in the objective function
    double coefficient( 0. );
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
        coefficient += switch_plan[i_time_step] * cost_startup;
        coefficient += ( 1 - m_coefficient_repartition_fixed ) * up_down_plan[ i_time_step ] * cost_fixed;
        coefficient += m_coefficient_repartition_prop * up_down_plan[ i_time_step ] * cost_prop * production_minimum;
    }


    //* create the scip variable
    string column_name = "column_unit_" + to_string(m_vector_columns_unit.size()); 
    SCIP_VAR* new_scip_variable;

    SCIPcreateVar(  m_scip_master,
        &new_scip_variable,                            // pointer 
        column_name.c_str(),                            // name
        0.,                                     // lowerbound
        +SCIPinfinity(m_scip_master),            // upperbound
        coefficient,          // coeff in obj function
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
    m_vector_columns_unit.push_back( new_column );
 

    //* add column to constraints
    // convexity unit
    SCIPaddCoefLinear(m_scip_master,
                m_constraints_convexity_unit[block_number],
                new_scip_variable,  /* variable to add */
                1.
    );         /* coefficient */        

    // variable splitting
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
        SCIPaddCoefLinear(m_scip_master,
                    m_constraints_variable_splitting[block_number][i_time_step],
                    new_scip_variable,  /* variable to add */
                    + up_down_plan[i_time_step]
        );         /* coefficient */      
    }

    return( SCIP_OKAY );

}


SCIP_RETCODE FormulationMasterOverlappingDecomposition::add_column_time( ProductionPlan* plan_of_new_column, bool initialization, int block_number ) 
{
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    int number_of_units( m_instance_ucp->get_units_number());

    vector<vector<double>> up_down_plan( plan_of_new_column->get_up_down_plan() );
    vector<vector<double>> quantity_plan( plan_of_new_column->get_quantity_plan() );
    vector<int> cost_prop( m_instance_ucp->get_costs_proportionnal());
    vector<int> cost_fixed( m_instance_ucp->get_costs_fixed() );
    vector<int> production_minimum( m_instance_ucp->get_production_min() );



    //* calculate the coefficient
    double coefficient = 0;
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        coefficient += m_coefficient_repartition_fixed * cost_fixed[i_unit] * up_down_plan[i_unit][block_number];
        coefficient += cost_prop[i_unit] * quantity_plan[i_unit][block_number];
        coefficient += - cost_prop[i_unit ] * production_minimum[i_unit ] * up_down_plan[i_unit][block_number];
        coefficient += ( 1 - m_coefficient_repartition_prop ) * cost_prop[i_unit] * production_minimum[i_unit] * up_down_plan[i_unit][block_number];
    }


    //* create the scip variable
    string column_name = "column_time_" + to_string(m_vector_columns_time.size()); 
    SCIP_VAR* new_scip_variable;

    SCIPcreateVar(  m_scip_master,
        &new_scip_variable,                            // pointer 
        column_name.c_str(),                            // name
        0.,                                     // lowerbound
        +SCIPinfinity(m_scip_master),            // upperbound
        coefficient,          // coeff in obj function
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
    m_vector_columns_time.push_back( new_column );


    //* add column to constraints
    // convexity
    SCIPaddCoefLinear(m_scip_master,
                m_constraints_convexity_time[block_number],
                new_scip_variable,  /* variable to add */
                1.  /* coefficient */
    );                

    // variable splitting

    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIPaddCoefLinear(m_scip_master,
                m_constraints_variable_splitting[i_unit][block_number],
                new_scip_variable,  /* variable to add */
                - up_down_plan[i_unit][block_number]  /* coefficient */
        );                
    }

    return( SCIP_OKAY );

}


ProductionPlan* FormulationMasterOverlappingDecomposition::get_production_plan_from_solution()
{
    int number_columns_unit( m_vector_columns_unit.size());
    int number_columns_time( m_vector_columns_time.size() );
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

    // unit decompo columns : for u and x
    vector<int> production_minimum( m_instance_ucp->get_production_min() );
    for(int i_column = 0; i_column < number_columns_unit; i_column ++)
    {
        // get the plan with all the corresponding informations, and the coefficient
        VariableMaster* current_variable = m_vector_columns_unit[i_column];
        ProductionPlan* current_plan = current_variable->get_production_plan();
        vector< vector< double > > current_up_down_plan( current_plan->get_up_down_plan() );
        vector< vector< double > > current_switch_plan( current_plan->get_switch_plan() );

        SCIP_Real coefficient_column( SCIPgetSolVal( m_scip_master, solution, current_variable->get_variable_pointer()));

        // add the value to the main plan
        for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
        {
            for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step++ )
            {
                up_down_plan[i_unit][i_time_step] += current_up_down_plan[i_unit][i_time_step] * coefficient_column;
                switch_plan[i_unit][i_time_step] += current_switch_plan[i_unit][i_time_step] * coefficient_column;
                quantity_plan[i_unit][i_time_step] += m_coefficient_repartition_prop * current_up_down_plan[i_unit][ i_time_step ] * production_minimum[i_unit] * coefficient_column;
            }
        }
    }

    // time decompo columns : for p and x
 
    for(int i_column = 0; i_column < number_columns_time; i_column ++)
    {
        // get the plan with all the corresponding informations, and the coefficient
        VariableMaster* current_variable = m_vector_columns_time[i_column];
        ProductionPlan* current_plan = current_variable->get_production_plan();
        int current_block = current_variable->get_block_number();
        vector<vector< double >> current_quantity_plan( current_plan->get_quantity_plan() );
        vector<vector< double >> current_up_down_plan( current_plan->get_up_down_plan() );

        SCIP_Real coefficient_column( SCIPgetSolVal( m_scip_master, solution, current_variable->get_variable_pointer()));

        // add the value to the main plan
        for( int i_unit = 0; i_unit < number_of_units; i_unit++ )
        {
            quantity_plan[i_unit][current_block] += current_quantity_plan[i_unit][current_block]*coefficient_column;
            quantity_plan[i_unit][current_block] += - current_up_down_plan[i_unit][current_block] * production_minimum[ i_unit ] *coefficient_column;
            quantity_plan[i_unit][current_block] += ( 1 - m_coefficient_repartition_prop ) * current_up_down_plan[i_unit][current_block] * production_minimum[ i_unit ] * coefficient_column;

            // up_down_plan[ i_unit ][ current_block ] += m_coefficient_repartition_fixed * current_up_down_plan[ i_unit ][ current_block ] * coefficient_column;
        }
    }

    // we can now create the plan
    ProductionPlan* production_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    production_plan->compute_cost();

    return( production_plan );
}






//* gets

SCIP_CONS** FormulationMasterOverlappingDecomposition::get_constraint_convexity_time( int time_step_number )
{
    return( &m_constraints_convexity_time[time_step_number]);
}

SCIP_CONS** FormulationMasterOverlappingDecomposition::get_constraint_convexity_unit( int unit_number )
{
    return( &m_constraints_convexity_unit[unit_number]);
}

SCIP_CONS** FormulationMasterOverlappingDecomposition::get_constraint_variable_splitting( int unit_number, int time_step_number )
{
    return( &m_constraints_variable_splitting[unit_number][time_step_number]);
}


int FormulationMasterOverlappingDecomposition::get_columns_number_unit()
{
    return( m_vector_columns_unit.size());
}


int FormulationMasterOverlappingDecomposition::get_columns_number_time()
{
    return( m_vector_columns_time.size());
}




