
/** 
 * @class FormulationMasterTimeDecomposition
 * 
 * * This class makes the formulation for the Master Problem
 * 
 * It also allows to add columns as the column generation goes on
*/

#ifndef FormulationMasterTimeDecomposition_H
#define FormulationMasterTimeDecomposition_H


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

// TimeDecomposition
#include "TimeDecomposition/FormulationMasterTimeDecomposition.h"



class FormulationMasterTimeDecomposition : public FormulationMaster
{

    public:
    
        //* class functions

        /**
         *  constructor 
        */
        FormulationMasterTimeDecomposition( InstanceUCP* instance, SCIP* scip_master);

        /**
         *  destructor 
        */
        ~FormulationMasterTimeDecomposition();



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


        //* usefull functions
        /**
         * create and add a column to the master problem and to it's constraints
        */
        SCIP_RETCODE add_column( ProductionPlan* plan_of_new_column, bool initialization, int block_number );

        /**
         * create the production plan corresponding to the last solving of the master problem
        */
        ProductionPlan* get_production_plan_from_solution();


        //* gets

        SCIP_CONS** get_constraint_convexity( int number_block );
        SCIP_Cons** get_constraint_min_uptime( int number_unit, int number_time_step );
        SCIP_Cons** get_constraint_min_downtime( int number_unit, int number_time_step );
        SCIP_Cons** get_constraint_switch( int number_unit, int number_time_step );

        int get_columns_number();

    private:
         
        
        
        //* variables
        std::vector< VariableMaster* > m_vector_columns;
        std::vector< std::vector < SCIP_VAR* > > m_variables_u;

        //* constraints
        std::vector< SCIP_Cons* > m_constraints_convexity; 
        std::vector< std::vector< SCIP_Cons* > > m_constraints_min_uptime;
        std::vector< std::vector< SCIP_Cons* > > m_constraints_min_downtime;
        std::vector< std::vector< SCIP_Cons* > > m_constraints_switch;
        


};



#endif

