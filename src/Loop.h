#ifndef __LOOP_H__
#define __LOOP_H__

#include "Node.h"

class Loop: public Cloneable<Loop> {
public:
	Loop(vector<Node*> &&stmts, location_t loc);
	Loop(const Loop& l, TypeSubs& ts);
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

};

#endif
