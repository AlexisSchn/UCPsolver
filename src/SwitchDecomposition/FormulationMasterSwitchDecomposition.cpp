
/**
 * 
 *  Implements the class FormulationMasterSwitchDecomposition
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

// SwitchDecomposition
#include "SwitchDecomposition/FormulationMasterSwitchDecomposition.h"

//** Relaxation
using namespace std;

 

//* class functions 

FormulationMasterSwitchDecomposition::FormulationMasterSwitchDecomposition( InstanceUCP* instance, 
    SCIP* scip_master, 
    bool use_inequality,
    double coefficient_repartition ):
    FormulationMaster(instance, scip_master)
{
    m_use_inequality = use_inequality;
    m_coefficient_repartition = coefficient_repartition; 

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



FormulationMasterSwitchDecomposition::~FormulationMasterSwitchDecomposition()
{
    SCIPsetMessagehdlrQuiet( m_scip_master, 1);
    SCIPfree(&m_scip_master);
}



//* Initilization of the problem

SCIP_RETCODE FormulationMasterSwitchDecomposition::create_variables()
{

    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    int number_of_units( m_instance_ucp->get_units_number());
    ostringstream current_var_name;

    // variables u

    vector<int> cost_start( m_instance_ucp->get_costs_startup() );
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        vector<SCIP_VAR*> variable_u_i;
        int cost_start_i( cost_start[i_unit] );
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_VAR* variable_u_i_t;
            current_var_name.str("");
            current_var_name << "u_" << i_unit << "_" << i_time_step ;

            // Creation of the variable
            SCIP_CALL( SCIPcreateVarBasic( m_scip_master,
                &variable_u_i_t,                // pointer
                current_var_name.str().c_str(), // name
                0.,                             // lowerbound
                1.,                             //! = +inf ? upperbound
                0. * cost_start_i,                   // coeff in obj function
                SCIP_VARTYPE_CONTINUOUS));          // type

            // Adding the variable to the problem and the var matrix
            SCIP_CALL( SCIPaddVar(m_scip_master, variable_u_i_t));
            variable_u_i.push_back(variable_u_i_t);
        }

        m_variables_u.push_back(variable_u_i);
    }

    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationMasterSwitchDecomposition::create_constraints()
{
    // creating the constraints
    ostringstream current_cons_name;

    int number_of_units( m_instance_ucp->get_units_number() );
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );

    //* convexity 
    for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
        SCIP_CONS* convexity_constraint_t;

        current_cons_name.str("");
        current_cons_name << "convexity_constraint_" << i_time_step;
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
        m_constraints_convexity.push_back( convexity_constraint_t);
    }


    //* minimum uptime 
    vector<int> min_uptime( m_instance_ucp->get_min_uptime() );
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        vector<SCIP_CONS*> cons_min_uptime_i;

        for(int i_time_step = min_uptime[i_unit] - 1; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_CONS* cons_min_uptime_i_t;
            current_cons_name.str("");
            current_cons_name << "cons_min_uptime_" << i_unit << "_" << i_time_step;

            // create the constraint
            SCIP_CALL( SCIPcreateConsLinear( m_scip_master, 
                &cons_min_uptime_i_t,                /** constraint pointer */ 
                current_cons_name.str().c_str(),         /** constraint name */
                0,                              /** number of variable added */
                nullptr,                        /** array of variable */
                nullptr,                        /** array of coefficient */
                -SCIPinfinity(m_scip_master),                              /** LHS */
                0,
                true,  /* initial */
			    false, /* separate */
			    true,  /* enforce */
			    true,  /* check */
			    true,  /* propagate */
			    false, /* local */
			    true,  /* modifiable */
			    false, /* dynamic */
			    false, /* removable */
			    false  /* stickingatnode */ 
            ));

            // add the variables
            for(int i_time_step2 = i_time_step - min_uptime[i_unit] + 1 ; i_time_step2 < i_time_step + 1; i_time_step2++)
            {
                SCIP_CALL( SCIPaddCoefLinear( m_scip_master,
                    cons_min_uptime_i_t,
                    m_variables_u[i_unit][i_time_step2],             /** variable to add */
                    1. ));                                    /** coefficient */
            }
            // adding it to the problem
            SCIP_CALL( SCIPaddCons(m_scip_master, cons_min_uptime_i_t));
            cons_min_uptime_i.push_back(cons_min_uptime_i_t);
        }
        m_constraints_min_uptime.push_back(cons_min_uptime_i);
    }


    //* minimum downtime
    vector<int> min_downtime = m_instance_ucp->get_min_downtime();
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        vector<SCIP_CONS*> cons_min_downtime_i;

        for(int i_time_step = min_downtime[i_unit]; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_CONS* cons_min_downtime_i_t;
            current_cons_name.str("");
            current_cons_name << "cons_min_downtime_" << i_unit << "_" << i_time_step;

            // create the constraint
            SCIP_CALL( SCIPcreateConsLinear( m_scip_master, 
                &cons_min_downtime_i_t,                /** constraint pointer */ 
                current_cons_name.str().c_str(),         /** constraint name */
                0,                              /** number of variable added */
                nullptr,                        /** array of variable */
                nullptr,                        /** array of coefficient */
                - SCIPinfinity(m_scip_master),                              /** LHS */
                1,
                true,  /* initial */
			    false, /* separate */
			    true,  /* enforce */
			    true,  /* check */
			    true,  /* propagate */
			    false, /* local */
			    true,  /* modifiable */
			    false, /* dynamic */
			    false, /* removable */
			    false  /* stickingatnode */ 
            ));          /** RHS */

            // add the variables
            for(int i_time_step2 = i_time_step - min_downtime[i_unit] + 1 ; i_time_step2 < i_time_step + 1; i_time_step2++)
            {
                SCIP_CALL( SCIPaddCoefLinear( m_scip_master,
                    cons_min_downtime_i_t,
                    m_variables_u[i_unit][i_time_step2],             /** variable to add */
                    1));                                    /** coefficient */
            }

            // adding it to the problem
            SCIP_CALL( SCIPaddCons(m_scip_master, cons_min_downtime_i_t));
            cons_min_downtime_i.push_back( cons_min_downtime_i_t );
        }
        m_constraints_min_downtime.push_back( cons_min_downtime_i );
    }
 

    //* variable splitting  
    SCIP_Real rhs_value;
    if( m_use_inequality == false )
    {
        rhs_value = 0.;
    }
    if( m_use_inequality == true )
    {
        rhs_value = + SCIPinfinity( m_scip_master );
    }
    
    for( int i_unit = 0; i_unit < number_of_units ; i_unit++ )
    {        
        vector<SCIP_CONS*> cons_variable_splitting_i;
        for(int i_time_step = 0; i_time_step < number_of_time_steps - 1; i_time_step ++)
        {
            SCIP_CONS* cons_variable_splitting_i_t;
            current_cons_name.str("");
            current_cons_name << "cons_variable_splitting_" << i_unit << "_" << i_time_step;

            // create the constraint
            SCIP_CALL( SCIPcreateConsLinear( m_scip_master, 
                &cons_variable_splitting_i_t,                /** constraint pointer */ 
                current_cons_name.str().c_str(),         /** constraint name */
                0,                              /** number of variable added */
                nullptr,                        /** array of variable */
                nullptr,                        /** array of coefficient */
                0.,                              /** LHS */
                rhs_value ,
                true,  /* initial */
                false, /* separate */
                true,  /* enforce */
                true,  /* check */
                true,  /* propagate */
                false, /* local */
                true,  /* modifiable */
                false, /* dynamic */
                false, /* removable */
                false  /* stickingatnode */ 
            ));          /** RHS */

            // add the variables : none

            // adding it to the problem
            SCIP_CALL( SCIPaddCons(m_scip_master, cons_variable_splitting_i_t));
            cons_variable_splitting_i.push_back( cons_variable_splitting_i_t );
        }
        m_constraints_variable_splitting.push_back( cons_variable_splitting_i );
    }
    
    
    //* switch constraints
    m_constraints_switch.reserve( number_of_units );
    for(int i_unit = 0; i_unit < number_of_units; i_unit++)
    {
        vector<SCIP_CONS*> cons_switch_i;
        
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step++)
        {
            SCIP_CONS* cons_switch_i_t;
            current_cons_name.str("");
            current_cons_name << "cons_switch_" << i_unit << "_" << i_time_step;

            SCIP_CALL(SCIPcreateConsLinear(m_scip_master, 
                &cons_switch_i_t,                   /* constraint pointer */ 
                current_cons_name.str().c_str(),    /* constraint name */
                0,                                  /* number of variable added */
                nullptr,                            /* array of variable */
                nullptr,                            /* array of coefficient */
                0.,       /* LHS */
                0.,
                true,  /* initial */
			    false, /* separate */
			    true,  /* enforce */
			    true,  /* check */
			    true,  /* propagate */
			    false, /* local */
			    true,  /* modifiable */
			    false, /* dynamic */
			    false, /* removable */
			    false  /* stickingatnode */ 
            ));            

            // add the variables
            SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
                cons_switch_i_t,
                m_variables_u[i_unit][i_time_step],      /* variable to add */
                -1));                                   /* coefficient */
            SCIP_CALL( SCIPaddCons( m_scip_master, cons_switch_i_t));
            cons_switch_i.push_back( cons_switch_i_t ) ;
        }
        m_constraints_switch.push_back( cons_switch_i );
    }



    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationMasterSwitchDecomposition::create_and_add_first_columns()
{

    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    
    ostringstream current_column_name;

    //* we create a first plan for each block to be able to solve the master problem
    // first time step : we need to switch the units that are not on
    vector<int> initial_state( m_instance_ucp->get_initial_state() );
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

            if( i_time_step > 0)
            {
                up_down_plan[i_unit][i_time_step - 1] = 1;
                quantity_plan[i_unit][i_time_step - 1] = production_max[i_unit]; 
            }
            else
            {
                switch_plan[i_unit][i_time_step] = 1 - initial_state[ i_unit ];
            }
        }

        // then we create the production plan and the master variable

        ProductionPlan* production_plan_block = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
        add_column( production_plan_block, true, i_time_step );
    }


    return( SCIP_OKAY );

}




//* usefull functions


SCIP_RETCODE FormulationMasterSwitchDecomposition::add_column( ProductionPlan* plan_of_new_column, bool initialization, int block_number ) 
{
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    int number_of_units( m_instance_ucp->get_units_number());

    //* create the scip variable
    string column_name = "column_" + to_string(m_vector_columns.size()); 
    SCIP_VAR* new_scip_variable;

    // coefficient : we need the cost only one time step (so that we dont have twice the cost for every x and p)
    double coefficient( 0. );
    vector<int> costs_fixed( m_instance_ucp->get_costs_fixed());
    vector<int> costs_proportionnal( m_instance_ucp->get_costs_proportionnal());
    vector<int> costs_startup( m_instance_ucp->get_costs_startup() );
    vector< vector< double > > up_down_plan( plan_of_new_column->get_up_down_plan() );
    vector< vector< double > > switch_plan( plan_of_new_column->get_switch_plan() );
    vector< vector< double > > quantity_plan( plan_of_new_column->get_quantity_plan() );    
    
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        coefficient += ( 1 - m_coefficient_repartition ) * costs_fixed[ i_unit ] * up_down_plan[ i_unit ][ block_number ];
        if ( block_number == number_of_time_steps - 1 )
        {
            coefficient += m_coefficient_repartition * costs_fixed[ i_unit ] * up_down_plan[ i_unit ][ block_number ];
        }
        if ( block_number > 0 )
        {
            coefficient += m_coefficient_repartition * costs_fixed[i_unit] * up_down_plan[ i_unit ][ block_number - 1  ];
        }
        coefficient += costs_startup[i_unit] * 1 * switch_plan[ i_unit ][ block_number ];
        coefficient += costs_proportionnal[ i_unit ] * quantity_plan[ i_unit ][ block_number ];
    }

    // create the variable and add it to the problem
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
    m_vector_columns.push_back( new_column );


    //* add column to constraints
    // convexity
    SCIPaddCoefLinear(m_scip_master,
                m_constraints_convexity[block_number],
                new_scip_variable,  /* variable to add */
                1.
    );         /* coefficient */        

    // min uptime
    vector<int> min_uptime( m_instance_ucp->get_min_uptime() );
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        // x 
        if( block_number >= min_uptime[ i_unit ] - 1 )
        {
            SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
                    m_constraints_min_uptime[i_unit][ block_number - min_uptime[i_unit] + 1 ],
                    new_scip_variable,        /** variable to add */
                    - up_down_plan[i_unit][block_number]                               /** coefficient */
            ));                                  
        }
    }
   
    // min downtime
    vector<int> min_downtime( m_instance_ucp->get_min_downtime() );
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        if( block_number <= number_of_time_steps - min_downtime[i_unit] - 1)
        {
            SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
                    m_constraints_min_downtime[i_unit][block_number],
                    new_scip_variable,                                  // variable to add
                    up_down_plan[i_unit][ block_number ]     // coefficient                                                                  // coefficient
            ));   
        }
    }

    // variable splitting
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        if( block_number >= 1)
        {
            SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
            m_constraints_variable_splitting[i_unit][block_number - 1],
            new_scip_variable,                                  // variable to add
            - up_down_plan[i_unit][ block_number - 1]     // coefficient                                                                  // coefficient
        ));
        }
        if( block_number < number_of_time_steps - 1)
        {   
            SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
                m_constraints_variable_splitting[i_unit][block_number],
                new_scip_variable,                                  // variable to add
                up_down_plan[i_unit][ block_number ]     // coefficient                                                                  // coefficient
            ));
        } 
    }

    // switch
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
                m_constraints_switch[i_unit][block_number],
                new_scip_variable,                                  // variable to add
                switch_plan[i_unit][ block_number ]     // coefficient                                                                  // coefficient
        ));
    }
    
    return( SCIP_OKAY );

}



ProductionPlan* FormulationMasterSwitchDecomposition::get_production_plan_from_solution()
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

    // for each column, we add its plan value to the base plan, times it's coefficient. Will depend on the decompositon
    for(int i_column = 0; i_column < number_columns; i_column ++)
    {
        // get the plan with all the corresponding informations, and the coefficient
        VariableMaster* current_variable = m_vector_columns[i_column];
        ProductionPlan* current_plan = current_variable->get_production_plan();
        int current_block = current_variable->get_block_number();
        vector<vector< double >> current_up_down_plan( current_plan->get_up_down_plan() );
        vector<vector< double >> current_quantity_plan( current_plan->get_quantity_plan() );
        vector<vector< double >> current_switch_plan( current_plan->get_switch_plan() );

        SCIP_Real coefficient_column( SCIPgetSolVal( m_scip_master, solution, current_variable->get_variable_pointer()));

        // add the value to the main plan
        for( int i_unit = 0; i_unit < number_of_units; i_unit++ )
        {
            up_down_plan[i_unit][current_block] += current_up_down_plan[i_unit][current_block]*coefficient_column;
            quantity_plan[i_unit][current_block] += current_quantity_plan[i_unit][current_block]*coefficient_column;
            switch_plan[i_unit][current_block] += current_switch_plan[i_unit][current_block] * coefficient_column;
        }
    }
 
    // we can now create the plan
    ProductionPlan* production_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    production_plan->compute_cost();

    return( production_plan );
}


//* gets

int FormulationMasterSwitchDecomposition::get_columns_number()
{
    return(m_vector_columns.size());
}


SCIP_CONS** FormulationMasterSwitchDecomposition::get_constraint_convexity( int time_step_number )
{
    return( &m_constraints_convexity[ time_step_number ] );
}

SCIP_CONS** FormulationMasterSwitchDecomposition::get_constraint_min_uptime( int unit_number, int time_step_number )
{
    return( &m_constraints_min_uptime[ unit_number ][ time_step_number ] );
}

SCIP_CONS** FormulationMasterSwitchDecomposition::get_constraint_min_downtime( int unit_number, int time_step_number )
{
    return( &m_constraints_min_downtime[ unit_number ][ time_step_number ] );
}

SCIP_CONS** FormulationMasterSwitchDecomposition::get_constraint_variable_splitting( int unit_number, int time_step_number )
{
    return( &m_constraints_variable_splitting[ unit_number ][ time_step_number ] );
}


SCIP_CONS** FormulationMasterSwitchDecomposition::get_constraint_switch( int unit_number, int time_step_number )
{
    return( &m_constraints_switch[ unit_number ][ time_step_number ] );
}



