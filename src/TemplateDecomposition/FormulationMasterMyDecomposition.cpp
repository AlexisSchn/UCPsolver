
/**
 * 
 *  Implements the class FormulationMasterMyDecomposition
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

// MyDecomposition
#include "MyDecomposition/FormulationMasterMyDecomposition.h"

//** Relaxation
using namespace std;



//* class functions

FormulationMasterMyDecomposition::FormulationMasterMyDecomposition( InstanceUCP* instance, SCIP* scip_master):
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



FormulationMasterMyDecomposition::~FormulationMasterMyDecomposition()
{
    SCIPsetMessagehdlrQuiet( m_scip_master, 1);
    SCIPfree(&m_scip_master);
}



//* Initilization of the problem

SCIP_RETCODE FormulationMasterMyDecomposition::create_variables()
{

    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    int number_of_units( m_instance_ucp->get_units_number());
    ostringstream current_var_name;

    // create the variables

    //! fillVariables p


    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationMasterMyDecomposition::create_constraints()
{
    // creating the constraints
    ostringstream current_cons_name;

    int number_of_units( m_instance_ucp->get_units_number() );
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );

    //! fill

    return( SCIP_OKAY );
}


SCIP_RETCODE FormulationMasterMyDecomposition::create_and_add_first_columns()
{

    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    
    ostringstream current_column_name;

    //* we create a bad first plan, but working, for each block !
    for( int i in block...)
    {
        // !fill
        // then we create the production plan and the master variable

        ProductionPlan* production_plan_block = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
        add_column( production_plan_block, true, i_unit );
    }    

    return( SCIP_OKAY );

}




//* usefull functions


SCIP_RETCODE FormulationMasterMyDecomposition::add_column( ProductionPlan* plan_of_new_column, bool initialization, int block_number ) 
{
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );
    int number_of_units( m_instance_ucp->get_units_number());

    //* create the scip variable
    string column_name = "column_" + to_string(m_vector_columns.size()); 
    SCIP_VAR* new_scip_variable;

    //! fill
    SCIPcreateVar(  m_scip_master,
        &new_scip_variable,                            // pointer 
        column_name.c_str(),                            // name
        0.,                                     // lowerbound
        +SCIPinfinity(m_scip_master),            // upperbound
        ,          // coeff in obj function
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
    // new_column->add_block_number( block_number );
    m_vector_columns.push_back( new_column );


    //* add column to constraints
    // convexity
    SCIPaddCoefLinear(m_scip_master,
                m_constraints_convexity[block_number],
                new_scip_variable,  /* variable to add */
                1.
    );         /* coefficient */        

    // other constraints : 

    //! fill

    
    return( SCIP_OKAY );

}



ProductionPlan* FormulationMasterMyDecomposition::get_production_plan_from_solution()
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
        //! fill
    }
 
    // we can now create the plan
    ProductionPlan* production_plan = new ProductionPlan( m_instance_ucp, up_down_plan, switch_plan, quantity_plan );
    production_plan->compute_cost();

    return( production_plan );
}


//* gets

//! fill with the get_constraints needed

int FormulationMasterMyDecomposition::get_columns_number()
{
    return(m_vector_columns.size());
}
