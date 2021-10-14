/** 
 * @class FormulationLinearRelaxation
*/

#ifndef FormulationLinearRelaxation_H
#define FormulationLinearRelaxation_H


//** Includes

//* Standart
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>


//* SCIP
#include <scip/scip.h>


//* User

// general
#include "DataClasses/InstanceUCP.h"
#include "DataClasses/ProductionPlan.h"


class FormulationLinearRelaxation
{


    public : 


        /* constructor */
        FormulationLinearRelaxation(InstanceUCP *instance, SCIP *scip);
        
        /* create the variables, add them to scip and store them */
        SCIP_RETCODE create_variables();

        /* create the constraints, add them to scip problem and store them */
        SCIP_RETCODE create_constraints();

        ProductionPlan* get_production_plan_from_solution();


    private:

        SCIP *m_scip;
        InstanceUCP *m_instance_ucp;

        /* variables */
        std::vector< std::vector< SCIP_VAR* >> m_variable_u;
        std::vector< std::vector< SCIP_VAR* >> m_variable_x;
        std::vector< std::vector< SCIP_VAR* >> m_variable_p;

        /* constraints */
        std::vector< SCIP_CONS* > m_constraint_demand;
        std::vector< std::vector< SCIP_CONS* >> m_constraint_production;
        std::vector< std::vector< SCIP_CONS* >> m_constraint_min_up_time;
        std::vector< std::vector< SCIP_CONS* >> m_constraint_min_down_time;
        std::vector< std::vector< SCIP_CONS* >> m_constraint_startup;
     

};

#endif