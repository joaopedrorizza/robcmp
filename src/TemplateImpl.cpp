#include "TemplateImpl.h"
#include "Program.h"
#include "FunctionImpl.h"
#include "BuildTypes.h"
#include "TemplateParamNode.h"
#include "semantic/PropagateTypes.h"
#include "semantic/Visitor.h"
#include <memory>

extern std::unique_ptr<BuildTypes> buildTypes;

Node* TemplateImpl::accept(Visitor &v) {
    return v.visit(*this);
}

string TemplateImpl::mangleName(string baseName, string returnType, vector<string> &params) {
    //MONOMORFIZAÇÃO
    // Monta o nome instanciado: Exemplo: swap<int8,int16>
    // Este nome é necessário para o linker e para o cache.
    std::string instantiatedName = baseName + "<";
    for (size_t i = 0; i < params.size(); ++i) {
        instantiatedName += params[i];
        if (i + 1 < params.size())
            instantiatedName += ",";
    }
    instantiatedName += ">";
    return instantiatedName;
}

Node* TemplateImpl::generateFor(const vector<string> &concreteTypes) {
    
    if (params.size() != concreteTypes.size()) {
        yyerrorcpp("Template instantiation failed: mismatched params", this);
        return nullptr;
    }

    //TODO: Verificar se o template foi instanciado para os tipos concretos recebidos. Se sim, apenas retornar. Se não, deixa continuar abaixo.

    

    std::map<std::string, DataType> substitutionMap;
    for (size_t i = 0; i < params.size(); ++i) {
        std::string paramName = params[i]->getName();
        DataType dt = buildTypes->getType(concreteTypes[i]);
        if (dt == BuildTypes::undefinedType) {
            yyerrorcpp("Unknown type '" + concreteTypes[i] + "' in template instantiation.", this);
            return nullptr;
        }
        substitutionMap[paramName] = dt;
        //std::cerr << "Template param: " << paramName << " → " << buildTypes->name(dt) << std::endl;
    }

    // TODO: encontra o tipo de retorno da função, no substitutionMap, se for parâmetro
    DataType rdt = buildTypes->getType(templ_dt);
    if (rdt == BuildTypes::undefinedType) {
        yyerrorcpp("Unknown type '" + templ_dt + "' in template instantiation.", this);
        return nullptr;
    }


    //FunctionImpl *fi = new FunctionImpl(rdt, 
    //DataType dt, string name, FunctionParams *fp, vector<Node*> &&stmts, location_t loc, 
	//	location_t ef, bool constructor = false
    //return fi;
    // depois de implementar, você vai ver um .ll que apresenta a função concreta, mas a
    // chamada vai ser removida (por hora)

    return nullptr;
}

/*FunctionImpl* TemplateImpl::instantiate(const std::string &instantiatedName, 
                                        const std::vector<std::string> &concreteTypes) {
    ...

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
        yyerrorcpp("TemplateImpl does not contain a valid FunctionImpl.", this);
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
    return nullptr;
}*/

