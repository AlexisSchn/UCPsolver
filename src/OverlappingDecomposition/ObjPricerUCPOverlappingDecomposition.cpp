/** 
 * @file
 * Implement the class ObjPricerUCPOverlappingDecomposition 
*/


//** Includes

//* Standart
#include <vector>


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
#include "OverlappingDecomposition/FormulationMasterOverlappingDecomposition.h"
#include "OverlappingDecomposition/FormulationPricerOverlappingDecompositionUnit.h"
#include "OverlappingDecomposition/FormulationPricerOverlappingDecompositionTime.h"
#include "OverlappingDecomposition/ObjPricerUCPOverlappingDecomposition.h"


//** Namespaces

using namespace std;
using namespace scip;


/** constructor */
ObjPricerUCPOverlappingDecomposition::ObjPricerUCPOverlappingDecomposition(
    SCIP* scip_master,       /**< SCIP pointer */
    const char* name,               /**< name of pricer */
    FormulationMasterOverlappingDecomposition* formulation_master,
    InstanceUCP* instance_ucp,
    double coefficient_repartition_fixed,
    double coefficient_repartition_prop
): 
    ObjPricer(scip_master, name, "Pricer", 0, TRUE),
    m_formulation_master( formulation_master ),
    m_instance_ucp( instance_ucp ),
    m_coefficient_repartition_fixed( coefficient_repartition_fixed ),
    m_coefficient_repartition_prop( coefficient_repartition_prop )
{
    m_total_solving_time_master = 0;
    m_total_solving_time_pricer = 0;
    m_nb_iterations = 0;
    m_time = clock();
 
    // m_primal_bounds.push_back( 5. );
    // cerr << "primal bound : " << m_primal_bounds[0] << endl;

    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );    
    int number_of_units( m_instance_ucp->get_units_number() );    

    //* pricing problems initialization
  
    // unit 
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++ )
    { 
        SCIP* scip_pricer(0);
        SCIPcreate( &scip_pricer ) ;
        SCIPincludeDefaultPlugins( scip_pricer );
        SCIPcreateProb(scip_pricer, "UCP_PRICER_PROBLEM", 0, 0, 0, 0, 0, 0, 0);
        SCIPsetIntParam(scip_pricer, "display/verblevel", 0);
        FormulationPricerOverlappingDecompositionUnit *formulation_pricer = new FormulationPricerOverlappingDecompositionUnit( 
            m_instance_ucp, 
            scip_pricer, 
            i_unit,
            m_coefficient_repartition_fixed,
            m_coefficient_repartition_prop
        );
        m_pricer_unit_problems.push_back( formulation_pricer );
    }

    // time
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {   
        SCIP* scip_pricer(0);
        SCIPcreate( &scip_pricer ) ;
        SCIPincludeDefaultPlugins( scip_pricer );
        SCIPcreateProb(scip_pricer, "UCP_PRICER_PROBLEM", 0, 0, 0, 0, 0, 0, 0);
        SCIPsetIntParam(scip_pricer, "display/verblevel", 0);
        FormulationPricerOverlappingDecompositionTime *formulation_pricer = new FormulationPricerOverlappingDecompositionTime( m_instance_ucp, 
            scip_pricer, 
            i_time_step,
            m_coefficient_repartition_fixed,
            m_coefficient_repartition_prop
        );
        m_pricer_time_problems.push_back( formulation_pricer );
    }
 
 
    //* reduced costs initialization
    m_reduced_costs_convexity_time.resize( number_of_time_steps );
    m_reduced_costs_convexity_unit.resize( number_of_units );
    m_reduced_costs_variable_splitting.resize( number_of_units );
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++ )
    {
        m_reduced_costs_variable_splitting[i_unit].resize( number_of_time_steps );
    }

}


/** destructor */
ObjPricerUCPOverlappingDecomposition::~ObjPricerUCPOverlappingDecomposition()
{
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );    
    int number_of_units( m_instance_ucp->get_units_number() );    
    
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++ )
    {
        free( m_pricer_unit_problems[i_unit] );
    }
    
    for( int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++ )
    {
        free( m_pricer_time_problems[i_time_step] );
    }
}


/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerUCPOverlappingDecomposition::scip_init)
{
    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());

    // Convexity constraints units
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIP_CALL( SCIPgetTransformedCons( scip, 
                *m_formulation_master->get_constraint_convexity_unit( i_unit ),
                m_formulation_master->get_constraint_convexity_unit( i_unit ) 
        ) );
    }

    // Convexity constraints time
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
        SCIP_CALL( SCIPgetTransformedCons( scip, 
                *m_formulation_master->get_constraint_convexity_time( i_time_step ),
                m_formulation_master->get_constraint_convexity_time( i_time_step ) 
        ) );
    }



    // variable splitting constraints
    for( int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            SCIP_CALL( SCIPgetTransformedCons( scip, 
                *m_formulation_master->get_constraint_variable_splitting( i_unit, i_time_step ),
                m_formulation_master->get_constraint_variable_splitting( i_unit, i_time_step ) 
        ) );
        }
    }


    return SCIP_OKAY;
}



SCIP_DECL_PRICERREDCOST(ObjPricerUCPOverlappingDecomposition::scip_redcost)
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

 

SCIP_RETCODE ObjPricerUCPOverlappingDecomposition::ucp_pricing(SCIP* scip)
{
 
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    int number_of_units( m_instance_ucp->get_units_number());
    double master_value(SCIPgetPrimalbound( scip ) );
    // cerr << SCIPgetPrimalbound( scip ) << endl;
    m_nb_iterations += 1;
    //m_primal_bounds.push_back( SCIPgetPrimalbound( scip ) );
    
    double lagrangian_bound_iter( 0. );
 
    //* get the new reduced costs
     
    SCIP_CONS* current_constraint(0);

    // convexity constraints for unit
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        current_constraint = *m_formulation_master->get_constraint_convexity_unit(i_unit);
        m_reduced_costs_convexity_unit[i_unit] = SCIPgetDualsolLinear( scip, current_constraint );
        // lagrangian_bound_iter += SCIPgetDualsolLinear( scip, current_constraint );
    }

    // convexity constraints for time
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {
        current_constraint = *m_formulation_master->get_constraint_convexity_time(i_time_step);
        m_reduced_costs_convexity_time[i_time_step] = SCIPgetDualsolLinear( scip, current_constraint );
        // lagrangian_bound_iter += SCIPgetDualsolLinear( scip, current_constraint );
    }
    
    // convexity constraints for variable splitting
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
        {
            current_constraint = *m_formulation_master->get_constraint_variable_splitting(i_unit, i_time_step);
            m_reduced_costs_variable_splitting[i_unit][i_time_step] = SCIPgetDualsolLinear( scip, current_constraint );
        }

    } 

    //*  Pricing for units
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {   

        FormulationPricerOverlappingDecompositionUnit* current_pricer = m_pricer_unit_problems[i_unit];
        current_pricer->change_reduced_costs(
            m_reduced_costs_variable_splitting[i_unit]
        );
        SCIP* scip_pricer = current_pricer->get_scip_pointer();
        m_time = clock();
        SCIPsolve( scip_pricer );
        m_total_solving_time_pricer += float( clock() - m_time ) /  CLOCKS_PER_SEC ;
        

        //* if a plan is found, create and add the column, else, do nothing, which will make the column generation stop
        SCIP_Real optimal_value(SCIPgetPrimalbound( scip_pricer ) );
        if( optimal_value < m_reduced_costs_convexity_unit[i_unit] -0.0001 )
        {
            //* create the plan
            ProductionPlan* new_plan = current_pricer->get_production_plan_from_solution();
            m_formulation_master->add_column_unit( new_plan, false, i_unit );
        }

        lagrangian_bound_iter += optimal_value;

        SCIPfreeTransform( scip_pricer ) ;
        
    }

    //*  pricing for time
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
    {   
        FormulationPricerOverlappingDecompositionTime* current_pricer = m_pricer_time_problems[i_time_step];
        current_pricer->change_reduced_costs(
            m_reduced_costs_variable_splitting
        );
        SCIP* scip_pricer = current_pricer->get_scip_pointer();
        m_time = clock();
        SCIPsolve( scip_pricer );
        m_total_solving_time_pricer += float( clock() - m_time ) /  CLOCKS_PER_SEC ;
        

        //* if a plan is found, create and add the column, else, do nothing, which will make the column generation stop
        SCIP_Real optimal_value(SCIPgetPrimalbound( scip_pricer ) );

        if( optimal_value < m_reduced_costs_convexity_time[i_time_step] -0.0001 )
        {
            //* create the plan
            ProductionPlan* new_plan = current_pricer->get_production_plan_from_solution();
            m_formulation_master->add_column_time( new_plan, false, i_time_step );
        }
        else if( m_formulation_master->get_columns_number_time() < 2 * number_of_time_steps)
        {
            //* This is ugly, but is due to special case :
            /**
             * if the up down plan from initialisation is the right one
             * then a column will not be added for it in the column generation
             * despite the quantity plan being bad
             */
            ProductionPlan* new_plan =  current_pricer->get_production_plan_from_solution();
            m_formulation_master->add_column_time( new_plan, false, i_time_step );
        }

        lagrangian_bound_iter += optimal_value;
        
        SCIPfreeTransform( scip_pricer ) ;

    }
    

    // cerr << lagrangian_bound_iter << endl;
    // m_lagrangian_bounds.push_back( lagrangian_bound_iter );

    return( SCIP_OKAY );
    
}