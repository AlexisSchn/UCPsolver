/** 
 * @class FormulationPricerTimeDecomposition
*/

#ifndef FormulationPricerTimeDecomposition_H
#define FormulationPricerTimeDecomposition_H

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

// TimeDecomposition



/* namespace */

class FormulationPricerTimeDecomposition : public FormulationPricer
{

    public : 


        //* class related function 

        /**
         * Constructor
         */
        FormulationPricerTimeDecomposition(InstanceUCP *instance, 
            SCIP *scip,
            int block_number
        );

        /** Destructor
         * 
         */
        ~FormulationPricerTimeDecomposition()
        {
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
        
        /**
         * modify the costs of the problem with regards to reduced costs
         */
        SCIP_RETCODE change_reduced_costs( 
            std::vector<std::vector<SCIP_Real>> reduced_costs_switch,
            std::vector<std::vector<SCIP_Real>> reduced_costs_min_uptime,
            std::vector<std::vector<SCIP_Real>> reduced_costs_min_downtime
        );

        //* gets
        
        int get_unit_number();



    private:

        //* variables
        std::vector< SCIP_VAR* > m_variables_x;
        std::vector< SCIP_VAR* > m_variables_p;
        
        //* contraintes
        std::vector<SCIP_Cons*> m_constraints_production_max;
        std::vector<SCIP_Cons*> m_constraints_production_min;
        SCIP_Cons* m_constraints_demand;
        
        //* block information
        int m_block_number;

};

#endif