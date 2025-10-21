/*#pragma once

#include "BuildTypes.h"
#include "Node.h"
#include "FunctionParams.h"
#include "FunctionAttributes.h"
#include "FunctionDecl.h"

class TemplateDecl : public FunctionBase
{
protected:
    vector<Node *> params;
    string templ_dt;

public:
    TemplateDecl(DataType dt, string name, FunctionParams *fp, vector<Node *> &&tpl, location_t loc) : FunctionBase(BuildTypes::undefinedType, name, fp, loc)
    {
        this->params = tpl;
        this->templ_dt = templ_dt;
        declaration = true;
    }

    virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;

    virtual Value *getLLVMValue(Node *, FunctionImpl *) override;

    Node *accept(Visitor &v) override;
};*/
