/** 
 * @class FormulationMaster
*/

#ifndef FormulationMaster_H
#define FormulationMaster_H



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



class FormulationMaster
{

    public:
    
        /**
         *  constructor 
         * @param instance
         * @param scip
         * 
        */
        FormulationMaster( InstanceUCP* instance, SCIP* scip_master);

        /**
         * destructor
        */
        virtual ~FormulationMaster()
        {}

        /**
         * @brief goes over the columns selected to get the current best plan 
         * @return a pointer to the production plan
         * @remark working for by itself, but may be modified in inherited classes
        */
        virtual ProductionPlan* get_production_plan_from_solution() = 0;
     
        /**
         * 
         * @brief adds a column to the restricted master problem . Pure virtual function 
         * for now we keep it aside, it is put in childrent classes
        */
        // virtual SCIP_RETCODE add_column( ProductionPlan* plan_of_new_column ) = 0;

        virtual SCIP_RETCODE create_variables() = 0;

        virtual SCIP_RETCODE create_constraints() = 0;

        virtual SCIP_RETCODE create_and_add_first_columns() = 0;



        //* gets

        SCIP* get_scip_pointer();
        InstanceUCP* get_instance();



    protected:


        InstanceUCP *m_instance_ucp;
        SCIP *m_scip_master;


        //* constraints : defined in derived class

};



#endif