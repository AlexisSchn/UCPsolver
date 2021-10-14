/** 
 * @class FormulationPricerMyDecomposition
*/

#ifndef FormulationPricerMyDecomposition_H
#define FormulationPricerMyDecomposition_H

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

// MyDecomposition



/* namespace */

class FormulationPricerMyDecomposition : public FormulationPricer
{

    public : 


        //* class related function 

        /**
         * Constructor
         */
        FormulationPricerMyDecomposition(InstanceUCP *instance, 
            SCIP *scip, 
            int block_number
        );

        /** Destructor
         * 
         */
        ~FormulationPricerMyDecomposition()
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
            std::vector<std::vector<SCIP_Real>> reduced_costs_somthing
        );

        /**
         * create the production plan corresponding to the solution
        */
        ProductionPlan* get_production_plan_from_solution();
        

        //* gets
        
        int get_block_number();



    private:

        //* variables

        
        //* block information
        int m_block_number;

};

#endif