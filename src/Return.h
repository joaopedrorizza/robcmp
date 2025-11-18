
#pragma once

#include "Node.h"
#include "semantic/Visitor.h"

class Return: public Cloneable<Return> {
public:
	Return(location_t loc): Cloneable<Return>(loc) {
		dt = tvoid;
	}

	Return(Node *n): Cloneable<Return>(n->getLoc()) {
		addChild(n);
	}

	Return(const Return& r) : Cloneable<Return>(r) {}

	Node *value() {
		if (node_children.empty())
			return NULL;
		return node_children[0];
	}
	
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

	Node* accept(Visitor& v) override {
		return v.visit(*this);
	}
};
