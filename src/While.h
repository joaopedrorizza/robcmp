
#pragma once

#include "Node.h"

class While: public Cloneable<While> {
private:
	Node *expr;
	Node *stmts;
	
public:
	While(Node *e, location_t loc);
	While(Node *e, vector<Node*> &&stmts, location_t loc);
	While(const While& other, TypeSubs& ts);
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

};
