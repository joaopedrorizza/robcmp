#include "TemplateFunctionDecl.h"
#include "FunctionImpl.h"
#include "semantic/Visitor.h"

// Construtor
TemplateFunctionDecl::TemplateFunctionDecl(TemplateDecl* tpl,
                                           const std::vector<int>& args,
                                           location_t loc)
    : FunctionDecl(tpl->fnImpl->getDataType(),   // tipo de retorno
                   tpl->fnImpl->getName(),       // nome
                   &tpl->fnImpl->getParameters(), 
                   tpl->fnImpl->getLoc()),       // localização
      templateDecl(tpl),
      typeArgs(args)
{
}

Node* TemplateFunctionDecl::accept(Visitor &v) {
    return v.visit(*this);
}
