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
#include "SwitchDecomposition2/FormulationPricerSwitchDecomposition2.h"


//** Namespaces
using namespace std;



FormulationPricerSwitchDecomposition2::FormulationPricerSwitchDecomposition2(InstanceUCP *instance, 
    SCIP *scip,
    int block_number,
    double coefficient_repartition
    ) :
    FormulationPricer( instance, scip ), 
    m_block_number( block_number ),
    m_coefficient_repartition( coefficient_repartition )
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
SCIP_RETCODE FormulationPricerSwitchDecomposition2::create_variables()
{
    ostringstream current_var_name;
    int number_of_time_steps = m_instance_ucp->get_time_steps_number();
    int number_of_units = m_instance_ucp->get_units_number();


    //* x
    // for the block_number timestep
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        current_var_name.str("");
        current_var_name << "x_" << i_unit << "_" << m_block_number;
        SCIP_VAR* variable_x_i_t;
    
        // create the variable
        SCIPcreateVarBasic(  m_scip_pricer,
            &variable_x_i_t,                    // pointer 
            current_var_name.str().c_str(),     // name
            0.,                                 // lowerbound
            1.,                                 // upperbound
            0. ,                       // coeff in obj function
            SCIP_VARTYPE_BINARY                 // type
        );
        SCIP_CALL(SCIPaddVar(m_scip_pricer, variable_x_i_t));
        m_variables_x_current.push_back(variable_x_i_t);
    }


    // for the block_number -1 timestep (if block_number > 0 )
    
    if( m_block_number > 0 )
    {
        for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
        {
            current_var_name.str("");
            current_var_name << "x_" << i_unit << "_" << m_block_number-1;
            SCIP_VAR* variable_x_i_t;

            // create the variable
            SCIPcreateVarBasic(  m_scip_pricer,
                &variable_x_i_t,                    // pointer 
                current_var_name.str().c_str(),     // name
                0.,                                 // lowerbound
                1.,                                 // upperbound
                0. ,                       // coeff in obj function
                SCIP_VARTYPE_BINARY                 // type
            );
            SCIP_CALL(SCIPaddVar(m_scip_pricer, variable_x_i_t));
            m_variables_x_previous.push_back(variable_x_i_t);
        }
    }
    

    //* p
    // for the block_number timestep
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
        m_variables_p_current.push_back(variable_p_i_t);
    }

    // for the block_number -1 timestep (if block_number > 0 )
    if( m_block_number > 0 )
    {
        for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
        {
            SCIP_VAR* variable_p_i_t;
            current_var_name.str("");
            current_var_name << "p_" << i_unit << "_" << m_block_number+1 ;

            // Creation of the variable
            SCIP_CALL( SCIPcreateVarBasic( m_scip_pricer,
                &variable_p_i_t,                // pointer
                current_var_name.str().c_str(), // name
                0,                     // lowerbound
                + SCIPinfinity( m_scip_pricer ),                              // upperbound
                0.,              // coeff in obj function
                SCIP_VARTYPE_CONTINUOUS));      // type

            // Adding the variable to the problem and the var matrix
            SCIP_CALL( SCIPaddVar(m_scip_pricer, variable_p_i_t));
            m_variables_p_previous.push_back(variable_p_i_t);
        }
    }

    //* u

    m_variables_u.reserve( number_of_units );
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIP_VAR* variable_u_t;
        current_var_name.str("");
        current_var_name << "u_" << m_block_number ;

        // Creation of the variable
        SCIP_CALL( SCIPcreateVarBasic( m_scip_pricer,
            &variable_u_t,                  // pointer
            current_var_name.str().c_str(), // name
            0.,                             // lowerbound
            1.,                             // upperbound
            0.,                   // coeff in obj function
            SCIP_VARTYPE_BINARY));          // type

        // Adding the variable to the problem and the var matrix
        SCIP_CALL( SCIPaddVar(m_scip_pricer, variable_u_t));
        m_variables_u.push_back(variable_u_t);
    }    


    return( SCIP_OKAY );
}


/* create all the constraints, and add them to the scip object and formulation object */
SCIP_RETCODE FormulationPricerSwitchDecomposition2::create_constraints()
{
    ostringstream current_cons_name;
    int number_of_time_steps = m_instance_ucp->get_time_steps_number();
    int number_of_units = m_instance_ucp->get_units_number();

    //* demand
    // for the block_number timestep

    SCIP_CONS* cons_demand_t;
    current_cons_name.str("");
    current_cons_name << "cons_demand_" << m_block_number ;
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
            m_variables_p_current[i_unit],  /* variable to add */
            1.));                               /* coefficient */
    }
    SCIP_CALL( SCIPaddCons(m_scip_pricer, cons_demand_t));


    // for the block_number -1 timestep (if block_number > 0 )
    if( m_block_number > 0 )
    {
        SCIP_CONS* cons_demand_t_previous;
        current_cons_name.str("");
        current_cons_name << "cons_demand_" << m_block_number-1 ;
        // creating the constraint
        SCIP_CALL(SCIPcreateConsBasicLinear(m_scip_pricer, 
            &cons_demand_t_previous,                     /* constraint pointer */ 
            current_cons_name.str().c_str(),    /* constraint name */
            0,                                  /* number of variable added */
            nullptr,                            /* array of variable */
            nullptr,                            /* array of coefficient */
            m_instance_ucp->get_demand()[m_block_number - 1 ],         /* LHS */
            +SCIPinfinity(m_scip_pricer)));              /* RHS */        
        // adding the variable
        for(int i_unit = 0; i_unit < number_of_units; i_unit++)
        {
            SCIP_CALL( SCIPaddCoefLinear(m_scip_pricer,
                cons_demand_t_previous,
                m_variables_p_previous[i_unit],  /* variable to add */
                1.));                               /* coefficient */
        }
        SCIP_CALL( SCIPaddCons(m_scip_pricer, cons_demand_t_previous));
    }


    //* production

    // for the block_number timestep
    
    // p < pmax
    vector<int> maximum_production( m_instance_ucp->get_production_max());
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIP_CONS* cons_pmax_i_t;
        current_cons_name.str("");
        current_cons_name << "cons_p" << i_unit << "_" << m_block_number;

        // create the constraint
        SCIP_CALL(SCIPcreateConsBasicLinear( m_scip_pricer, 
            &cons_pmax_i_t,                      /** constraint pointer */ 
            current_cons_name.str().c_str(),             /** constraint name */
            0,                                  /** number of variable added */
            nullptr,                            /** array of variable */
            nullptr,                            /** array of coefficient */
            -SCIPinfinity(m_scip_pricer),                /** LHS */
            0));                                /** RHS */

        // add the variables
        SCIP_CALL( SCIPaddCoefLinear(m_scip_pricer,
            cons_pmax_i_t,
            m_variables_x_current[i_unit],                           /** variable to add */
            - maximum_production[i_unit] ));     /* coefficient */
        SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
            cons_pmax_i_t,
            m_variables_p_current[i_unit],                           /** variable to add */
            1));                                    /** coefficient */

        // add the constraint
        SCIP_CALL( SCIPaddCons(m_scip_pricer, cons_pmax_i_t));
    }


    // pmin < p

    vector<int> minimum_production( m_instance_ucp->get_production_min() );
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
            m_variables_x_current[i_unit],                           /** variable to add */
            - minimum_production[i_unit] ));     /* coefficient */
        SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
            cons_pmin_i_t,
            m_variables_p_current[i_unit],                           /** variable to add */
            1));                                    /** coefficient */

        // add the constraint
        SCIP_CALL( SCIPaddCons(m_scip_pricer, cons_pmin_i_t));
    }


    // for the block_number -1 timestep (if block_number > 0 )
    if( m_block_number > 0 )
    {
        // p < pmax
        for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
        {
            SCIP_CONS* cons_pmax_i_t;
            current_cons_name.str("");
            current_cons_name << "cons_p" << i_unit << "_" << m_block_number-1;

            // create the constraint
            SCIP_CALL(SCIPcreateConsBasicLinear( m_scip_pricer, 
                &cons_pmax_i_t,                      /** constraint pointer */ 
                current_cons_name.str().c_str(),             /** constraint name */
                0,                                  /** number of variable added */
                nullptr,                            /** array of variable */
                nullptr,                            /** array of coefficient */
                -SCIPinfinity(m_scip_pricer),                /** LHS */
                0));                                /** RHS */

            // add the variables
            SCIP_CALL( SCIPaddCoefLinear(m_scip_pricer,
                cons_pmax_i_t,
                m_variables_x_previous[i_unit],                           /** variable to add */
                - maximum_production[i_unit] ));     /* coefficient */
            SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
                cons_pmax_i_t,
                m_variables_p_previous[i_unit],                           /** variable to add */
                1));                                    /** coefficient */

            // add the constraint
            SCIP_CALL( SCIPaddCons(m_scip_pricer, cons_pmax_i_t));
        }


        // pmin < p

        for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
        {
            SCIP_CONS* cons_pmin_i_t;
            current_cons_name.str("");
            current_cons_name << "cons_pmin_" << i_unit << "_" << m_block_number-1;

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
                m_variables_x_previous[i_unit],                           /** variable to add */
                - minimum_production[i_unit] ));     /* coefficient */
            SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
                cons_pmin_i_t,
                m_variables_p_previous[i_unit],                           /** variable to add */
                1));                                    /** coefficient */

            // add the constraint
            SCIP_CALL( SCIPaddCons(m_scip_pricer, cons_pmin_i_t));
        }


    }


    //* switch constraints

    if( m_block_number == 0 )
    {
        vector<int> initial_state(m_instance_ucp->get_initial_state());
        for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
        {
            SCIP_CONS* cons_startup_i;  
            current_cons_name.str("");
            current_cons_name << "cons_startup_" << i_unit;

            SCIP_CALL(SCIPcreateConsBasicLinear(m_scip_pricer, 
                &cons_startup_i,                  /* constraint pointer */ 
                current_cons_name.str().c_str(),    /* constraint name */
                0,                                  /* number of variable added */
                nullptr,                            /* array of variable */
                nullptr,                            /* array of coefficient */
                -SCIPinfinity(m_scip_pricer),              /* LHS */
                +initial_state[ i_unit ] ));          /* RHS */
            SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
                cons_startup_i, 
                m_variables_x_current[i_unit],       /* variable to add */
                1));                /* coefficient */
            SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
                cons_startup_i,
                m_variables_u[i_unit],                           /* variable to add */
                -1));                                    /* coefficient */
            SCIP_CALL( SCIPaddCons( m_scip_pricer, cons_startup_i));
        }
    }
    else
    {
        for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
        {
            SCIP_CONS* cons_startup_i;
            current_cons_name.str("");
            current_cons_name << "cons_startup_" << i_unit;

            SCIP_CALL(SCIPcreateConsBasicLinear(m_scip_pricer, 
                &cons_startup_i,                  /* constraint pointer */ 
                current_cons_name.str().c_str(),    /* constraint name */
                0,                                  /* number of variable added */
                nullptr,                            /* array of variable */
                nullptr,                            /* array of coefficient */
                -SCIPinfinity(m_scip_pricer),              /* LHS */
                0. ));          /* RHS */
            SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
                cons_startup_i, 
                m_variables_x_current[ i_unit ],       /* variable to add */
                1));                /* coefficient */
            SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
                cons_startup_i, 
                m_variables_x_previous[ i_unit ],       /* variable to add */
                -1));                /* coefficient */
            SCIP_CALL( SCIPaddCoefLinear( m_scip_pricer,
                cons_startup_i,
                m_variables_u[ i_unit],                           /* variable to add */
                -1));                                    /* coefficient */
            SCIP_CALL( SCIPaddCons( m_scip_pricer, cons_startup_i));
        }
    }


    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationPricerSwitchDecomposition2::change_reduced_costs( 
    vector<vector<SCIP_Real>> reduced_costs_min_uptime,
    vector<vector<SCIP_Real>> reduced_costs_min_downtime,
    vector<vector<SCIP_Real>> reduced_costs_variable_splitting
)
{
    double coefficient( 0. );
    int number_of_time_steps = m_instance_ucp->get_time_steps_number();
    int number_of_units = m_instance_ucp->get_units_number();
    vector<int> fixed_costs( m_instance_ucp->get_costs_fixed() );
    vector<int> startup_costs( m_instance_ucp->get_costs_startup() );
    vector<int> min_uptime( m_instance_ucp->get_min_uptime() );
    vector<int> min_downtime( m_instance_ucp->get_min_downtime() );
    
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {

        // x current 

        // // coefficient calculation
        // coefficient = 0.;
        // coefficient += fixed_costs[i_unit];
 
        // // min uptime / downtime reduced costs
        // if( m_block_number >= min_uptime[i_unit] - 1)
        // {
        //     coefficient += reduced_costs_min_uptime[i_unit][m_block_number - min_uptime[i_unit] + 1];
        // }
        // if( m_block_number <= number_of_time_steps - min_downtime[i_unit] - 1 )
        // {
        //     coefficient -= reduced_costs_min_downtime[i_unit][m_block_number];
        // }
        // // variable splitting
        // if( m_block_number < number_of_time_steps - 1 )
        // {
        //     coefficient -= reduced_costs_variable_splitting[ i_unit ][ m_block_number ];
        // }
        // SCIP_CALL(SCIPchgVarObj(m_scip_pricer, m_variables_x_current[i_unit], coefficient));

        // // x previous
        // if( m_block_number > 0 )
        // {
        //     coefficient = 0. ;
        //     // coefficient += fixed_costs[ i_unit ];
        //     coefficient += reduced_costs_variable_splitting[ i_unit ][ m_block_number - 1];
        //     SCIP_CALL(SCIPchgVarObj(m_scip_pricer, m_variables_x_previous[i_unit], coefficient));
        // }

        // x current 

        // coefficient calculation
        coefficient = 0.;
        coefficient += ( 1 - m_coefficient_repartition ) * fixed_costs[i_unit];
        if( m_block_number == number_of_time_steps - 1 )
        {
            coefficient += m_coefficient_repartition * fixed_costs[i_unit];
        }

        // min uptime / downtime reduced costs
        if( m_block_number >= min_uptime[i_unit] - 1)
        {
            coefficient += reduced_costs_min_uptime[i_unit][m_block_number - min_uptime[i_unit] + 1];
        }
        if( m_block_number <= number_of_time_steps - min_downtime[i_unit] - 1 )
        {
            coefficient -= reduced_costs_min_downtime[i_unit][m_block_number];
        }
        // variable splitting
        if( m_block_number < number_of_time_steps - 1 )
        {
            coefficient -= reduced_costs_variable_splitting[ i_unit ][ m_block_number ];
        }
        SCIP_CALL(SCIPchgVarObj(m_scip_pricer, m_variables_x_current[i_unit], coefficient));

        // x previous
        if( m_block_number > 0 )
        {
            coefficient = 0. ;
            coefficient += m_coefficient_repartition * fixed_costs[ i_unit ];
            coefficient += reduced_costs_variable_splitting[ i_unit ][ m_block_number - 1];
            SCIP_CALL(SCIPchgVarObj(m_scip_pricer, m_variables_x_previous[i_unit], coefficient));
        }



        // u
        coefficient = 0.;
        coefficient += startup_costs[ i_unit ];
        for( int i_time_step = min_uptime[i_unit] - 1; i_time_step < number_of_time_steps; i_time_step ++ )
        {
            if( m_block_number <= i_time_step )
            {
                if( m_block_number >= i_time_step - min_uptime[ i_unit ] + 1) // ??
                {
                    coefficient -= reduced_costs_min_uptime[i_unit][ i_time_step - min_uptime[i_unit] + 1 ];
                }
            }
        }
        for( int i_time_step = min_downtime[i_unit]; i_time_step < number_of_time_steps; i_time_step ++ )
        {
            if( m_block_number >= i_time_step - min_downtime[ i_unit ] + 1)
            {
                if( m_block_number <= i_time_step)
                {
                    coefficient -= reduced_costs_min_downtime[i_unit][ i_time_step - min_downtime[i_unit] ];
                }
            }
        }  
        SCIP_CALL(SCIPchgVarObj(m_scip_pricer, m_variables_u[ i_unit ], coefficient));
 
    }

    return( SCIP_OKAY );

}



ProductionPlan* FormulationPricerSwitchDecomposition2::get_production_plan_from_solution()
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
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        up_down_plan[i_unit].resize(number_of_time_steps, 0);
        switch_plan[i_unit].resize(number_of_time_steps, 0);
        quantity_plan[i_unit].resize(number_of_time_steps, 0);

        up_down_plan[i_unit][m_block_number] = SCIPgetSolVal( m_scip_pricer, solution, m_variables_x_current[i_unit]);
        quantity_plan[i_unit][m_block_number] = SCIPgetSolVal( m_scip_pricer, solution, m_variables_p_current[i_unit]);
        switch_plan[i_unit][m_block_number] = SCIPgetSolVal( m_scip_pricer, solution, m_variables_u[i_unit]);

        if( m_block_number > 0 )
        {
            up_down_plan[i_unit][m_block_number - 1] = SCIPgetSolVal( m_scip_pricer, solution, m_variables_x_previous[i_unit]);
            quantity_plan[i_unit][m_block_number - 1] = SCIPgetSolVal( m_scip_pricer, solution, m_variables_p_previous[i_unit]);
        }
    }
    

    // create the plan object

    ProductionPlan* new_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    new_plan->compute_cost();

    return( new_plan );
}





// * gets 


int FormulationPricerSwitchDecomposition2::get_block_number()
{
    return( m_block_number );
}

