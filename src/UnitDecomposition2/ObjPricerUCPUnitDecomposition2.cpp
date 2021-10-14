/** 
 * @file
 * Implement the class ObjPricerUCPUnitDecomposition2 
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
#include "UnitDecomposition2/FormulationMasterUnitDecomposition2.h"
#include "UnitDecomposition2/FormulationPricerUnitDecomposition2.h"
#include "UnitDecomposition2/ObjPricerUCPUnitDecomposition2.h"


//** Namespaces

using namespace std;
using namespace scip;


/** constructor */
ObjPricerUCPUnitDecomposition2::ObjPricerUCPUnitDecomposition2(
    SCIP* scip_master,       /**< SCIP pointer */
    const char* name,               /**< name of pricer */
    FormulationMasterUnitDecomposition2* formulation_master,
    InstanceUCP* instance_ucp
):
    ObjPricer(scip_master, name, "Pricer", 0, TRUE)
{
    m_formulation_master = formulation_master;
    m_instance_ucp = instance_ucp;
}


/** destructor */
ObjPricerUCPUnitDecomposition2::~ObjPricerUCPUnitDecomposition2()
{}


/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerUCPUnitDecomposition2::scip_init)
{
    
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++ ) 
    {
        SCIP_CALL( SCIPgetTransformedCons( scip, 
                *m_formulation_master->get_complicating_constraints(i_time_step),
                m_formulation_master->get_complicating_constraints(i_time_step) ) );
    } 

    int number_of_units( m_instance_ucp->get_units_number());
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
SCIP_DECL_PRICERREDCOST(ObjPricerUCPUnitDecomposition2::scip_redcost)
{
    SCIPdebugMsg(scip, "call scip_redcost ...\n");

    /* set result pointer, see above */
    *result = SCIP_SUCCESS;

    /* call pricing routine */
    ucp_pricing(scip);

    return SCIP_OKAY;
} 



void ObjPricerUCPUnitDecomposition2::ucp_pricing(SCIP* scip)
{

    m_list_RMP_opt.push_back( SCIPgetPrimalbound( scip ) );
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    int number_of_units( m_instance_ucp->get_units_number());

    //* get the reduced costs
    // demand constraints
    vector< SCIP_Real > reduced_cost_demand;
    reduced_cost_demand.resize( number_of_time_steps );
    SCIP_CONS* current_constraint(0);

    for(int i_time_step = 0; i_time_step < number_of_time_steps; i_time_step ++ ) 
    {
        current_constraint =  *m_formulation_master->get_complicating_constraints(i_time_step) ;
        reduced_cost_demand[i_time_step] = SCIPgetDualsolLinear( scip, current_constraint );
    }

    // convexity constraints
    vector< SCIP_Real > reduced_costs_convexity;
    reduced_costs_convexity.resize( number_of_units );
    for( int i_unit = 0; i_unit < number_of_units; i_unit++ )
    {
        current_constraint = *(m_formulation_master->get_convexity_constraint( i_unit ));
        reduced_costs_convexity[i_unit] = SCIPgetDualsolLinear( scip, current_constraint ) ;

    }

    //*  create and solve the pricing problems with the reduced values
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {   
        SCIP* scip_pricer(0);
        SCIPcreate( &scip_pricer );
        SCIPincludeDefaultPlugins( scip_pricer );
        SCIPcreateProb(scip_pricer, "UCP_PRICER_PROBLEM", 0, 0, 0, 0, 0, 0, 0);
        SCIPsetIntParam(scip_pricer, "display/verblevel", 0);
        FormulationPricerUnitDecomposition2 formulation_pricer( m_instance_ucp, 
            scip_pricer, reduced_cost_demand, i_unit);
        SCIPsolve( scip_pricer );
        // SCIPprintBestSol(scip_pricer, NULL, FALSE) ;

        //* if a plan is found, create and add the column, else, do nothing, which will make the column generation stop
        SCIP_Real optimal_value(SCIPgetPrimalbound( scip_pricer ) );
        if( optimal_value < reduced_costs_convexity[i_unit] -0.0001 )
        {
            //* create the plan and send it to the master problem to create a new column
            ProductionPlan* new_plan = formulation_pricer.get_production_plan_from_solution();
            m_formulation_master->add_column( new_plan, false, i_unit );

        }
    
    }
    
}









