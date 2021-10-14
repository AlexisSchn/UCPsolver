
/** 
 * @class ObjPricerUCPSwitchDecomposition.h
 ** This class is needed by SCIP to make a column generation.
 * an object of the class will have to be created before the 
*/

#ifndef ObjPricerUCPSwitchDecomposition_H
#define ObjPricerUCPSwitchDecomposition_H

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

// SwitchDecomposition
#include "SwitchDecomposition/FormulationMasterSwitchDecomposition.h"
#include "SwitchDecomposition/FormulationPricerSwitchDecomposition.h"

//** Namespace 
using namespace scip;

class ObjPricerUCPSwitchDecomposition : public ObjPricer
{

    public:

        //* class functions

        /**
         * constructor
         */
        ObjPricerUCPSwitchDecomposition(
            SCIP* scip_master,                  /**< SCIP pointer */
            const char* name,                   /**< name of pricer */
            FormulationMasterSwitchDecomposition* formulation_master,
            InstanceUCP* instance_ucp,
            double coefficient_repartition
        );

        /**
         * destructor
         */
        virtual ~ObjPricerUCPSwitchDecomposition();



        //* interface (?) functions recquired by scip

        /** 
         * SCIP calls this function to generate new variables
         * the pricing takes place here.
         *  possible return values for *result:
         *  - SCIP_SUCCESS    : at least one improving variable was found, or it is ensured that no such variable exists
         *  - SCIP_DIDNOTRUN  : the pricing process was aborted by the pricer, there is no guarantee that the current LP solution is optimal
         */
        virtual SCIP_DECL_PRICERREDCOST(scip_redcost);

        /** 
         * SCIP transforms the constraints of the problem during the presolving (?)
         * we need to get the right adresses  of the transformed constraints to get their reduced costs  
        */
        virtual SCIP_DECL_PRICERINIT(scip_init);
        
        
        //* user functions

        /**
         * performs the pricing, adding the variables recquired to the master problem.
         * if no variables are added, the column generation will stop.
         */
        SCIP_RETCODE ucp_pricing(SCIP* scip);

        
        int get_nb_iterations()
        {
            return(m_nb_iterations);
        }

        double get_solving_time_master()
        {
            return( m_total_solving_time_master );
        }

        double get_solving_time_pricer()
        {
            return( m_total_solving_time_pricer );
        }


    private:

        FormulationMasterSwitchDecomposition* m_formulation_master;
        InstanceUCP* m_instance_ucp;
        std::vector< FormulationPricerSwitchDecomposition* > m_pricer_problems;

        double m_coefficient_repartition;

        int m_nb_iterations;

        // times
        double m_total_solving_time_master;
        double m_total_solving_time_pricer;
        clock_t m_time;

        // reduced costs
        std::vector<SCIP_Real> m_reduced_costs_convexity;
        std::vector< std::vector< SCIP_Real > > m_reduced_costs_min_uptime;
        std::vector< std::vector< SCIP_Real > > m_reduced_costs_min_downtime;
        std::vector< std::vector< SCIP_Real > > m_reduced_costs_variable_splitting;
        std::vector< std::vector< SCIP_Real > > m_reduced_costs_switch;


};


#endif

