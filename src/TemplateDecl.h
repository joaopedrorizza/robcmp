// TemplateDecl.h (CORRIGIDO)
#pragma once

#include "Node.h"
#include "FunctionDecl.h"
#include "TemplateParamNode.h" // Inclui a definição correta do parâmetro
#include <vector>
#include <string>
#include <algorithm> // Necessário para std::move, embora o loop seja melhor

class FunctionImpl; // Forward declaration para o método instantiate

class TemplateDecl : public Node {
public:
    std::vector<TemplateParamNode*> params;
    FunctionImpl *fnImpl; // ✅ troque de FunctionDecl* para FunctionImpl*

    TemplateDecl(std::vector<Node*> *tpl_params, 
                 FunctionImpl *fd, 
                 location_t loc)
        : Node(loc), fnImpl(fd) {
        if (tpl_params) {
            for (Node* param_node : *tpl_params)
                params.push_back(static_cast<TemplateParamNode*>(param_node));
            delete tpl_params;
        }
    }

    FunctionImpl* instantiate(const std::string &instantiatedName, 
                              const std::vector<std::string> &concreteTypes);

    Node* accept(Visitor &v) override;
};
