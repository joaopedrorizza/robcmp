#pragma once

#include "Node.h"
#include "Identifier.h"
#include "Variable.h"
#include "semantic/Visitor.h"

class Load: public Cloneable<Load> {
private:
	Identifier ident;
	Variable *leftValue = NULL;
	Node *identSymbol = NULL;
	
public:
	Load(const char* i, location_t loc): Cloneable<Load>(loc), ident(i, loc) {}
	Load(Identifier i): Cloneable<Load>(i.getLoc()), ident(i.getFullName(), i.getLoc()) {}
	Load(Node *n): Cloneable<Load>(n->getLoc()), ident(n->getName(), n->getLoc()) {
		identSymbol = n;
	}

Load(const Load& o, TypeSubs& ts)
    : Cloneable<Load>(o, ts),
      ident(o.ident),
      leftValue(nullptr),
      identSymbol(nullptr)
{
    // preserve dt, substituting if needed
    dt = o.dt;

    // if original had an identSymbol, try to re-resolve in current scope
    if (o.identSymbol) {
        // prefer re-resolving by identifier in the current scope (if available)
        // Otherwise clone the symbol node
        if (Node *sym = ident.getSymbol(getScope())) {
            identSymbol = sym;
        } else {
            // fallback: clone the original symbol node (preserve structure)
            identSymbol = o.identSymbol->cloneTree(ts);
        }
    }

    // leftValue (if present) clone
    if (o.leftValue) {
        // static_cast is ok if leftValue is always Variable* or derived
        leftValue = dynamic_cast<Variable*>(o.leftValue->cloneTree(ts));
    }
}

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
