/** 
 * @class FormulationPricerOverlappingDecompositionTime
*/

#ifndef FormulationPricerOverlappingDecompositionTime_H
#define FormulationPricerOverlappingDecompositionTime_H

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

// OverlappingDecompositionTime



/* namespace */

class FormulationPricerOverlappingDecompositionTime : public FormulationPricer
{

    public : 


        //* class related function 

        /**
         * Constructor
         */
        FormulationPricerOverlappingDecompositionTime(InstanceUCP *instance, 
            SCIP *scip,
            int block_number,
            double coefficient_repartition_fixed,
            double coefficient_repartition_prop
        );

        /** Destructor
         * 
         */
        ~FormulationPricerOverlappingDecompositionTime()
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
            std::vector<std::vector<SCIP_Real>> reduced_costs_variable_splitting
        );

        /**
         * create the production plan corresponding to the solution
        */
        ProductionPlan* get_production_plan_from_solution();
        

        //* gets
        
        int get_block_number();



    private:

        double m_coefficient_repartition_fixed;

        double m_coefficient_repartition_prop;


        //* variables
        std::vector< SCIP_VAR* > m_variables_x;
        std::vector< SCIP_VAR* > m_variables_p;
        
        //* block information
        int m_block_number;

};

#endif