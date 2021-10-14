
/** 
 * @class FormulationMasterOverlappingDecomposition
 * 
 * * This class makes the formulation for the Master Problem
 * 
 * It also allows to add columns as the column generation goes on
*/

#ifndef FormulationMasterOverlappingDecomposition_H
#define FormulationMasterOverlappingDecomposition_H


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

// OverlappingDecomposition
#include "OverlappingDecomposition/FormulationMasterOverlappingDecomposition.h"



class FormulationMasterOverlappingDecomposition : public FormulationMaster
{

    public:
    
        //* class functions
        
        /**
         *  constructor 
        */
        FormulationMasterOverlappingDecomposition( 
            InstanceUCP* instance, 
            SCIP* scip_master,
            bool use_inequality,
            double coefficient_repartition_fixed,
            double coefficient_repartition_prop
        );

        /**
         *  destructor 
        */
        ~FormulationMasterOverlappingDecomposition();



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
         * create and add a column for the units to the master problem and to its constraints
        */
        SCIP_RETCODE add_column_unit( ProductionPlan* plan_of_new_column, bool initialization, int block_number );

        /**
         * create and add a column for the times to the master problem and to its constraints
        */
        SCIP_RETCODE add_column_time( ProductionPlan* plan_of_new_column, bool initialization, int block_number );

        /**
         * create the production plan corresponding to the last solving of the master problem
        */
        ProductionPlan* get_production_plan_from_solution();


        //* gets

        SCIP_CONS** get_constraint_convexity_time( int time_step_number );
        SCIP_CONS** get_constraint_convexity_unit( int unit_number );
        SCIP_CONS** get_constraint_variable_splitting( int unit_number, int time_step_number );

        int get_columns_number_unit();
        int get_columns_number_time();


    private:
         
        bool m_use_inequality;

        double m_coefficient_repartition_prop;


        /**
         * Quantity of the fixed costs we put in the time pricer pb
         */
        double m_coefficient_repartition_fixed;
        
        //* variables
        std::vector< VariableMaster* > m_vector_columns_time;
        std::vector< VariableMaster* > m_vector_columns_unit;

        //* constraints
        std::vector< SCIP_Cons* > m_constraints_convexity_time; 
        std::vector< SCIP_Cons* > m_constraints_convexity_unit;
        std::vector< std::vector< SCIP_Cons* > > m_constraints_variable_splitting;

};



#endif

