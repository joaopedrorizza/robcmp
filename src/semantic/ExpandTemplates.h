#pragma once

#include "PropagateTypes.h"
#include "Visitor.h"
#include "Identifier.h"
#include "TemplateImpl.h"
#include "TemplateCall.h"
#include "FunctionCall.h"
#include "Program.h"
#include "BuildTypes.h"
#include "Variable.h"
#include "Return.h"
#include "ParamsCall.h"
#include <map>
#include <iostream>

extern std::unique_ptr<BuildTypes> buildTypes;

class ExpandTemplates : public Visitor
{
private:
    std::map<std::string, DataType> *subMap = nullptr;

    void propagateChildren(Node &n)
    {
        for (auto it = n.node_children.begin(); it != n.node_children.end(); ++it)
        {
            Node *replace = (*it)->accept(*this);
            if (replace)
            {
                *it = replace;
            }
        }
    }

public:
    ExpandTemplates() {}

    void setSubstitutionMap(std::map<std::string, DataType> *map)
    {
        subMap = map;
    }

    virtual Node *visit(Node &n) override
    {
        propagateChildren(n);
        return nullptr;
    }

    Node *visit(TemplateCall &n) override
    {
        propagateChildren(n);

        std::string baseName = n.getIdent().getFullName();
        Identifier templ_ident(baseName, n.getLoc());
        Node *templ_symbol = templ_ident.getSymbol(n.getScope());
        if (!templ_symbol)
        {
            yyerrorcpp("Template function not defined: " + baseName, &n);
            return nullptr;
        }

        TemplateImpl *templ = dynamic_cast<TemplateImpl *>(templ_symbol);
        if (!templ)
        {
            yyerrorcpp(string_format("%s is not a template declaration.", baseName.c_str()), &n);
            return nullptr;
        }

        Node *targetFunc = templ->generateFor(n.getTemplateParams());

        if (!targetFunc)
        {
            yyerrorcpp("Template generation failed for types: " + join(n.getTemplateParams(), ", "), &n);
            return nullptr;
        }

        std::string mangled = dynamic_cast<FunctionImpl *>(targetFunc)->getName();

        ParamsCall *args = new ParamsCall();
        for (Node *child : n.children())
            args->append(child);

        FunctionCall *concreteCall = new FunctionCall(mangled, args, n.getLoc());
        concreteCall->setScope(n.getScope());

        return concreteCall;
    }

    /*Node *visit(Return &r) override
    {
        propagateChildren(r);

        Return *newReturn = new Return(r.getLoc());
        for (Node *child : r.children())
        {
            newReturn->addChild(child);
        }

        return newReturn;
    }

    Node *visit(Variable &v) override
    {
        std::string typeName = buildTypes->name(v.getDataType());
        if (subMap && subMap->count(typeName))
        {
            DataType concreteType = (*subMap)[typeName];
            Variable *newVar = new Variable(v.getIdent().getFullName(),
                                            concreteType,
                                            v.getLoc());
            return newVar;
        }

        return nullptr;
    }*/
};