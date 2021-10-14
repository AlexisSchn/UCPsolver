/**
 * @file VariableMaster.cpp
 * 
 * 
*/


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



//** Namespaces
using namespace std;



VariableMaster::VariableMaster( SCIP_VAR* column_variable, ProductionPlan* production_plan )
{
    m_scip_variable = column_variable;
    m_production_plan = production_plan;
}

void VariableMaster::add_block_number( int block_number )
{
    m_block_number = block_number;
}



//* gets 

SCIP_VAR* VariableMaster::get_variable_pointer()
{
    return( m_scip_variable );
}

ProductionPlan* VariableMaster::get_production_plan()
{
    return( m_production_plan );
}

int VariableMaster::get_block_number()
{
    return( m_block_number );
}









