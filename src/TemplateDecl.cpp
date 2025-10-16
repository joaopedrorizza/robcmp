#include "TemplateDecl.h"
#include "Program.h"
#include "FunctionImpl.h"
#include "BuildTypes.h"
#include "TemplateParamNode.h"
#include "semantic/PropagateTypes.h"
#include "semantic/Visitor.h"
#include <memory>

extern std::unique_ptr<BuildTypes> buildTypes;

Node* TemplateDecl::accept(Visitor &v) {
    return v.visit(*this);
}

FunctionImpl* TemplateDecl::instantiate(const std::string &instantiatedName, 
                                        const std::vector<std::string> &concreteTypes) {
    if (params.size() != concreteTypes.size()) {
        yyerrorcpp("Template instantiation failed: mismatched parameter count.", this);
        return nullptr;
    }

    std::map<std::string, DataType> substitutionMap;
    for (size_t i = 0; i < params.size(); ++i) {
        std::string paramName = params[i]->getName();
        DataType dt = buildTypes->getType(concreteTypes[i]);
        if (dt == BuildTypes::undefinedType) {
            yyerrorcpp("Unknown type '" + concreteTypes[i] + "' in template instantiation.", this);
            return nullptr;
        }
        substitutionMap[paramName] = dt;
        std::cerr << "Template param: " << paramName << " → " << buildTypes->name(dt) << std::endl;
    }

    FunctionImpl *originalImpl = fnImpl;
    if (!originalImpl) {
        yyerrorcpp("TemplateDecl does not contain a valid FunctionImpl.", this);
        return nullptr;
    }

    FunctionImpl *concreteFunc = originalImpl->clone(instantiatedName);
    if (!concreteFunc) {
        yyerrorcpp("Failed to clone template function.", this);
        return nullptr;
    }

    std::cerr << "Cloned function: " << concreteFunc->getName() << std::endl;

    auto subMapPtr = std::make_unique<std::map<std::string, DataType>>(substitutionMap);
    PropagateTypes substitutor(subMapPtr.get());

    // Substitui tipos no corpo, parâmetros e retorno
    //substitutor.propagateChildren(*concreteFunc);
    substitutor.visit(*concreteFunc);


    std::cerr << "Finished type substitution for: " << concreteFunc->getName() << std::endl;

    return concreteFunc;
}

