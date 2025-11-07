/*#pragma once

#include "PropagateTypes.h"
#include "ConstructorCall.h"
#include "FunctionCall.h"
#include "TemplateImpl.h"
#include "Scalar.h"
#include "Program.h"
#include "Node.h"
#include "FunctionImpl.h"
#include "TemplateCall.h"
#include "semantic/Visitor.h"

class ExpandTemplates: public Visitor {

    public:
    ExpandTemplates() {}

    void propagateChildren(Node& n, std::function<void(Node&)> lambda) {
    for (auto it = n.node_children.begin(); it != n.node_children.end(); ++it) {
        Node *replace = (*it)->accept(*this);
        if (replace) {
            *it = replace;
        }
        if (lambda)
            lambda(**it);
    }
}

    virtual Node* visit(TemplateCall& n){

         propagateChildren(n);

   ParamsCall *newParams = new ParamsCall();
    for (Node *param : n.getParameters())
    {
        newParams->append(param);
    }

    const std::string baseName = n.getIdent().getFullName();

    // 1) lookup do TemplateImpl 
    TemplateImpl *templImpl = nullptr;

     if (!n.getScope())
        templImpl =  nullptr;

    // 1) tenta resolver no escopo atual
    Identifier templ_ident(baseName, n.getLoc());
    if (Node *sym = templ_ident.getSymbol(n.getScope()))
    {
        if (auto *ti = dynamic_cast<TemplateImpl *>(sym))
             templImpl = ti;
              
    }

    // 2) fallback global no Program
    if (program)
    {
        Identifier glob(baseName, n.getLoc());
        if (Node *s = glob.getSymbol(program))
        {
            if (auto *ti2 = dynamic_cast<TemplateImpl *>(s))
                templImpl = ti2;
        }
    }

    if (!templImpl)
    {
        // NÃO é template → vira FunctionCall do nome base, usando os MESMOS argumentos
        auto *call = new FunctionCall(baseName, newParams, n.getLoc()); // <-- AQUI
        call->setScope(n.getScope());
        return call;
    }

    // 2) instanciar
    FunctionImpl *instNode = templImpl->generateFor(n.getTemplateArgs());

    auto *concreteFunc = dynamic_cast<FunctionImpl *>(instNode);
    if (!concreteFunc)
    {
        yyerrorcpp("Template instantiation did not produce a FunctionImpl for '" + baseName + "'.", &n);
        return nullptr;
    }

    FunctionCall *call = new FunctionCall(concreteFunc->getName(), newParams, n.getLoc()); // <-- AQUI
    call->setScope(program);
    call->setDataType(concreteFunc->getDataType());

    program->addChild(call);

    return call;

    }
}*/