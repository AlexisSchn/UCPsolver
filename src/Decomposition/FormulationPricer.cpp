/**
 * @file FormulationPricer.cpp
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

// Decomposition
#include "Decomposition/FormulationPricer.h"



//** Namespaces
using namespace std;





FormulationPricer::FormulationPricer(InstanceUCP *instance, SCIP *scip ): 
    m_scip_pricer(scip), m_instance_ucp(instance)
{
}


SCIP* FormulationPricer::get_scip_pointer()
{
    return( m_scip_pricer );
}

