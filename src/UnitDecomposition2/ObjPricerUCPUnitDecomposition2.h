/** 
 * @class Pricer.h
 ** Allows SCIP to make a column generation. Tell what is the pricing problem
*/

#ifndef ObjPricerUCPUnitDecomposition2_H
#define ObjPricerUCPUnitDecomposition2_H

//** Includes

//* Standart
#include <vector>


//* SCIP
#include "objscip/objpricer.h"
#include "scip/pub_var.h"
#include <scip/scip.h>


//* User 

// general
#include "DataClasses/InstanceUCP.h"

// Decomposition
#include "Decomposition/FormulationPricer.h"

// Unit Decomposition 2
#include "UnitDecomposition2/FormulationMasterUnitDecomposition2.h"
#include "UnitDecomposition2/FormulationPricerUnitDecomposition2.h"

//** Namespace 
using namespace scip;

class ObjPricerUCPUnitDecomposition2 : public ObjPricer
{

    public:

    /** Constructs the pricer object with the data needed */
    ObjPricerUCPUnitDecomposition2(
        SCIP* scip_master,                  /**< SCIP pointer */
        const char* name,                   /**< name of pricer */
        FormulationMasterUnitDecomposition2* formulation_master,
        InstanceUCP* instance_ucp
    );

    /** Destructs the pricer object. */
    virtual ~ObjPricerUCPUnitDecomposition2();

    /** reduced cost pricing method of variable pricer for feasible LPs */
    virtual SCIP_DECL_PRICERREDCOST(scip_redcost);

    /** initialization method of variable pricer (called after problem was transformed) */
    virtual SCIP_DECL_PRICERINIT(scip_init);

    SCIP_RETCODE SCIPpricerUCPActivate();

    /** perform pricing */
    void ucp_pricing(SCIP* scip);


    private:

    FormulationMasterUnitDecomposition2* m_formulation_master;
    InstanceUCP* m_instance_ucp;
    std::vector< double > m_list_RMP_opt;
};


#endif