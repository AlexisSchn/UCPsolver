
/** 
 * @class ObjPricerUCPMyDecomposition.h
 ** This class is needed by SCIP to make a column generation.
 * an object of the class will have to be created before the 
*/

#ifndef ObjPricerUCPTimeDecomposition_H
#define ObjPricerUCPTimeDecomposition_H

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

// Unit Decomposition 2
#include "TimeDecomposition/FormulationMasterTimeDecomposition.h"
#include "TimeDecomposition/FormulationPricerTimeDecomposition.h"

//** Namespace 
using namespace scip;

class ObjPricerUCPTimeDecomposition : public ObjPricer
{

    public:

        //* class functions

        /**
         * constructor
         */
        ObjPricerUCPTimeDecomposition(
            SCIP* scip_master,                  /**< SCIP pointer */
            const char* name,                   /**< name of pricer */
            FormulationMasterTimeDecomposition* formulation_master,
            InstanceUCP* instance_ucp
        );

        /**
         * destructor
         */
        virtual ~ObjPricerUCPTimeDecomposition();



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

        FormulationMasterTimeDecomposition* m_formulation_master;
        InstanceUCP* m_instance_ucp;

        std::vector< FormulationPricerTimeDecomposition* > m_pricer_problems;

        // std::vector< double > m_master_values;
        int m_nb_iterations;

        // times
        double m_total_solving_time_master;
        double m_total_solving_time_pricer;
        double m_time_1;
        clock_t m_time;

        // reduced costs
        std::vector<SCIP_Real> m_reduced_costs_convexity;
        std::vector<std::vector<SCIP_Real>> m_reduced_costs_switch;
        std::vector<std::vector<SCIP_Real>> m_reduced_costs_min_uptime;
        std::vector<std::vector<SCIP_Real>> m_reduced_costs_min_downtime;

    
};


#endif