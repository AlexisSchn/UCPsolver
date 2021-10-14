
/**
 * 
 *  Implements the class FormulationMasterTimeDecomposition2
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

// TimeDecomposition2
#include "TimeDecomposition2/FormulationMasterTimeDecomposition2.h"

//** Relaxation
using namespace std;



//* class functions

FormulationMasterTimeDecomposition2::FormulationMasterTimeDecomposition2( InstanceUCP* instance, SCIP* scip_master):
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



FormulationMasterTimeDecomposition2::~FormulationMasterTimeDecomposition2()
{
    SCIPsetMessagehdlrQuiet( m_scip_master, 1);
    SCIPfree(&m_scip_master);
}



//* Initilization of the problem

SCIP_RETCODE FormulationMasterTimeDecomposition2::create_variables()
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
                cost_start_i,                   // coeff in obj function
                SCIP_VARTYPE_CONTINUOUS));          // type

            // Adding the variable to the problem and the var matrix
            SCIP_CALL( SCIPaddVar(m_scip_master, variable_u_i_t));
            variable_u_i.push_back(variable_u_i_t);
        }

        m_variables_u.push_back(variable_u_i);
    }


    // variables p
    vector<int> costs_prop( m_instance_ucp->get_costs_proportionnal() );
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        vector< SCIP_Var* > variable_p_i;
        for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_VAR* variable_p_i_t;
            current_var_name.str("");
            current_var_name << "p_" << i_unit << "_" << i_time_step ;

            // Creation of the variable
            SCIP_CALL( SCIPcreateVarBasic( m_scip_master,
                &variable_p_i_t,                // pointer
                current_var_name.str().c_str(), // name
                0,                     // lowerbound
                +SCIPinfinity(m_scip_master),                             // upperbound
                costs_prop[i_unit],              // coeff in obj function
                SCIP_VARTYPE_CONTINUOUS));      // type

            // Adding the variable to the problem and the var matrix
            SCIP_CALL( SCIPaddVar(m_scip_master, variable_p_i_t));
            variable_p_i.push_back(variable_p_i_t);
        }
        m_variables_p.push_back( variable_p_i );
    }

    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationMasterTimeDecomposition2::create_constraints()
{
    // creating the constraints
    ostringstream current_cons_name;
    int number_of_units( m_instance_ucp->get_units_number() );
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    
    
    //convexity
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


    // min uptime   
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


    // downtime
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


    // switch

    // first time step
    vector<int> initial_state( m_instance_ucp->get_initial_state() );
    m_constraints_switch.resize( number_of_units );
    for(int i_unit = 0; i_unit < number_of_units; i_unit++)
    {
        m_constraints_switch[i_unit].resize( number_of_time_steps );
        SCIP_CONS* cons_switch_i_0;
        current_cons_name.str("");
        current_cons_name << "cons_switch_" << i_unit << "_0";

        SCIP_CALL(SCIPcreateConsLinear(m_scip_master, 
            &cons_switch_i_0,                  /* constraint pointer */ 
            current_cons_name.str().c_str(),    /* constraint name */
            0,                                  /* number of variable added */
            nullptr,                            /* array of variable */
            nullptr,                            /* array of coefficient */
            -SCIPinfinity(m_scip_master),              /* LHS */
            +initial_state[i_unit],
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

        SCIP_CALL( SCIPaddCoefLinear( m_scip_master,
            cons_switch_i_0,
            m_variables_u[i_unit][0],                           /* variable to add */
            -1));                                    /* coefficient */

        SCIP_CALL( SCIPaddCons( m_scip_master, cons_switch_i_0));
        m_constraints_switch[i_unit][0] = cons_switch_i_0 ;
    }

    // rest of the time steps
    for(int i_unit = 0; i_unit < number_of_units; i_unit++)
    {
        vector<SCIP_CONS*> cons_switch_i;
        
        for(int i_time_step = 1; i_time_step < number_of_time_steps; i_time_step++)
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
                -SCIPinfinity(m_scip_master),       /* LHS */
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
            SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
                cons_switch_i_t,
                m_variables_u[i_unit][i_time_step],      /* variable to add */
                -1));                                   /* coefficient */
            SCIP_CALL( SCIPaddCons( m_scip_master, cons_switch_i_t));
            m_constraints_switch[i_unit][i_time_step] = cons_switch_i_t ;
        }
    }


    // Pmax
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
                m_variables_p[i_unit][i_time_step],                           /** variable to add */
                - 1.                                    /** coefficient */
            );
            // add the constraint
            SCIP_CALL( SCIPaddCons(m_scip_master, cons_pmax_i_t));
            cons_pmax_i.push_back(cons_pmax_i_t);
        }
        m_constraints_pmax.push_back(cons_pmax_i);
    }


    // Pmin

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
                m_variables_p[i_unit][i_time_step],                           /** variable to add */
                1. ));                                    /** coefficient */

            // add the constraint
            SCIP_CALL( SCIPaddCons(m_scip_master, cons_pmin_i_t));
            cons_pmin_i.push_back(cons_pmin_i_t);
        }
        m_constraints_pmin.push_back(cons_pmin_i);
    }


    // Demand
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
                m_variables_p[i_unit][i_time_step],  /* variable to add */
                1.));                               /* coefficient */
        }
        SCIPaddCons(m_scip_master, demand_constraint_t);
        m_constraints_demand.push_back(demand_constraint_t);
    }


    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationMasterTimeDecomposition2::create_and_add_first_columns()
{

    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    vector<int> initial_states( m_instance_ucp->get_initial_state());
    
    ostringstream current_column_name;

    //* we create a first plan for each block to be able to solve the master problem
    // first time step : we need to switch the units that are not on
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

        }

        // then we create the production plan and the master variable

        ProductionPlan* production_plan_block = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
        add_column( production_plan_block, true, i_time_step );
    }

    return( SCIP_OKAY );

}


 
 
//* usefull functions


SCIP_RETCODE FormulationMasterTimeDecomposition2::add_column( ProductionPlan* plan_of_new_column, bool initialization, int block_number ) 
{
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    int number_of_units( m_instance_ucp->get_units_number());

    vector<vector<double>> up_down_plan( plan_of_new_column->get_up_down_plan() );

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
                1.  /* coefficient */
    );                


    
    // switch
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        // u_i_t
          SCIP_CALL( SCIPaddCoefLinear( m_scip_master,
            m_constraints_switch[i_unit][ block_number ], 
            new_scip_variable,          /* variable to add */
            up_down_plan[i_unit][block_number]        /* coefficient */
        ));
        // u_i_t+1 (if t is not the last time step)
        if( block_number < number_of_time_steps - 1)
        {
            SCIP_CALL( SCIPaddCoefLinear( m_scip_master,
                m_constraints_switch[i_unit][block_number +1], 
                new_scip_variable,          /* variable to add */
                - up_down_plan[i_unit][block_number]        /* coefficient */
            ));
        }
    }

    // min uptime
    vector<int> min_uptime( m_instance_ucp->get_min_uptime() );
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
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


    // production constraints

    vector<int> production_max( m_instance_ucp->get_production_max());
    vector<int> production_min( m_instance_ucp->get_production_min());

    for( int i_unit = 0; i_unit < number_of_units; i_unit++ )
    {
        // pmax
        SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
            m_constraints_pmax[i_unit][block_number],
            new_scip_variable,                                          /* variable to add */
            + production_max[i_unit] * up_down_plan[i_unit][block_number] ));       /* coefficient */
        // pmin
        SCIP_CALL( SCIPaddCoefLinear(m_scip_master,
            m_constraints_pmin[i_unit][block_number],
            new_scip_variable,                                          /* variable to add */
            - production_min[i_unit] * up_down_plan[i_unit][block_number] ));       /* coefficient */
    }

    return( SCIP_OKAY );

}


 
ProductionPlan* FormulationMasterTimeDecomposition2::get_production_plan_from_solution()
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

    // for each column, we add its plan value to the base plan, times it's coefficient. 
    // here : x and p
    for(int i_column = 0; i_column < number_columns; i_column ++)
    {
        // get the plan with all the corresponding informations, and the coefficient
        VariableMaster* current_variable = m_vector_columns[i_column];
        ProductionPlan* current_plan = current_variable->get_production_plan();
        int current_block = current_variable->get_block_number();
        vector<vector< double >> current_up_down_plan( current_plan->get_up_down_plan() );
        vector<vector< double >> current_quantity_plan( current_plan->get_quantity_plan() );
        SCIP_Real coefficient_column( SCIPgetSolVal( m_scip_master, solution, current_variable->get_variable_pointer()));

        // add the value to the main plan
        for( int i_unit = 0; i_unit < number_of_units; i_unit++ )
        {
            up_down_plan[i_unit][current_block] += current_up_down_plan[i_unit][current_block]*coefficient_column;
        }
    }

    // add the switch variable from the master plan
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            switch_plan[i_unit][i_time_step] +=  SCIPgetSolVal( m_scip_master, solution, m_variables_u[i_unit][i_time_step]);
        }
    }

    // add the production variable from the master plan
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            quantity_plan[i_unit][i_time_step] +=  SCIPgetSolVal( m_scip_master, solution, m_variables_p[i_unit][i_time_step]);
        }
    }
    // we can now create the plan
    ProductionPlan* production_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    production_plan->compute_cost();

    return( production_plan );
}


//* gets


SCIP_CONS** FormulationMasterTimeDecomposition2::get_constraint_convexity( int number_block )
{
    return( &m_constraints_convexity[number_block]);
}

SCIP_Cons** FormulationMasterTimeDecomposition2::get_constraint_min_uptime( int number_unit, int number_time_step )
{
    return( &m_constraints_min_uptime[number_unit][number_time_step]);
}

SCIP_Cons** FormulationMasterTimeDecomposition2::get_constraint_min_downtime( int number_unit, int number_time_step )
{
    return( &m_constraints_min_downtime[number_unit][number_time_step]);
}

SCIP_Cons** FormulationMasterTimeDecomposition2::get_constraint_switch( int number_unit, int number_time_step )
{
    return( &m_constraints_switch[number_unit][number_time_step]);
}


SCIP_Cons** FormulationMasterTimeDecomposition2::get_constraint_pmax( int number_unit, int number_time_step )
{
    return( &m_constraints_pmax[number_unit][number_time_step]);
}


SCIP_Cons** FormulationMasterTimeDecomposition2::get_constraint_pmin( int number_unit, int number_time_step )
{
    return( &m_constraints_pmin[number_unit][number_time_step]);
}


int FormulationMasterTimeDecomposition2::get_columns_number()
{
    return(m_vector_columns.size());
}
