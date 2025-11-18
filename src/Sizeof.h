#pragma once

#include "Node.h"

class Sizeof: public Cloneable<Sizeof> {
private:
    Node *type;

public:
	Sizeof(Node* type, location_t loc): Cloneable<Sizeof>(loc), type(type) {
        node_children.push_back(type);
    }
	
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override {
        DataType dt = type->getDataType();
        if (dt == BuildTypes::undefinedType) {
            yyerrorcpp(string_format("Unknown type for %s.", type->getName().c_str()), this);
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
