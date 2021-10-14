/** 
 * @file
 * Implement the class ObjPricerUCPTimeDecomposition 
*/


//** Includes

//* Standart
#include <vector>
#include <ctime>

//* SCIP
#include "objscip/objpricer.h"
#include "scip/pub_var.h"
#include <scip/scip.h>


//* User 

// general
#include "DataClasses/InstanceUCP.h"

// Decomposition
#include "Decomposition/FormulationPricer.h"

// Unit Decomposition
#include "TimeDecomposition/FormulationMasterTimeDecomposition.h"
#include "TimeDecomposition/FormulationPricerTimeDecomposition.h"
#include "TimeDecomposition/ObjPricerUCPTimeDecomposition.h"


//** Namespaces

using namespace std;
using namespace scip;


/** constructor */
ObjPricerUCPTimeDecomposition::ObjPricerUCPTimeDecomposition(
    SCIP* scip_master,       /**< SCIP pointer */
    const char* name,               /**< name of pricer */
    FormulationMasterTimeDecomposition* formulation_master,
    InstanceUCP* instance_ucp
):
    ObjPricer(scip_master, name, "Pricer", 0, TRUE)
{
    m_formulation_master = formulation_master;
    m_instance_ucp = instance_ucp;
    m_nb_iterations = 0;
    m_total_solving_time_master = 0;
    m_total_solving_time_pricer = 0;
    m_time_1 = 0;
    m_time = clock();

    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    int number_of_units( m_instance_ucp->get_units_number());


    //* initialize the pricer problem

    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {   
        SCIP* scip_pricer(0);
        SCIPcreate( &scip_pricer ) ;
        SCIPincludeDefaultPlugins( scip_pricer );
        SCIPcreateProb(scip_pricer, "UCP_PRICER_PROBLEM", 0, 0, 0, 0, 0, 0, 0);
        SCIPsetIntParam(scip_pricer, "display/verblevel", 0);
        FormulationPricerTimeDecomposition* formulation_pricer = new FormulationPricerTimeDecomposition( m_instance_ucp, 
            scip_pricer,
            i_time_step
        );
        m_pricer_problems.push_back( formulation_pricer );
    }

    //* initialize the reduced costs vectors

    // convexity constraints
    m_reduced_costs_convexity.resize( number_of_time_steps, 0. );
  
    // switch constraints

    m_reduced_costs_switch.resize( number_of_units );
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {    
        m_reduced_costs_switch[ i_unit ].resize( number_of_time_steps, 0.);
    }
  
    // min uptime constraints
    m_reduced_costs_min_uptime.resize( number_of_units );
    vector<int> min_uptime( m_instance_ucp->get_min_uptime() );
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        m_reduced_costs_min_uptime[i_unit].resize( number_of_time_steps - min_uptime[i_unit] + 1  , 0. );
    } 
 
    // min downtime constraints
    m_reduced_costs_min_downtime.resize( number_of_units );
    vector<int> min_downtime = m_instance_ucp->get_min_downtime();
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        m_reduced_costs_min_downtime[i_unit].resize( number_of_time_steps - min_downtime[i_unit], 0.);
    }
    

}


/** destructor */
ObjPricerUCPTimeDecomposition::~ObjPricerUCPTimeDecomposition()
{}


/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerUCPTimeDecomposition::scip_init)
{
    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());

    // convexity constraints
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
            SCIP_CALL( SCIPgetTransformedCons( scip, 
                    *m_formulation_master->get_constraint_convexity( i_time_step ),
                    m_formulation_master->get_constraint_convexity( i_time_step ) 
            ) );
    }

    // switch constraints
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_CALL( SCIPgetTransformedCons( scip, 
                    *m_formulation_master->get_constraint_switch( i_unit, i_time_step ),
                    m_formulation_master->get_constraint_switch( i_unit, i_time_step ) 
            ) );
        }
    }

    // min uptime constraints
    vector<int> min_uptime( m_instance_ucp->get_min_uptime() );
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = min_uptime[i_unit] - 1; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_CALL( SCIPgetTransformedCons( scip, 
                    *m_formulation_master->get_constraint_min_uptime( i_unit, i_time_step - min_uptime[i_unit] + 1),
                    m_formulation_master->get_constraint_min_uptime( i_unit, i_time_step - min_uptime[i_unit] + 1) 
            ) );
        }    
    }
 
    // min downtime constraints 
    vector<int> min_downtime = m_instance_ucp->get_min_downtime();
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps - min_downtime[i_unit]; i_time_step ++)
        {
            SCIP_CALL( SCIPgetTransformedCons( scip, 
                    *m_formulation_master->get_constraint_min_downtime( i_unit, i_time_step ),
                    m_formulation_master->get_constraint_min_downtime( i_unit, i_time_step ) 
            ) );
        }
    }

    return(SCIP_OKAY);
}
 
SCIP_DECL_PRICERREDCOST(ObjPricerUCPTimeDecomposition::scip_redcost)
{
    /* set result pointer */
    *result = SCIP_SUCCESS;

    /* call pricing routine */
    SCIP_RETCODE pricing_result = SCIP_OKAY;
    pricing_result = ucp_pricing(scip);
    if( pricing_result != SCIP_OKAY)
    {
        *result = SCIP_DIDNOTRUN;
    }

    return SCIP_OKAY;
} 

 
 
SCIP_RETCODE ObjPricerUCPTimeDecomposition::ucp_pricing(SCIP* scip)
{
    // cerr <<  SCIPgetPrimalbound( scip )  << endl;
    // m_master_values.push_back(SCIPgetPrimalbound(scip));
    // SCIPprintBestSol(scip, NULL, FALSE);
    m_nb_iterations += 1;

    m_total_solving_time_master += float( clock () - m_time ) /  CLOCKS_PER_SEC ;

 
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    int number_of_units( m_instance_ucp->get_units_number());

    //* get the reduced costs
  
    SCIP_CONS* current_constraint(0);

    // convexity constraints
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
        current_constraint = *m_formulation_master->get_constraint_convexity(i_time_step);
        m_reduced_costs_convexity[i_time_step] = SCIPgetDualsolLinear( scip, current_constraint );
    }
    

  
    // switch constraints

    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {    
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            current_constraint = *m_formulation_master->get_constraint_switch(i_unit, i_time_step);
            m_reduced_costs_switch[i_unit][i_time_step] =  SCIPgetDualsolLinear( scip, current_constraint );
        }
    }
 
    // min uptime constraints
    vector<int> min_uptime( m_instance_ucp->get_min_uptime() );
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = min_uptime[i_unit] - 1; i_time_step < number_of_time_steps; i_time_step ++)
        {
            current_constraint = *m_formulation_master->get_constraint_min_uptime(i_unit, i_time_step - min_uptime[i_unit] + 1);
            m_reduced_costs_min_uptime[i_unit][i_time_step - min_uptime[i_unit] + 1] = SCIPgetDualsolLinear( scip, current_constraint );
        }
    } 
 
    // min downtime constraints
    vector<int> min_downtime = m_instance_ucp->get_min_downtime();
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps - min_downtime[i_unit]; i_time_step ++)        
        {
            current_constraint = *m_formulation_master->get_constraint_min_downtime(i_unit, i_time_step  );
            m_reduced_costs_min_downtime[i_unit][i_time_step] = SCIPgetDualsolLinear( scip, current_constraint );
        }
    }
    
    //*  create and solve the pricing problems with the reduced values
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {   
        m_time = clock();
        FormulationPricerTimeDecomposition* current_pricer = m_pricer_problems[i_time_step];
        current_pricer->change_reduced_costs(
            m_reduced_costs_switch,
            m_reduced_costs_min_uptime,
            m_reduced_costs_min_downtime
        );
        SCIP* scip_pricer = current_pricer->get_scip_pointer();
        m_time = clock();
        SCIPsolve( scip_pricer );
        m_total_solving_time_pricer += float( clock() - m_time ) /  CLOCKS_PER_SEC ;
        // SCIPprintBestSol(scip_pricer, NULL, FALSE) ;

        //* if a plan is found, create and add the column, else, do nothing, which will make the column generation stop
        SCIP_Real optimal_value(SCIPgetPrimalbound( scip_pricer ) );
        if( optimal_value < m_reduced_costs_convexity[i_time_step] -0.0001 )
        {
            //* create the plan
            ProductionPlan* new_plan = current_pricer->get_production_plan_from_solution();
            m_formulation_master->add_column( new_plan, false, i_time_step );
        }
        else if( m_formulation_master->get_columns_number() < 2 * number_of_time_steps)
        {
            //* This is ugly, but is due to special case :
            /**
             * if the up down plan from initialisation is the right one
             * then a column will not be added for it in the column generation
             * despite the quantity plan being bad
             */
            ProductionPlan* new_plan = current_pricer->get_production_plan_from_solution();
            m_formulation_master->add_column( new_plan, false, i_time_step );
        }
        
        SCIPfreeTransform( scip_pricer ) ;
    }
    

    m_time = clock();

    return( SCIP_OKAY );
    
}




