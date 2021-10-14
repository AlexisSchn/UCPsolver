
/** 
 * @class ObjPricerUCPOverlappingDecomposition.h
 ** This class is needed by SCIP to make a column generation.
 * an object of the class will have to be created before the 
*/

#ifndef ObjPricerUCPOverlappingDecomposition_H
#define ObjPricerUCPUOverlappingDecomposition_H

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

// Unit Decomposition 2
#include "OverlappingDecomposition/FormulationMasterOverlappingDecomposition.h"
#include "OverlappingDecomposition/FormulationPricerOverlappingDecompositionUnit.h"
#include "OverlappingDecomposition/FormulationPricerOverlappingDecompositionTime.h"

//** Namespace 
using namespace scip;

class ObjPricerUCPOverlappingDecomposition : public ObjPricer
{

    public:

        //* class functions

        /**
         * constructor
         */
        ObjPricerUCPOverlappingDecomposition(
            SCIP* scip_master,                  /**< SCIP pointer */
            const char* name,                   /**< name of pricer */
            FormulationMasterOverlappingDecomposition* formulation_master,
            InstanceUCP* instance_ucp,
            double coefficient_repartition_fixed,
            double coefficient_repartition_prop
        );

        /**
         * destructor
         */
        virtual ~ObjPricerUCPOverlappingDecomposition();



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



    private:

        FormulationMasterOverlappingDecomposition* m_formulation_master;
        InstanceUCP* m_instance_ucp;
        
        //* parameters
        double m_coefficient_repartition_fixed;
        double m_coefficient_repartition_prop;

        //* stats
        int m_nb_iterations;
        // std::vector<SCIP_Real> m_primal_bounds;
        // std::vector< double > m_lagrangian_bounds;
        double m_total_solving_time_master;
        double m_total_solving_time_pricer;
        clock_t m_time;


        //* reduced costs 
        std::vector<SCIP_Real> m_reduced_costs_convexity_unit;
        std::vector<SCIP_Real> m_reduced_costs_convexity_time;
        std::vector< std::vector< SCIP_Real > > m_reduced_costs_variable_splitting;

        //* pricer problems
        std::vector< FormulationPricerOverlappingDecompositionUnit* > m_pricer_unit_problems;
        std::vector< FormulationPricerOverlappingDecompositionTime* > m_pricer_time_problems;

};


#endif