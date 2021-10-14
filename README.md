# Solving UCP with various decomposition and using Column Generation

### Welcome 

This code allows to apply different decompositions to get lower bounds on the MUCP
Works with SCIP 7.0.2
Developped by Alexis Schneider from June to September 2021.

### TODO sur le code :
Améliorer la lecture du main ; y'a trop de choses dedans.
Pour cela, il faut faire deux choses :
- bouger les fonctions d'appels de SCIP dans les constructeurs des décompositions
- piloter la résolution plus facilement par un fichier d'entrée, qui dit où trouver la data, quelles décompositions faire, et d'autres parametres importants. Pour l'instant on peut passer quelques réglages par le terminal mais c'est trop limité et très peu évolutif.


## How to use this code

### Preparing the executable

You must first "make" the executable. 
Make sure to have everything set up. SCIP must be downloaded and compiled. No additional libraires are needed. 
Update the makefile with your local scip directory.
use make to compile the code into an executable that you can find in the bin folder.
You can use it from the terminal, directly or by using a script ( an example was left in the script folder )


### parameters 
compilation :
make LPS=cpx to use CPLEX instead of SOPLEX (default solver)
executable :
./ucp.exe   entry_file_name (text) exit_file_name(text) must_write_data_out(boolean : 0 or 1)


## How is this code structured

If you are interested in adding new features and new decompositions, here is a small guide to understand how everything is designed and works.

#### Basis of solving classes 
The way the code was designed, to solve a simple problem, we create a Formulation class.
When constructed, formulation objects initialize by creating the variable, and then the constraints. This is the general cases ; with decomposition, it becomes a bit more complex.
Formulation objects stores informations about the variables, and when usefull, the constraints. The constraints are not necessearly usefull to store. They are added at the same time as created to the SCIP problem. You must store them when you want to get their dual values.
I did not create a Formulation class, that all the FormulationSomething would inherit from, but it would be a needed addition at some point. This is a flaw in the design.


#### And for a decomposition ?
For a decomposition, it becomes more complex.
SCIP allows to make column generation, but it is not easy to implements. Expect that it is a difficult process.
You must have :
- a new FormulationMaster class, that inherits from the FormulationMaster class. It allows to add columns to the problem, and the constraints. This is the problem that SCIP is trying to solve.
- A new FormulationPricer class, that inherits from the FormulationMaster class, is also needed. It is created at the beggining of the column generation ; but you get duals values that changes from iteration to iteration, and that changes the coefficient of the variables in the objective function. This operation is done through a function from the class.
- A new ObjPricer class, that inherits from the ObjPricer class from SCIP. SCIP recquires to have this class, with a bunch of functions. In the resolution, it is there that you get the dual values, feed them to the pricer problem, and look at the result to decide weither to add new columns. If a new column is added, SCIP will automaticaly solve the Master Problem again.


### Source code

In the src folder, you can find all the code in .cpp or .h files. It is organised the following way :

#### main.cpp : 
The main function pilote what is done in the executable
There is also a function for data importation from the parameters given from the terminal entry.
Right now the main function is really messy, and needs to be cleaned up, as poited in the #todo list.

#### DataClasses folder :
Classes that allows to store data about the instances and the solutions produced. 
Quite different concepts but I wanted to put them in the same place to clear up the src folder.

Instance.cpp and Instance.h : 
An InstanceUCP objects stores all the data about the instance that we want to solve. This was the first class made ; it is very important because it is used all over the code. Be carefull if you want to modify it, changing an int to a double may create errors in many parts of the code.

ProductionPlan.cpp and .h :
A solution to an instance of the UCP is a production plan. At the end of the solving, you must put the values of the SCIP variables into double vectors, and create a production plan from them.
These objects are used in the decompositions. Once again be carefull if you want to modify it !


#### OtherResolution folder  
Here is stored the compact formulation and the linear relaxation formulation. There are exactly the same, the only difference is that the x and u variables are linear instead of being binaries.


#### Decomposition folder :
Here are the classes that allows you to implement a decomposition, they are described earlier
A VariableMaster object is a column. It is the same, right now, for every decomposition. It could be possible to make a child class VariableMasterNewDecomposition for every new decomposition, but it was not needed so far.


#### Template decomposition 
Here are some templates for the classes. They allows you to gain some time, but be carefull, you need to make sure that you have gone over every part of the classes for your new decomposition to work. Be really carefull about copy and paste, it leads to errors that are sometime really hard to find, especially when your code is working but giving wrong results.


### Decompositions 
At the end of the internship, there was a design change to decomposition (create a single pricing problem and change its coefficient values instead of creating a new one at each iterations, which was bad design). But it was not implemented on every decomposition. The best design should be switch decomposition because it was the last one made.

#### Unit Decomposition 2
Here is the unit decomposition, with the production constraints in the subproblem.
It is the second decomposition implemented (Unit Decomposition 1 was deleted, it was more a test), so the structure is quite old. 

#### Unit Decomposition 3 
The unit decomposition with production constraitns in the master problem

#### Time Decomposition 
The classical Time Decomposition, see the latex document.

#### Time Decomposition 2
Time Decomposition 2 from the latex document, with Pmax in the knapsack constraints of the subproblem

#### Time Decomposition 2 Pmin and Pmax
You change the costs in the objective function of the subproblem, and it leads to changing some parts of the master problem as well

# Switch Decomposition 2
This is the startup decomposition from the latex document
You must specify weither you want to use the inequality and the parameter for the cost repartition.

#### Switch decomposition 
This is the startup decompositon, the only difference is that you create a variable u in the master problem ; this allows to implement easily the decomposition, but slows it. This is described in "how to implement a decomposition", see below.

# Overlapping decomposition
The overlapping between Unit and Time decomposition
You can choose to use the inequality, and the coefficients for costs repartition




## How do i create a new decomposition ?

To create a new decomposition, the first thing to do is to have it written well somewhere. This is very important, because small errors can lead to annoying trouble later. Start by writting it clean with latex.

Then use the templates, putting the right things at the right places. Avoid copy paste as much as possible, because it can created errors that are hard to find.

If you have a decomposition with a lot of dual costs (that is the case with time decompositions and interval decompositions), what you can do to have results faster is to implement, in the master problem, a variable, for example x = sum( delta_pi * pi^x). This way, you can replace the columns by the variable in the constraints. That means that you will only use the reduced costs from the constraints x = sum( delta_pi * pi^x). 
This allows you to implement the column generation faster. It will have the same result as the original one, but it will be slower.
Once you get it working, you can replace the x by the delta_pi * pi^x on every constraints, one by one, and put the right reduced costs in the pricer objective function, checking if it does not change the result. This way, you can "start easy, get more complicated".

Some decomposition might recquire some adaptation, for example if it is an overlapping decomposition, you will need 2 pricer problem classes, or if you want to add some functionnalities, some parameters, which the template do not cover.

Another thing to be carefull : when you make, you create .o objects in the obj/static/o.linux.../ folder. You must create a folder MyNewDecomposition here, where the compilation will put the .o objects

I've used VS Code to develop this. It allows to debug the code, which is an interesting feature because you can easily see where things get wrong.

## Good luck !


