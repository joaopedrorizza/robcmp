#pragma once

#include "Node.h"
#include "Identifier.h"
#include "ParamsCall.h"
#include "FunctionCall.h"
#include "TemplateParams.h"
#include "Variable.h"

class TemplateCall : public Node
{
private:
	Identifier ident;
	TemplateParams *tpl_parameters;
    Node *symbol = NULL;
	Variable *leftValue = NULL;

public:
    TemplateCall(const string& name, TemplateParams *tp, ParamsCall *pc, location_t loc): Node(loc), ident(name, loc) {
		this->tpl_parameters = tp;
      
    node_children.reserve(pc->getNumParams());
	node_children.insert(end(node_children), pc->getParameters().begin(),
	pc->getParameters().end());
		delete pc;
    }

    virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

	virtual DataType getDataType() override;

	virtual void setLeftValue(Variable *symbol) override {
		leftValue = symbol;
	}

	std::vector<Node *>& getParameters() {
		return node_children;
	}

	TemplateParams* getTemplateParams() {
		return tpl_parameters;
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
