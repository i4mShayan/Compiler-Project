#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "AST.h"

class Optimizer
{
public:
 void remove_dead_variables(AST *Tree);

};
#endif