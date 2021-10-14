/** 
 * @class FormulationPricerUnitDecomposition3
*/

#ifndef FormulationPricerUnitDecomposition3_H
#define FormulationPricerUnitDecomposition3_H

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

// Unit Decomposition



/* namespace */

class FormulationPricerUnitDecomposition3 : public FormulationPricer
{

    public : 

        //* constructor
        FormulationPricerUnitDecomposition3(InstanceUCP *instance, 
            SCIP *scip,
            int unit_number
        );

        //* destructor
        ~FormulationPricerUnitDecomposition3();

        ProductionPlan* get_production_plan_from_solution();
        
        SCIP_RETCODE change_reduced_costs( 
            std::vector< SCIP_Real > reduced_costs_pmax,
            std::vector< SCIP_Real > reduced_costs_pmin
        );


        //* virtual functions
        SCIP_RETCODE create_variables();

        SCIP_RETCODE create_constraints();


        //* gets
        
        int get_unit_number();



    private:

        //* variables
        std::vector< SCIP_VAR* > m_variable_u;
        std::vector< SCIP_VAR* > m_variable_x;

        //* contraintes
        std::vector< SCIP_CONS* > m_constraint_min_up_time;
        std::vector< SCIP_CONS* > m_constraint_min_down_time;
        std::vector< SCIP_CONS* > m_constraint_startup;
        
        //* block information
        int m_unit_number;

};

#endif