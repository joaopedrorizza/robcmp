#pragma once

#include "Node.h"
#include "Identifier.h"
#include "semantic/Visitor.h"

class Load: public Cloneable<Load> {
private:
	Identifier ident;
	Variable *leftValue = NULL;
	Node *identSymbol = NULL;
	
public:
	Load(const char* i, location_t loc): Cloneable<Load>(loc), ident(i, loc) {	}
	Load(Identifier i): Cloneable<Load>(i.getLoc()), ident(i.getFullName(), i.getLoc()) { }
	Load(Node *n): Cloneable<Load>(n->getLoc()), ident(n->getName(), n->getLoc()) {
		identSymbol = n;
	}

	Load(const Load& o) : Cloneable<Load>(o), ident(o.ident) {}
 
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

	virtual DataType getDataType() override;

	virtual bool isConstExpr() override;

	virtual void setLeftValue(Variable *symbol) override;

	virtual const string getName() const override {
		return ident.getFullName();
	}

	virtual Node* accept(Visitor& v) override {
		return v.visit(*this);
	}

	Node* getIdentSymbol(bool showError = true);

	static Value* getRecursiveField(Identifier &ident, Node *scope, FunctionImpl *func);
	
	friend class SymbolizeTree;
};
