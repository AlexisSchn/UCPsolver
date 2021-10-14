/** 
 * @class FormulationPricerSwitchDecomposition2
*/

#ifndef FormulationPricerSwitchDecomposition2_H
#define FormulationPricerSwitchDecomposition2_H

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

// SwitchDecomposition2



/* namespace */

class FormulationPricerSwitchDecomposition2 : public FormulationPricer
{

    public : 


        //* class related function 

        /**
         * Constructor
         */
        FormulationPricerSwitchDecomposition2(InstanceUCP *instance, 
            SCIP *scip, 
            int block_number,
            double coefficient_repartition
        );

        /** Destructor
         * 
         */
        ~FormulationPricerSwitchDecomposition2()
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
         * modify the costs of the problem with regards to reduced costs
         */
        SCIP_RETCODE change_reduced_costs( 
            std::vector<std::vector<SCIP_Real>> reduced_costs_min_uptime,
            std::vector<std::vector<SCIP_Real>> reduced_costs_min_downtime,
            std::vector<std::vector<SCIP_Real>> reduced_costs_variable_splitting
        );

        /**
         * create the production plan corresponding to the solution
        */
        ProductionPlan* get_production_plan_from_solution();
        

        //* gets
        
        int get_block_number();



    private:

        double m_coefficient_repartition;

        //* variables
        std::vector< SCIP_Var* > m_variables_x_current;
        std::vector< SCIP_Var* > m_variables_x_previous;

        std::vector< SCIP_Var* > m_variables_p_current;
        std::vector< SCIP_Var* > m_variables_p_previous;

        std::vector< SCIP_Var* > m_variables_u;
        
        //* block information
        int m_block_number;

};

#endif


