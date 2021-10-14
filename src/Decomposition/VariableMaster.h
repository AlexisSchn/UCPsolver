/** 
 * @class VariableMaster
 ** Objects from this class stores the information about a master variable.
*/

#ifndef VARIABLEMASTER_H
#define VARIABLEMASTER_H


//** Includes

//* Standart
#include <vector>
#include <iostream>


//* SCIP
#include <scip/scipdefplugins.h>
#include <scip/scip.h>


//* User

// general
#include "DataClasses/ProductionPlan.h"

// Decomposition
#include "Decomposition/VariableMaster.h"




class VariableMaster
{

    public : 
    
    VariableMaster( SCIP_VAR* column_variable, ProductionPlan* production_plan );
    ~VariableMaster();

    void add_block_number(int block_number);



    //* gets

    SCIP_VAR* get_variable_pointer();
    ProductionPlan* get_production_plan();
    int get_block_number();


    private :

    ProductionPlan* m_production_plan;
    SCIP_VAR* m_scip_variable;

    char* p_variable_name;

    int m_block_number; /** < number of the block */



};



#endif