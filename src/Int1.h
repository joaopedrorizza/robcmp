
#pragma once

#include "Node.h"

class Int1: public Cloneable<Int1> {
private:
	bool value;

public:
	Int1(bool v, location_t loc): Cloneable<Int1>(loc), value(v) {}

	Int1(const Int1& v, TypeSubs& ts): Cloneable<Int1>(v.getLoc()), value(v.value) {}

	bool getValue() const { return value; }
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

	virtual bool isConstExpr() override {
		return true;
	}
	
	virtual DataType getDataType() override {
		return tbool;
	}

};
