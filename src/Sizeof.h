#pragma once

#include "Node.h"
#include "Load.h"

class Sizeof: public Cloneable<Sizeof> {
private:
    string targetDtName;

public:
	Sizeof(Node* type, location_t loc): Cloneable<Sizeof>(loc) {
        if (Load *ld = dynamic_cast<Load*>(type)) {
            targetDtName = ld->getName();
        } else {
            assert(false && "Sizeof should receive a Load of an indentifier.");
        }
    }
	
    Sizeof(const Sizeof& n, TypeSubs& ts) : Cloneable<Sizeof>(n),
        targetDtName(n.targetDtName) {
            
        auto newType = ts.find(targetDtName);
        if (newType != ts.end()) {
            targetDtName = buildTypes->name(newType->second);
        }
    }

	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override {
        DataType dt = buildTypes->getType(targetDtName);
        if (dt == BuildTypes::undefinedType) {
            yyerrorcpp(string_format("Unknown type for %s.", targetDtName.c_str()), this);
            return NULL;
        }
        uint16_t bytes = buildTypes->bitWidth(dt) / 8;
        return ConstantInt::get(Type::getInt16Ty(global_context), bytes, false);
    }

	virtual bool isConstExpr() override {
		return true;
	}

	virtual DataType getDataType() override {
		return tint16u;
	}

};
