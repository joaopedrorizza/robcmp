/*#include "TemplateCall.h"
#include "FunctionCall.h"
#include "BuildTypes.h"
#include "FunctionDecl.h"
#include "FunctionImpl.h"
#include "TemplateImpl.h"
#include "HeaderGlobals.h"
#include "Load.h"
#include "BackLLVM.h"
#include "UserType.h"
#include "Program.h"
#include "Interface.h"
#include "semantic/PropagateTypes.h"
#include "semantic/Visitor.h"
#include "ParamsCall.h"

extern Program *program;
extern std::unique_ptr<BuildTypes> buildTypes;

Value *TemplateCall::generate(FunctionImpl *func,
                              BasicBlock *block,
                              BasicBlock *allocblock)
{
    RobDbgInfo.emitLocation(this);

    std::string baseName = ident.getFullName();

    // 1) Encontrar o TemplateImpl correspondente no escopo atual
    Identifier templ_ident(baseName, getLoc());
    Node *templ_symbol = templ_ident.getSymbol(getScope());
    if (!templ_symbol) {
        yyerrorcpp("Template function " + baseName + " not defined.", this);
        return nullptr;
    }

    TemplateImpl *templImpl = dynamic_cast<TemplateImpl *>(templ_symbol);
    if (!templImpl) {
        yyerrorcpp(string_format("%s is not a template declaration.", baseName.c_str()).c_str(), this);
        return nullptr;
    }

    // 2) Pedir ao TemplateImpl para gerar (ou retornar cache) a função instanciada
    //    getTemplateParams() deve retornar vector<string> com os tipos concretos (ex: ["int8"])
    const std::vector<std::string> concreteTypes = this->getTemplateParams();

    Node *instNode = templImpl->generateFor(concreteTypes);
    if (!instNode) {
        yyerrorcpp("Error instantiating template " + baseName + ".", this);
        return nullptr;
    }

    // Esperamos que a instância retornada seja um FunctionImpl
    FunctionImpl *concreteFunc = dynamic_cast<FunctionImpl *>(instNode);
    if (!concreteFunc) {
        yyerrorcpp("Template instantiation did not produce a function for " + baseName + ".", this);
        return nullptr;
    }

    // 3) Se a instância não estiver registrada no programa, registrar
    //    (algumas implementações de generateFor já fazem isso — aqui garantimos)
    //    Usamos program->addChild / addSymbol como já utilizado no seu código.
    bool insertedToProgram = false;
    // tenta verificar se já existe - Program pode ter método find (se tiver, prefira); fallback:
    // Procurar pelo nome na tabela de símbolos do escopo global (program)
    // (Se program fornecer findFunction / getSymbol, usar diretamente — aqui uso addSymbol defensivo)
    // Se já existe um símbolo com o nome da função, presumimos que já esteja registrada.
    Identifier checkId(concreteFunc->getName(), getLoc());
    Node *existing = checkId.getSymbol(program);
    if (!existing) {
        program->addChild(concreteFunc);
        program->addSymbol(concreteFunc);
        insertedToProgram = true;
        std::cerr << "TemplateCall: registered instantiated function: " << concreteFunc->getName() << std::endl;
    }

    // 4) Criar ParamsCall com os argumentos originais desta TemplateCall
    //    (note: TemplateCall::node_children contém os argumentos)
    ParamsCall *pc = new ParamsCall();
    for (Node *arg : this->node_children) {
        // Não clonamos; assumimos que FunctionCall::generate irá usar os nós como estão
        pc->append(arg);
    }

    // 5) Criar um FunctionCall que aponte para a função monomorfizada
    std::string instantiatedName = concreteFunc->getName();
    std::cerr << "TemplateCall: creating FunctionCall for instantiated function: " << instantiatedName << std::endl;
    FunctionCall *concreteCall = new FunctionCall(instantiatedName, pc, getLoc());

    // Preservar escopo e leftValue (se aplicável)
    //concreteCall->setScope(getScope());
    if (leftValue)
        concreteCall->setLeftValue(leftValue);

    // 6) Delegar geração (a FunctionCall fará a geração de código usando o FunctionImpl concreto)
    Value *val = concreteCall->generate(func, block, allocblock);

    // 7) Propagar tipo de retorno (opcional) - caso queira propagar aqui
    //    se concreteFunc tiver tipo definido, podemos ajustar este TemplateCall/type info
    //    (o fluxo normal de PropagateTypes geralmente cuidará disso)
    // PropagateTypes propag;
    // propag.propagateChildren(*concreteCall);

    return val;
}

Node *TemplateCall::accept(Visitor &v)
{
    return v.visit(*this);
}*/

#include "TemplateCall.h"
#include "FunctionCall.h"
#include "BuildTypes.h"
#include "FunctionDecl.h"
#include "FunctionImpl.h"
#include "TemplateImpl.h"
#include "HeaderGlobals.h"
#include "Load.h"
#include "BackLLVM.h"
#include "UserType.h"
#include "Program.h"
#include "Interface.h"
#include "semantic/PropagateTypes.h"
#include "semantic/Visitor.h"
#include "ParamsCall.h"

extern Program *program;
extern std::unique_ptr<BuildTypes> buildTypes;

// O corpo da função é removido e substituído por uma validação.
// A lógica de expansão agora está inteiramente no ExpandTemplates::visit(TemplateCall &n).
Value *TemplateCall::generate(FunctionImpl *func,
                              BasicBlock *block,
                              BasicBlock *allocblock)
{
    // Este código garante que o nó TemplateCall foi substituído por FunctionCall
    // na fase de Template Expansion Pass, evitando a repetição da lógica e garantindo
    // que a AST correta seja gerada.
    yyerrorcpp("TemplateCall node should have been expanded to FunctionCall before code generation.", this);

    return nullptr;
}

Node *TemplateCall::accept(Visitor &v)
{
    return v.visit(*this);
}
