#include "TemplateCall.h"
#include "FunctionCall.h"
#include "BuildTypes.h"
#include "FunctionDecl.h"
#include "FunctionImpl.h"
#include "TemplateDecl.h"
#include "HeaderGlobals.h"
#include "Load.h"
#include "BackLLVM.h"
#include "UserType.h"
#include "Program.h"
#include "Interface.h"
#include "semantic/PropagateTypes.h"
#include "semantic/Visitor.h"

// testando github

extern Program *program;

Value *TemplateCall::generate(FunctionImpl *func,
                                      BasicBlock *block,
                                      BasicBlock *allocblock) {
    /*
    RobDbgInfo.emitLocation(this);

    std::string baseName = ident.getFullName();
    

    //MONOMORFIZAÇÃO
    // Monta o nome instanciado: Exemplo: swap<int8,int16>
    // Este nome é necessário para o linker e para o cache.
    std::string instantiatedName = baseName + "<";
    for (size_t i = 0; i < templateParams.size(); ++i) {
        instantiatedName += templateParams[i];
        if (i + 1 < templateParams.size())
            instantiatedName += ",";
    }
    instantiatedName += ">";

    std::cerr << "Instantiated: " << instantiatedName << std::endl;

    // ------------------------------------------------------------------
    // PASSO 1: VERIFICAÇÃO DE CACHE E INSTANCIAÇÃO
    // ------------------------------------------------------------------
    
    // Tenta buscar a função instanciada primeiro.
    std::cerr << "Looking for instantiated function in cache: " << instantiatedName << std::endl;

    FunctionImpl *concreteFunc = program->getInstantiatedTemplate(instantiatedName); 

    if (!concreteFunc) {
        // 1.1 Busca o TemplateDecl original
        TemplateDecl *templDecl = nullptr;
        for (Node *child : program->children()) {
            templDecl = dynamic_cast<TemplateDecl*>(child);
            if (templDecl) {
                std::cerr << "Found TemplateDecl with fnImpl name: " << templDecl->fnImpl->getFinalName() << std::endl;
            }
            if (templDecl && templDecl->fnImpl->getFinalName() == baseName)
                break;
        }

        if (!templDecl) {
            yyerrorcpp("Template function " + baseName + " not defined.", this);
            return NULL;
        }
        
        // 1.2 REALIZA A INSTANCIAÇÃO (CLONAGEM E SUBSTITUIÇÃO DE TIPOS)
        // Isso é a parte que falta: O compilador deve clonar o corpo da função 
        // e substituir todos os TemplateParam pelo tipo concreto da chamada (templateParams).
        
        // **ESTE É ONDE VOCÊ DEVE INSERIR A LÓGICA DE INSTANCIAÇÃO DO SEU COMPILADOR**
        // Em um compilador de verdade, isso é feito por um método dedicado (e.g., TemplateInstantiator).
        
        // Exemplo simplificado (você precisa implementar o clone-e-substitui):
        concreteFunc = templDecl->instantiate(instantiatedName, templateParams); 
        std::cerr << "Instantiated function created: " << concreteFunc->getName() << std::endl;
        std::cerr << "Adding instantiated function to program and cache." << std::endl;


        
        if (!concreteFunc) {
            yyerrorcpp("Error instantiating template " + instantiatedName + ".", this);
            return NULL;
        }

        // 1.3 Adiciona ao programa e ao cache
        program->addChild(concreteFunc);
        program->cacheInstantiatedTemplate(instantiatedName, concreteFunc); 
        program->addSymbol(concreteFunc);
        std::cerr << "Function inserted via addSymbol: " << concreteFunc->getName() << std::endl;

        // ✅ Geração do corpo da função instanciada
        BasicBlock *tempAlloc = BasicBlock::Create(global_context, "", concreteFunc->getLLVMFunction());
        BasicBlock *tempBody = BasicBlock::Create(global_context, "body", concreteFunc->getLLVMFunction());
        concreteFunc->generate(concreteFunc, tempBody, tempAlloc);
    }

    // ------------------------------------------------------------------
    // PASSO 2: DELEGAÇÃO PARA FUNCTIONCALL CONCRETO
    // ------------------------------------------------------------------
    
    // A chamada de função real DEVE usar o nome instanciado.
    ParamsCall *pc = new ParamsCall();
    for (Node *n : node_children) {
        pc->append(n); 
    }

    // Cria um FunctionCall concreto com o nome instanciado
    std::cerr << "Creating FunctionCall for: " << instantiatedName << std::endl;
    FunctionCall *concreteCall = new FunctionCall(instantiatedName, pc, getLoc());

    // ... (Herança de escopo e leftValue, já OK) ...
    concreteCall->setScope(getScope());
    if (leftValue)
        concreteCall->setLeftValue(leftValue);

    // Agora delega geração para FunctionCall normal (correto)
    Value *val = concreteCall->generate(func, block, allocblock);

    // ... (Propaga tipo de retorno, já OK) ...

    return val;
    */
    return nullptr;
}

Node* TemplateCall::accept(Visitor& v) {
    return v.visit(*this);
}
