
#pragma once

#include "Node.h"
#include "FunctionDecl.h"

class TemplateImpl : public FunctionBase {
protected:
    vector<Node*> params;
    string templ_dt;
public:
	TemplateImpl(string templ_dt, string name, FunctionParams *fp, vector<Node*> &&stmts,
		vector<Node*> &&tpl, location_t loc, bool constructor = false) :
		FunctionBase(BuildTypes::undefinedType, name, fp, std::move(stmts), loc, constructor) {
        this->params = tpl;
        this->templ_dt = templ_dt;
	}

    Value *generate(FunctionImpl *, BasicBlock *, BasicBlock *allocblock) override {
        return nullptr;
    }

    Node *generateFor(const vector<string> &concreteTypes);

    string mangleName(string baseName, string returnType, vector<string> &params);

    Node* accept(Visitor &v) override;
};
