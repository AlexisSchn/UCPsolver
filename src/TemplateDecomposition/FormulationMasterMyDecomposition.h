
/** 
 * @class FormulationMasterMyDecomposition
 * 
 * * This class makes the formulation for the Master Problem
 * 
 * It also allows to add columns as the column generation goes on
*/

#ifndef FormulationMasterMyDecomposition_H
#define FormulationMasterMyDecomposition_H


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

// MyDecomposition
#include "MyDecomposition/FormulationMasterMyDecomposition.h"



class FormulationMasterMyDecomposition : public FormulationMaster
{

    public:
    
        //* class functions
        
        /**
         *  constructor 
        */
        FormulationMasterMyDecomposition( InstanceUCP* instance, SCIP* scip_master);

        /**
         *  destructor 
        */
        ~FormulationMasterMyDecomposition();



        //* Initialization of the problem

        /**
         *  create the non-columns variables of the master problem 
        */
        SCIP_RETCODE create_variables();

        /**
         *  create the constraints of the master problem 
        */
        SCIP_RETCODE create_constraints();

        /**
         *  initialize the problems with a few columns that allow the master problem to be solvable.
        */
        SCIP_RETCODE create_and_add_first_columns();


        //* Recquired functions
        /**
         * create and add a column to the master problem and to it's constraints
        */
        SCIP_RETCODE add_column( ProductionPlan* plan_of_new_column, bool initialization, int block_number );

        /**
         * create the production plan corresponding to the last solving of the master problem
        */
        ProductionPlan* get_production_plan_from_solution();


        //* gets

        SCIP_CONS** get_constraint_convexity(); // exemple

        int get_columns_number();

    private:
         
        
        
        //* variables
        std::vector< VariableMaster* > m_vector_columns;

        // constraints
        std::vector< SCIP_Cons* > m_constraints_convexity; // exemple


};



#endif

