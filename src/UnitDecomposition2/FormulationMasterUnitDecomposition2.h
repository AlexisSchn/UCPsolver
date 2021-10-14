/** 
 * @class FormulationMasterUnitDecomposition2
*/

#ifndef FormulationMasterUnitDecomposition2_H
#define FormulationMasterUnitDecomposition2_H


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
#include "UnitDecomposition2/FormulationMasterUnitDecomposition2.h"



class FormulationMasterUnitDecomposition2 : public FormulationMaster
{

    public:
    
        /**
         *  constructor 
        */
        FormulationMasterUnitDecomposition2( InstanceUCP* instance, SCIP* scip_master);

        /**
         *  destructor 
        */
        ~FormulationMasterUnitDecomposition2();



        /**
         * @brief adds a column to the restricted master problem 
        */
        SCIP_RETCODE add_column( ProductionPlan* plan_of_new_column, bool initialization, int block_number );
        
        SCIP_RETCODE create_variables();

        SCIP_RETCODE create_constraints();

        SCIP_RETCODE create_and_add_first_columns();

        ProductionPlan* get_production_plan_from_solution();



        //* gets

        SCIP_CONS** get_convexity_constraint(int unit_number);
        SCIP_CONS** get_complicating_constraints(int number_time_step);
        int get_columns_number();

    private:


        //* variables
        std::vector< VariableMaster* > m_vector_columns;

        // constraints
        std::vector< SCIP_CONS* > m_complicating_constraints;
        std::vector< SCIP_Cons* > m_convexity_constraint;


};



#endif