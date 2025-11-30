#pragma oncd

#include "Node.h"
#include "semantic/Visitor.h"

class CmpOp: public Cloneable<CmpOp> {
private:
	Node *left;
	Node *right;
	int op;

public:
	CmpOp (Node *l, int op, Node *r);
    CmpOp(const CmpOp &other, TypeSubs &ts);

	int getOperator() const { return op; };
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;
	Node *lexpn() {
		return node_children[0];
	}
	Node *rexpn() {
		return node_children[1];
	}

	Node* accept(Visitor &v) override {
		return v.visit(*this);
	}
};
