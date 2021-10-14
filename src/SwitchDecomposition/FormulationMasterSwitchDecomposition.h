
/** 
 * @class FormulationMasterSwitchDecomposition
 * 
 * * This class makes the formulation for the Master Problem
 * 
 * It also allows to add columns as the column generation goes on
*/

#ifndef FormulationMasterSwitchDecomposition_H
#define FormulationMasterSwitchDecomposition_H


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

// SwitchDecomposition
#include "SwitchDecomposition/FormulationMasterSwitchDecomposition.h"



class FormulationMasterSwitchDecomposition : public FormulationMaster
{

    public:
    
        //* class functions
        
        /**
         *  constructor 
        */
        FormulationMasterSwitchDecomposition( 
            InstanceUCP* instance, 
            SCIP* scip_master, 
            bool use_inequality,
            double coefficient_repartition
        );

        /**
         *  destructor 
        */
        ~FormulationMasterSwitchDecomposition();



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

        SCIP_CONS** get_constraint_convexity( int time_step_number ); // exemple

        SCIP_CONS** get_constraint_min_uptime( int unit_number, int time_step_number );

        SCIP_CONS** get_constraint_min_downtime( int unit_number, int time_step_number );

        SCIP_CONS** get_constraint_variable_splitting( int unit_number, int time_step_number );

        SCIP_CONS** get_constraint_switch( int unit_number, int time_step_number );

        int get_columns_number();



    private:
        
        /**
         * weither to use inequality on the variable splitting constraint.
         * should be false by default
        */
        bool m_use_inequality;

        /** 
         * coefficient for the repartition of the costs between the splitted variables.
         * 1 means all the costs goes into x_current (base case);
         * 0 means all the costs goes into x_previous.
        */
        double m_coefficient_repartition;

        //* variables
        std::vector< VariableMaster* > m_vector_columns;
        std::vector< std::vector< SCIP_Var* > > m_variables_u;

        // constraints
        std::vector< SCIP_Cons* > m_constraints_convexity; 
        std::vector< std::vector< SCIP_Cons* > > m_constraints_min_uptime;
        std::vector< std::vector< SCIP_Cons* > > m_constraints_min_downtime;
        std::vector< std::vector< SCIP_Cons* > > m_constraints_variable_splitting;        
        std::vector< std::vector< SCIP_Cons* > > m_constraints_switch;

};



#endif

