/** 
 * @class FormulationMasterUnitDecomposition3
*/

#ifndef FormulationMasterUnitDecomposition3_H
#define FormulationMasterUnitDecomposition3_H


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
#include "Decomposition/FormulationMaster.h"
#include "Decomposition/VariableMaster.h"

// Unit Decomposition
#include "UnitDecomposition3/FormulationMasterUnitDecomposition3.h"



class FormulationMasterUnitDecomposition3 : public FormulationMaster
{

    public:
    
        /**
         *  constructor 
        */
        FormulationMasterUnitDecomposition3( InstanceUCP* instance, SCIP* scip_master);

        /**
         *  destructor 
        */
        ~FormulationMasterUnitDecomposition3();


        SCIP_RETCODE create_variables();

        SCIP_RETCODE create_constraints();

        SCIP_RETCODE create_and_add_first_columns();

        /**
         * @brief adds a column to the restricted master problem 
        */
        SCIP_RETCODE add_column( ProductionPlan* plan_of_new_column, bool initialization, int block_number );


        ProductionPlan* get_production_plan_from_solution();


        //* gets

        SCIP_CONS** get_convexity_constraint(int unit_number);
        SCIP_CONS** get_constraint_pmax(int number_unit, int number_time_step);
        SCIP_CONS** get_constraint_pmin(int number_unit, int number_time_step);

        int get_columns_number();

    private:
         
        
        
        //* variables
        std::vector< VariableMaster* > m_vector_columns;
        std::vector< std::vector < SCIP_VAR* > > m_variable_p;

        // constraints
        std::vector< SCIP_CONS* > m_constraints_demand;

        std::vector< std::vector < SCIP_CONS* > > m_constraints_pmin;
        std::vector< std::vector < SCIP_CONS* > > m_constraints_pmax;

        std::vector< SCIP_Cons* > m_constraints_convexity;


};



#endif