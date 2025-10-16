
#pragma once

#include "Visitor.h"
#include "Identifier.h"
#include "TemplateDecl.h"
#include "TemplateFunctionCall.h"

class ExpandTemplates: public Visitor {
public:
	ExpandTemplates() {}

    virtual Node* visit(Node& n) override {
        for (auto it = n.node_children.begin(); it != n.node_children.end(); ++it) {
            Node *replace = (*it)->accept(*this);
            if (replace) {
                *it = replace;
            }
        }
        return NULL;
    }

    virtual Node* visit(TemplateFunctionCall& n) override {

        /*
            Encontrar o template com o nome da função sendo chamada
            Verificar se o template já foi gerado com os argumentos do template sendo chamado, exemplo:
                Chamada: swap#int8(12, 25);
                Encontrar o template swap e verificar se ele foi implementado para o argumento int8

            Trocar o nó TemplateFunctionCall por uma chamada normal para swap#int8 monomorfizada

        */

        Identifier templ_ident(n.getName(), n.getLoc());
        Node *templ_symbol = templ_ident.getSymbol(n.getScope());
	    if (!templ_symbol)
		    return NULL;

        TemplateDecl *templ = dynamic_cast<TemplateDecl*>(templ_symbol);
        if (!templ) {
            yyerrorcpp(string_format("%s is not a template declaration.", n.getName().c_str()).c_str(), &n);
            return NULL;
        }

        Node *fi = templ->generateFor(n.getTemplateParams());
        return fi;
    }
};
