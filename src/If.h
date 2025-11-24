#pragma once

#include "Node.h"

class If: public Cloneable<If> {
private:
	Node *expr;
	Node *thenst;
	Node *elsest;
public:
	If(Node *e, vector<Node*> &&tst, location_t loc);
	If(Node *e, vector<Node*> &&tst, vector<Node*> &&est, location_t loc);
	 // Construtor de clone (obrigatório)
    If(const If& other, TypeSubs& ts);

	virtual Value* generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;
	virtual Node* accept(Visitor& v) override;
};
