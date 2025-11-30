
#pragma once

#include "Node.h"

class Double: public Cloneable<Double> {
private:
	double number;
public:
	Double(double n, location_t loc): Cloneable<Double>(loc), number(n) {}
	Double(const Double& d, TypeSubs& ts): Cloneable<Double>(d.getLoc()), number(d.number){}

	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;
	virtual bool isConstExpr() override {
		return true;
	}
	virtual DataType getDataType() override {
		return tdouble;
	}
};
