/** 
 * @class FormulationPricerUnitDecomposition2
*/

#ifndef FormulationPricerUnitDecomposition2_H
#define FormulationPricerUnitDecomposition2_H

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

class FormulationPricerUnitDecomposition2 : public FormulationPricer
{

    public : 

        //* constructor
        FormulationPricerUnitDecomposition2(InstanceUCP *instance, SCIP *scip, 
        std::vector<SCIP_Real> reduced_costs_demand,
        int unit_number );

        //* destructor
        ~FormulationPricerUnitDecomposition2()
        {};

        ProductionPlan* get_production_plan_from_solution();
        
        //* virtual functions
        SCIP_RETCODE create_variables();
        SCIP_RETCODE create_constraints();


        //* gets
        
        int get_unit_number();



    private:

        //* variables
        std::vector< SCIP_VAR* > m_variable_u;
        std::vector< SCIP_VAR* > m_variable_x;
        std::vector< SCIP_VAR* > m_variable_p;

        //* contraintes
        std::vector< SCIP_CONS* > m_constraint_production;
        std::vector< SCIP_CONS* > m_constraint_min_up_time;
        std::vector< SCIP_CONS* > m_constraint_min_down_time;
        std::vector< SCIP_CONS* > m_constraint_startup;
        
        //* reduced costs
        std::vector<SCIP_Real> m_reduced_costs_demand;

        //* block information
        int m_unit_number;

};

#endif