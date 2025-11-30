#pragma once

#include "Node.h"

class Float: public Cloneable<Float> {
private:
	float number;
public:
	Float(float n, location_t loc): Cloneable<Float>(loc), number(n) {}
	
    Float(const Float& other, TypeSubs& ts)
        : Cloneable<Float>(other.getLoc()), number(other.number) {}

	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

	virtual bool isConstExpr() override {
		return true;
	}
	
	virtual DataType getDataType() override {
		return tfloat;
	}
};
