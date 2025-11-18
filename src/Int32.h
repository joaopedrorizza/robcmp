
#pragma once

#include "Node.h"

class Int32: public Cloneable<Int32> {
private:
	int number;
	
public:
	Int32(int n, location_t loc): Cloneable<Int32>(loc), number(n) {}
	
	Int32(const Int32& n, TypeSubs& ts): Cloneable<Int32>(n.getLoc()), number(n.number) {}
	
	int getNumber() const { return number; }

	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

	virtual bool isConstExpr() override {
		return true;
	}

	virtual DataType getDataType() override {
		return tint32;
	}
};

class UInt32: public Node {
private:
	unsigned int number;
	
public:
	UInt32(unsigned int n, location_t loc): Node(loc), number(n) {}
	
	unsigned int getNumber() const { return number; }

	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

	virtual bool isConstExpr() override {
		return true;
	}

	virtual DataType getDataType() override {
		return tint32u;
	}
};
