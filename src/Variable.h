
#pragma once

#include "Node.h"
#include "Identifier.h"

enum PointerMode {pm_unknown, pm_pointer, pm_nopointer};

class Variable: public Cloneable<Variable, NamedNode> {
protected:
    Value *alloc = NULL;
    Identifier ident;
	int gepIndex = -1;
	enum PointerMode pointer = pm_unknown;
	bool pseudoVar = false;
	string dts;

public:
    Variable(const string &name, location_t loc): Cloneable<Variable, NamedNode>(name, loc), ident(name, loc) {}
	
	Variable(const string &name, DataType dt, location_t loc): Cloneable<Variable, NamedNode>(name, loc), ident(name, loc) {
		this->dt = dt;
	}

	Variable(const string &name, string dts, location_t loc): Cloneable<Variable, NamedNode>(name, loc), ident(name, loc) {
		this->dts = dts;
	}

	Variable(const Variable& v) : Cloneable<Variable, NamedNode>(v), ident(v.ident), dts(v.dts) {}

    virtual Value* getLLVMValue(Node *stem, FunctionImpl *gfunc = NULL) override;
	
    virtual void setGEPIndex(int idx) {
		gepIndex = idx;
	}

	int getGEPIndex() const {
		return gepIndex;
	}

	void setPointer(enum PointerMode v) {
		pointer = v;
	}

	enum PointerMode getPointerMode() {
		return pointer;
	}

	void setAlloca(Value *alloc) {
		this->alloc = alloc;
	}

	virtual Node *getExpr() {
		return NULL;
	}

	virtual void setExpr(Node *e) {}

	Identifier& getIdent() {
		return ident;
	}

	void setPseudoVar(bool value) {
		pseudoVar = value;
	}

	bool isPseudoVar() {
		return pseudoVar;
	}

	virtual DataType getDataType() override {
		if (dt == BuildTypes::undefinedType) {
			dt = buildTypes->getType(dts, true);
		}
		return dt;
	}

	string getDataTypeName() {
		return dts;
	}
};
