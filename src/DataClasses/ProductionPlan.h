/** 
 * @class ProductionPlan
 ** Objects from this class stores the information about a production plan. We can then put it inside a master variable.
*/

#ifndef PRODUCTIONPLAN_H
#define PRODUCTIONPLAN_H


//** Includes

//* Standart
#include <vector>
#include <iostream>


//* SCIP
#include <scip/scipdefplugins.h>
#include <scip/scip.h>


//* User

// general
#include "DataClasses/InstanceUCP.h"



class ProductionPlan
{

    public:
    
    ProductionPlan( InstanceUCP* instance_ucp, 
        std::vector < std::vector  < double > > up_down_plan, 
        std::vector < std::vector  < double > > switch_plan, 
        std::vector < std::vector  < double > > quantity_plan 
    );

    ~ProductionPlan();

    
    void show();
    
    void compute_cost();

    double get_cost();


    std::vector< std::vector< double > > get_up_down_plan();
    std::vector< std::vector< double > > get_switch_plan();
    std::vector< std::vector< double > > get_quantity_plan(); 


    
    private:
    
    InstanceUCP* m_instance_ucp;
    
    std::vector < std::vector  < double > >    m_up_down_plan;
    std::vector < std::vector  < double > >    m_quantity_plan;
    std::vector < std::vector  < double > >    m_switch_plan;

    double m_cost;
};


#endif