/** 
 * @class FormulationPricer
 ** 
*/

#ifndef FormulationPricer_H
#define FormulationPricer_H


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

// Decomposition
#include "Decomposition/FormulationPricer.h"



class FormulationPricer
{

    public : 

        /** 
         * constructor
        */
        FormulationPricer(InstanceUCP *instance, SCIP *scip );

        /** 
         * virtual destructor
        */
        virtual ~FormulationPricer()
        {}

        /** 
         * Pure virtual function
         *
        */
        virtual SCIP_RETCODE create_variables() = 0;
        /** 
         * Pure virtual function
         *
        */
        virtual SCIP_RETCODE create_constraints() = 0;



        //* gets

        SCIP* get_scip_pointer();
    

    protected:

        SCIP *m_scip_pricer;
        InstanceUCP *m_instance_ucp;

        //* variables : defined in derived classes

        //* constraints : defined in derived classes

        //* reduced costs : defined in derived classes

};

#endif