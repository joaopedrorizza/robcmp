
#pragma once

#include "Node.h"
#include "semantic/Visitor.h"

class BinaryOp: public Cloneable<BinaryOp> {
private:
	int op;

public:
	BinaryOp(Node *l, int op, Node *r);

	BinaryOp(const BinaryOp& b, TypeSubs& ts) : Cloneable<BinaryOp>(b), op(b.op) {}

	Value *logical_operator(enum Instruction::BinaryOps op, 
		FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock);

	Value *binary_operator(enum Instruction::BinaryOps opint, 
		enum Instruction::BinaryOps opflt, FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock);

	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

	int getOperator() const { return op; };

	virtual DataType getDataType() override;

	virtual bool isConstExpr() override;
	
	Node *lhsn() {
		return node_children[0];
	}

	Node *rhsn() {
		return node_children[1];
	};

	Node* accept(Visitor &v) override {
		return v.visit(*this);
	}
};
