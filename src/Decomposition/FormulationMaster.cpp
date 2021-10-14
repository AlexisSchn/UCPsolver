/**
 * @file FormulationMaster.cpp
 *  Implements the class FormulationMaster 
 * 
*/


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



//** Namespaces
using namespace std;



FormulationMaster::FormulationMaster( InstanceUCP* instance, SCIP* scip_master)
{
    m_instance_ucp = instance;
    m_scip_master = scip_master;
}




SCIP* FormulationMaster::get_scip_pointer()
{
    return( m_scip_master );
}


InstanceUCP* FormulationMaster::get_instance()
{
    return( m_instance_ucp );
}






