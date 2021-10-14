/** 
 * @class FormulationPricerTimeDecomposition2pmax
*/

#ifndef FormulationPricerTimeDecomposition2pmax_H
#define FormulationPricerTimeDecomposition2pmax_H

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
#include "Decomposition/FormulationPricer.h"

// TimeDecomposition2pmax



/* namespace */

class FormulationPricerTimeDecomposition2pmax : public FormulationPricer
{

    public : 


        //* class related function 

        /**
         * Constructor
         */
        FormulationPricerTimeDecomposition2pmax(InstanceUCP *instance, 
            SCIP *scip,
            int block_number,
            std::vector<std::vector<SCIP_Real>> reduced_costs_switch,
            std::vector<std::vector<SCIP_Real>> reduced_costs_min_uptime,
            std::vector<std::vector<SCIP_Real>> reduced_costs_min_downtime,
            std::vector<std::vector<SCIP_Real>> reduced_costs_pmaxpmin,
            std::vector<SCIP_Real> reduced_costs_demand
        );

        /** Destructor
         * 
         */
        ~FormulationPricerTimeDecomposition2pmax()
        {
            SCIPsetMessagehdlrQuiet( m_scip_pricer, 1);
            SCIPfree(&m_scip_pricer);
        };


        //* solving related functions

        /**
         * create the variable
         */
        SCIP_RETCODE create_variables();

        /**
         * create the constraints
         */
        SCIP_RETCODE create_constraints();

        /**
         * create the production plan corresponding to the solution
        */
        ProductionPlan* get_production_plan_from_solution();
        

        //* gets
        
        int get_unit_number();



    private:

        //* variables
        std::vector< SCIP_VAR* > m_variables_x;
        
        //* contraintes
        SCIP_Cons* m_constraints_demand;

        //* reduced costs
        std::vector<std::vector<SCIP_Real>> m_reduced_costs_min_uptime;
        std::vector<std::vector<SCIP_Real>> m_reduced_costs_min_downtime;
        std::vector<std::vector<SCIP_Real>> m_reduced_costs_switch;
        std::vector<std::vector<SCIP_Real>> m_reduced_costs_pmaxpmin;
        std::vector<SCIP_Real> m_reduced_costs_demand;

        //* block information
        int m_block_number;

};

#endif