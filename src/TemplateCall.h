#pragma once

#include "Node.h"
#include "Identifier.h"
#include "ParamsCall.h"
#include "FunctionCall.h"
#include "Variable.h"

class TemplateCall : public Node
{
private:
	Identifier ident;
    std::vector<std::string> templateArgs_; 
   Node *symbol = NULL;
	Variable *leftValue = NULL;

public:
    TemplateCall(const string& name, std::vector<std::string> &&tplArgs, ParamsCall *pc, location_t loc): Node(loc), ident(name, loc) {
      
    this->templateArgs_ = std::move(tplArgs);

    node_children.reserve(pc->getNumParams());
	node_children.insert(end(node_children), pc->getParameters().begin(),
	pc->getParameters().end());
		delete pc;
    }


    const std::vector<std::string> &getTemplateArgs() const { return templateArgs_; }
    //const std::string instantiate(const string& baseName); // <-- lowering

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
