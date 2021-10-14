/** 
 * @file
 * Implement the class ObjPricerUCPUnitDecomposition3 
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
#include "UnitDecomposition3/FormulationMasterUnitDecomposition3.h"
#include "UnitDecomposition3/FormulationPricerUnitDecomposition3.h"
#include "UnitDecomposition3/ObjPricerUCPUnitDecomposition3.h"


//** Namespaces

using namespace std;
using namespace scip;


/** constructor */
ObjPricerUCPUnitDecomposition3::ObjPricerUCPUnitDecomposition3(
    SCIP* scip_master,       /**< SCIP pointer */
    const char* name,               /**< name of pricer */
    FormulationMasterUnitDecomposition3* formulation_master,
    InstanceUCP* instance_ucp
):
    ObjPricer(scip_master, name, "Pricer", 0, TRUE),
    m_formulation_master( formulation_master ),
    m_instance_ucp( instance_ucp )
{
    // initialization
    m_time = clock();
    m_nb_iterations = 0;
    m_total_solving_time_master = 0;
    m_total_solving_time_pricer = 0;

    int number_of_units( m_instance_ucp->get_units_number() );
    int number_of_time_steps( m_instance_ucp->get_time_steps_number() );

    // creation of the pricer problems 
    for(int i_unit = 0; i_unit < number_of_units ; i_unit ++ )
    {
        SCIP* scip_pricer(0);
        SCIPcreate( &scip_pricer );
        SCIPincludeDefaultPlugins( scip_pricer );
        SCIPcreateProb(scip_pricer, "UCP_PRICER_PROBLEM", 0, 0, 0, 0, 0, 0, 0);
        SCIPsetIntParam(scip_pricer, "display/verblevel", 0);
        FormulationPricerUnitDecomposition3* formulation_pricer = new FormulationPricerUnitDecomposition3( m_instance_ucp, 
            scip_pricer, i_unit
        );
        m_pricer_problems.push_back( formulation_pricer );
    }


    // initializing the reduced costs lists
    m_reduced_costs_pmax.resize( number_of_units );
    m_reduced_costs_pmin.resize( number_of_units );
    for(int i_unit = 0; i_unit < number_of_units ; i_unit ++ )
    {
        m_reduced_costs_pmax[i_unit].resize( number_of_time_steps, 0. );
        m_reduced_costs_pmin[i_unit].resize( number_of_time_steps, 0. );
    }
    m_reduced_costs_convexity.resize( number_of_units, 0. );

}


/** destructor */
ObjPricerUCPUnitDecomposition3::~ObjPricerUCPUnitDecomposition3()
{
    
}


/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerUCPUnitDecomposition3::scip_init)
{
    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());

    // pmin pmax constraints
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++ ) 
        {
            SCIP_CALL( SCIPgetTransformedCons( scip, 
                    *m_formulation_master->get_constraint_pmax( i_unit, i_time_step),
                    m_formulation_master->get_constraint_pmax( i_unit, i_time_step) ) );
            SCIP_CALL( SCIPgetTransformedCons( scip, 
                    *m_formulation_master->get_constraint_pmin( i_unit, i_time_step),
                    m_formulation_master->get_constraint_pmin( i_unit, i_time_step) ) );
        } 
    }

    // convexicity constraints
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {
        SCIP_CALL( SCIPgetTransformedCons(scip, 
            *m_formulation_master->get_convexity_constraint( i_unit ), 
            m_formulation_master->get_convexity_constraint( i_unit ) ) );
    }

    return SCIP_OKAY;
}


/** Pricing of additional variables if LP is feasible.
 *
 *  - get the values of the dual variables you need
 *  - construct the reduced-cost arc lengths from these values
 *  - find the shortest admissible tour with respect to these lengths
 *  - if this tour has negative reduced cost, add it to the LP
 *
 *  possible return values for *result:
 *  - SCIP_SUCCESS    : at least one improving variable was found, or it is ensured that no such variable exists
 *  - SCIP_DIDNOTRUN  : the pricing process was aborted by the pricer, there is no guarantee that the current LP solution is optimal
 */
SCIP_DECL_PRICERREDCOST(ObjPricerUCPUnitDecomposition3::scip_redcost)
{
    SCIPdebugMsg(scip, "call scip_redcost ...\n");

    /* set result pointer, see above */
    *result = SCIP_SUCCESS;

    /* call pricing routine */
    ucp_pricing(scip);

    return SCIP_OKAY;
} 
 


void ObjPricerUCPUnitDecomposition3::ucp_pricing(SCIP* scip)
{

    // stats about the master problem solving
    m_nb_iterations += 1;

    SCIP_Real optimal_value_master(SCIPgetPrimalbound( scip ) );
    m_primal_bounds.push_back( optimal_value_master );
    double solving_time_master = float( clock () - m_time ) /  CLOCKS_PER_SEC ;
    m_solving_times_master.push_back( solving_time_master );
    m_total_solving_time_master += solving_time_master;
    // cerr <<  optimal_value_master  << endl;

    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    int number_of_units( m_instance_ucp->get_units_number());


    //* get the reduced costs
    // pmin and pmax constraints
    SCIP_CONS* current_constraint(0);
    for(int i_unit = 0; i_unit < number_of_units ; i_unit ++ )
    {
        for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++ ) 
        {
            current_constraint =  *m_formulation_master->get_constraint_pmax(i_unit, i_time_step) ;
            m_reduced_costs_pmax[i_unit][i_time_step] = SCIPgetDualsolLinear( scip, current_constraint );
            current_constraint =  *m_formulation_master->get_constraint_pmin(i_unit, i_time_step) ;
            m_reduced_costs_pmin[i_unit][i_time_step] = SCIPgetDualsolLinear( scip, current_constraint );
        }
    }

    // convexity constraints
    for( int i_unit = 0; i_unit < number_of_units; i_unit++ )
    {
        current_constraint = *(m_formulation_master->get_convexity_constraint( i_unit ));
        m_reduced_costs_convexity[i_unit] = SCIPgetDualsolLinear( scip, current_constraint ) ;
    }



    //*  create and solve the pricing problems with the reduced values
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {    

        FormulationPricerUnitDecomposition3* current_pricer = m_pricer_problems[i_unit];
        current_pricer->change_reduced_costs(
            m_reduced_costs_pmax[i_unit],
            m_reduced_costs_pmin[i_unit]
        );

        SCIP* scip_pricer = current_pricer->get_scip_pointer();
        SCIPsolve( scip_pricer );

        SCIP_Real optimal_value_pricer(SCIPgetPrimalbound( scip_pricer ) );
        SCIP_Real solving_time_pricer(SCIPgetSolvingTime( scip_pricer ) );
        m_total_solving_time_pricer += solving_time_pricer;

        //* if a plan is found, create and add the column, else, do nothing, which will make the column generation stop
        if( optimal_value_pricer < m_reduced_costs_convexity[i_unit] -0.0001 )
        {
            //* create the plan
            ProductionPlan* new_plan = current_pricer->get_production_plan_from_solution();
            m_formulation_master->add_column( new_plan, false, i_unit );
        }

        SCIPfreeTransform( scip_pricer ) ;
    }

    m_time = clock();
    
}




// double ObjPricerUCPUnitDecomposition3::compute_lagrangian_bound()
// {
//     double lagrangian_bound = 0.;
    
//     for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++)
//     {
//         lagrangian_bound += dual_prices
//     }
// }
 



double ObjPricerUCPUnitDecomposition3::get_solving_time_master()
{
    return( m_total_solving_time_master );
}

double ObjPricerUCPUnitDecomposition3::get_solving_time_pricer()
{
    return( m_total_solving_time_pricer );
}


