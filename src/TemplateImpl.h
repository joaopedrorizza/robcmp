
#pragma once

#include "Node.h"
#include "FunctionDecl.h"
#include "FunctionImpl.h"
#include "BackLLVM.h"

class TemplateImpl : public FunctionBase
{
private:
    SourceLocation endfunction;
    BasicBlock *falloc = NULL;
    BasicBlock *fblock = NULL;

    DIFile *funit;

protected:
    vector<Node *> params;
    string returnType;

public:
    TemplateImpl(string returnType, string name, FunctionParams *fp, vector<Node *> &&stmts,
                 vector<Node *> &&tpl, location_t loc, location_t ef, bool constructor = false) : FunctionBase(BuildTypes::undefinedType, name, fp, std::move(stmts), loc, constructor), endfunction(ef)
    {
        this->params = tpl;
        this->returnType = returnType;

        this->declaration = false;
        funit = RobDbgInfo.currFile();

        for (auto p : fp->getParameters())
        {
            symbols[p->getName()] = p;
        }
    }

    Value *generate(FunctionImpl *, BasicBlock *, BasicBlock *allocblock) override;

    FunctionImpl *generateFor(const vector<Variable *> &concreteTypes, SourceLocation& loc);

    string mangleName(string baseName, string returnType_, vector<string> &params);

    Node *accept(Visitor &v) override;
};
