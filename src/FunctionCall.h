
#pragma once

#include "Node.h"
#include "ParamsCall.h"
#include "Identifier.h"
#include "Variable.h"

class FunctionCall: public Cloneable<FunctionCall> {
private:
	Identifier ident;
	Node *symbol = NULL;
	Variable *leftValue = NULL;

public:
	FunctionCall(const string& name, ParamsCall *pc, location_t loc): Cloneable<FunctionCall>(loc), ident(name, loc) {
		node_children.reserve(pc->getNumParams());
		node_children.insert(end(node_children), pc->getParameters().begin(),
			pc->getParameters().end());
		delete pc;
	}

	FunctionCall(const FunctionCall& f, TypeSubs& t) : Cloneable<FunctionCall>(f.getLoc()), 
		ident(f.ident) {}
	
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

	virtual DataType getDataType() override;

	virtual void setLeftValue(Variable *symbol) override {
		leftValue = symbol;
	}

	std::vector<Node *>& getParameters() {
		return node_children;
	}

	Node* accept(Visitor& v) override;

	const string getName() const override {
		return ident.getFullName();
	}

	Identifier& getIdent() {
		return ident;
	}

	void changeIdentifier(const string& id) {
		ident.changeIdentifier(id);
	}
};
