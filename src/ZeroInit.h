
#pragma once

#include "BuildTypes.h"
#include "HeaderGlobals.h"
#include "Node.h"

class ZeroInit: public Cloneable<ZeroInit> {
public:
	ZeroInit(DataType dt, location_t loc): Cloneable<ZeroInit>(loc) {
		this->dt = dt;
	}

	ZeroInit(const ZeroInit& zi, TypeSubs& ts): Cloneable<ZeroInit>(zi.getLoc()) {
		this->dt = zi.dt;
	}
	
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override {
		return ConstantAggregateZero::get(buildTypes->llvmType(dt));
        //return ConstantPointerNull::get(PointerType::getUnqual(buildTypes->llvmType(dt)));
    }

	virtual bool isConstExpr() override {
		return true;
	}
};
