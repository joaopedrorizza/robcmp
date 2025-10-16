#pragma once

#include "TemplateDecl.h"
#include "FunctionDecl.h"
#include <vector>

class TemplateFunctionDecl : public FunctionDecl {
public:
    TemplateDecl* templateDecl;       // Declaração do template original
    std::vector<int> typeArgs;        // Índices/types concretizados do template

    TemplateFunctionDecl(TemplateDecl* tpl,
                         const std::vector<int>& args,
                         location_t loc);

    virtual Node* accept(Visitor &v) override;
};
