#pragma once

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

    void propagateChildren(Node& n) {
        auto children_copy = n.node_children;
        auto itorig = n.node_children.begin();
        for (auto it = children_copy.begin(); it != children_copy.end(); ++it) {
            Node *replace = (*it)->accept(*this);
            if (replace) {
                *itorig = replace;
            }
            ++itorig;
        }
    }
    
    virtual Node* visit(Node& n) override {
        propagateChildren(n);
        return NULL;
    }

    virtual Node* visit(TemplateCall& n) override {

        ParamsCall *newParams = new ParamsCall();
        for (Node *param : n.getParameters()) {
            newParams->append(param);
        }

        const std::string baseName = n.getIdent().getFullName();

        // Find theTemplateImpl 
        TemplateImpl *templImpl = nullptr;
        Identifier templ_ident(baseName, n.getLoc());
        if (Node *sym = templ_ident.getSymbol(n.getScope())) {
            if (auto *ti = dynamic_cast<TemplateImpl *>(sym))
                templImpl = ti;
        }
        assert(templImpl && "Template implementation not found.");

        // 2) instanciar
        FunctionImpl *concreteFunc = templImpl->generateFor(n.getTemplateParams()->getTemplateParameters(), n);
        Node *templateScope = templImpl->getScope();
        if (!concreteFunc->getScope()) {
            concreteFunc->setScope(templateScope);
            templateScope->addChild(concreteFunc);
            templateScope->addSymbol(concreteFunc);
        }

        FunctionCall *call = new FunctionCall(concreteFunc->getName(), newParams, n.getLoc());
        call->setScope(n.getScope());
        return call;
    }
};
