/** 
 * @class Pricer.h
 ** Allows SCIP to make a column generation. Tell what is the pricing problem
*/

#ifndef ObjPricerUCPUnitDecomposition3_H
#define ObjPricerUCPUnitDecomposition3_H

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
#include "UnitDecomposition3/FormulationMasterUnitDecomposition3.h"
#include "UnitDecomposition3/FormulationPricerUnitDecomposition3.h"

//** Namespace 
using namespace scip;

class ObjPricerUCPUnitDecomposition3 : public ObjPricer
{

    public:

        /** Constructs the pricer object with the data needed */
        ObjPricerUCPUnitDecomposition3(
            SCIP* scip_master,                  /**< SCIP pointer */
            const char* name,                   /**< name of pricer */
            FormulationMasterUnitDecomposition3* formulation_master,
            InstanceUCP* instance_ucp
        );

        /** Destructs the pricer object. */
        virtual ~ObjPricerUCPUnitDecomposition3();

        /** reduced cost pricing method of variable pricer for feasible LPs */
        virtual SCIP_DECL_PRICERREDCOST(scip_redcost);

        /** initialization method of variable pricer (called after problem was transformed) */
        virtual SCIP_DECL_PRICERINIT(scip_init);


        /** perform pricing */
        void ucp_pricing(SCIP* scip);

        
        // /**
        //  * compute the lagrangian bounds
        // */
        // double compute_lagrangian_bound();

        /**
         * get the total solving time of the master problems
         */
        double get_solving_time_master();

        /**
         * get the total solving time of the pricing problems
         */
        double get_solving_time_pricer();

        int get_nb_iterations()
        {
            return(m_nb_iterations);
        }


    private:

        FormulationMasterUnitDecomposition3* m_formulation_master;
        InstanceUCP* m_instance_ucp;

        // pricer problems
        std::vector< FormulationPricerUnitDecomposition3* > m_pricer_problems;


        // reduced costs
        std::vector< std::vector < SCIP_Real > > m_reduced_costs_pmax;
        std::vector< std::vector < SCIP_Real > > m_reduced_costs_pmin;
        std::vector < SCIP_Real > m_reduced_costs_convexity;

        // bounds
        std::vector< double > m_primal_bounds;
        std::vector< double > m_lagrangian_bounds;

        // countings
        int m_nb_iterations;

        // times
        std::vector< double > m_solving_times_master;
        double m_total_solving_time_master;
        double m_total_solving_time_pricer;

        clock_t m_time;
};


#endif