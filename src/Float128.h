#pragma once

#include "Node.h"

class Float128: public Cloneable<Float128> {
private:
	long double number;
public:
	Float128(long double n, location_t loc): Cloneable<Float128>(loc), number(n) {}
	Float128(const Float128& f, TypeSubs& ts): Cloneable<Float128>(f.getLoc()), number(f.number) {}
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;
	
	virtual bool isConstExpr() override {
		return true;
	}
	
	virtual DataType getDataType() override {
		return tldouble;
	}
};

