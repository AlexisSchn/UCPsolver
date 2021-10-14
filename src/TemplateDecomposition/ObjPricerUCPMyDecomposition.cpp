/** 
 * @file
 * Implement the class ObjPricerUCPMyDecomposition 
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
#include "MyDecomposition/FormulationMasterMyDecomposition.h"
#include "MyDecomposition/FormulationPricerMyDecomposition.h"
#include "MyDecomposition/ObjPricerUCPMyDecomposition.h"


//** Namespaces

using namespace std;
using namespace scip;


/** constructor */
ObjPricerUCPMyDecomposition::ObjPricerUCPMyDecomposition(
    SCIP* scip_master,       /**< SCIP pointer */
    const char* name,               /**< name of pricer */
    FormulationMasterMyDecomposition* formulation_master,
    InstanceUCP* instance_ucp
):
    ObjPricer(scip_master, name, "Pricer", 0, TRUE)
{
    m_formulation_master = formulation_master;
    m_instance_ucp = instance_ucp;
    m_total_solving_time_master = 0;
    m_total_solving_time_pricer = 0;
    m_nb_iterations = 0;
    m_time = clock();

    //* initialize the pricer problem
    for(int i_block = 0; i_block < number_of_blocks; i_block ++)
    {   
        SCIP* scip_pricer(0);
        SCIPcreate( &scip_pricer ) ;
        SCIPincludeDefaultPlugins( scip_pricer );
        SCIPcreateProb(scip_pricer, "UCP_PRICER_PROBLEM", 0, 0, 0, 0, 0, 0, 0);
        SCIPsetIntParam(scip_pricer, "display/verblevel", 0);
        FormulationPricerMyDecomposition* formulation_pricer = new FormulationPricerMyDecomposition( m_instance_ucp, 
            scip_pricer,
            i_block
        );
        m_pricer_problems.push_back( formulation_pricer );
    }

    //* initialize the reduced costs vectors
    // convexity constraints
    m_reduced_costs_convexity.resize( number_of_time_blocks, 0. );
    // rest of the constraints


}


/** destructor */
ObjPricerUCPMyDecomposition::~ObjPricerUCPMyDecomposition()
{}


/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerUCPMyDecomposition::scip_init)
{
    int number_of_units( m_instance_ucp->get_units_number());
    int number_of_time_steps( m_instance_ucp->get_time_steps_number());

    //! TO FILL
    // for each constraint where we need the reduced costs : 
    // use 
    // SCIP_CALL( SCIPgetTransformedCons( scip, 
    //                 *m_formulation_master->get_constraint( ),
    //                 m_formulation_master->get_constraint( ) ) )

    return SCIP_OKAY;
}



SCIP_DECL_PRICERREDCOST(ObjPricerUCPMyDecomposition::scip_redcost)
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



SCIP_RETCODE ObjPricerUCPMyDecomposition::ucp_pricing(SCIP* scip)
{

    int number_of_time_steps( m_instance_ucp->get_time_steps_number());
    int number_of_units( m_instance_ucp->get_units_number());

    m_total_solving_time_master += float( clock () - m_time ) /  CLOCKS_PER_SEC ;

    //* get the reduced costs
    // for each constraints 
    //! to fill

    //with
    // vector< SCIP_Real > reduced_costs
    //  SCIP_CONS* current_constraint(0);
    // current_constraint = *m_formulation_master-> get_constraint();
    // SCIPgetDualsolLinear( scip, current_constraint );


    //*  create and solve the pricing problems with the reduced values
    for(int i_unit = 0; i_unit < number_of_units; i_unit ++)
    {   
        FormulationPricerMyDecomposition* current_pricer = m_pricer_problems[i_time_step];
        current_pricer->change_reduced_costs(
            m_reduced_costs //! to fill with reduced costs
        );
        SCIP* scip_pricer = current_pricer->get_scip_pointer();
        m_time = clock();
        SCIPsolve( scip_pricer );
        m_total_solving_time_pricer += float( clock() - m_time ) /  CLOCKS_PER_SEC ;
        
        // SCIPprintBestSol(scip_pricer, NULL, FALSE) ;

        //* if a plan is found, create and add the column, else, do nothing, which will make the column generation stop
        SCIP_Real optimal_value(SCIPgetPrimalbound( scip_pricer ) );
        if( optimal_value < //! to fill with reduced cost we need -0.0001 )
        {
            //* create the plan
            ProductionPlan* new_plan = formulation_pricer.get_production_plan_from_solution();
            m_formulation_master->add_column( new_plan, false, i_unit );

        }
    
    }

    return( SCIP_OKAY );    
}







