#include "TemplateImpl.h"
#include "Program.h"
#include "FunctionImpl.h"
#include "BuildTypes.h"
#include "TemplateParamNode.h"
#include "semantic/PropagateTypes.h"
#include "semantic/Visitor.h"
#include "semantic/ExpandTemplates.h"
#include <memory>
#include <sstream>
#include <algorithm>
#include <cctype>

extern std::unique_ptr<BuildTypes> buildTypes;

Node *TemplateImpl::accept(Visitor &v)
{
    return v.visit(*this);
}

static std::string encodeTypeString(const std::string &typeName)
{
    std::string t = typeName;
    t.erase(std::remove_if(t.begin(), t.end(), ::isspace), t.end());

    if (t == "int" || t == "int32" || t == "i32")
        return "i";
    if (t == "int8" || t == "int8_t" || t == "i8")
        return "a"; // escolha arbitrária
    if (t == "int16" || t == "i16")
        return "s";
    if (t == "int64" || t == "i64" || t == "long")
        return "l";
    if (t == "float" || t == "f32")
        return "f";
    if (t == "double" || t == "f64")
        return "d";
    if (t == "char")
        return "c";
    if (t == "void" || t == "void_t")
        return "v";
    if (t == "bool")
        return "b";

    // tipos compostos: substituir caracteres problemáticos e usar comprimento prefixado
    std::string safe;
    for (char ch : t)
    {
        if (std::isalnum((unsigned char)ch))
            safe.push_back(ch);
        else
            safe.push_back('_');
    }
    std::ostringstream os;
    os << safe.size() << safe;
    return os.str();
}

string TemplateImpl::mangleName(string baseName, string returnType, vector<string> &params)
{
    // Itanium-style simplified (determinístico)
    std::ostringstream os;
    os << "_Z" << baseName.size() << baseName;
    // iniciar argumento de template
    os << "I";
    for (size_t i = 0; i < params.size(); ++i)
    {
        os << encodeTypeString(params[i]);
    }
    os << "E";
    // opcional: anexar retorno (pode não ser necessário, mas evita colisões se sobrecarga por retorno existir)
    if (!returnType.empty())
    {
        os << "R" << encodeTypeString(returnType);
    }
    return os.str();
}

Node *TemplateImpl::generateFor(const vector<string> &concreteTypes)
{

    std::cout << "[TemplateImpl] Gerando instância para: " << this->getName() << std::endl;
    for (const auto &t : concreteTypes)
    {
        std::cout << "  - Tipo concreto: " << t << std::endl;
    }

    // 1) validação
    if (params.size() != concreteTypes.size())
    {
        yyerrorcpp("Template instantiation failed: mismatched params", this);
        return nullptr;
    }

    // 2) preparar mapa de substituição (nome do parâmetro de template -> DataType)
    std::map<std::string, DataType> substitutionMap;
    for (size_t i = 0; i < params.size(); ++i)
    {
        TemplateParamNode *tpn = dynamic_cast<TemplateParamNode *>(params[i]);
        if (!tpn)
        {
            yyerrorcpp("Template parameter node expected.", this);
            return nullptr;
        }
        std::string paramName = tpn->getName();
        DataType dt = buildTypes->getType(concreteTypes[i]);
        if (dt == BuildTypes::undefinedType)
        {
            yyerrorcpp("Unknown type '" + concreteTypes[i] + "' in template instantiation.", this);
            return nullptr;
        }
        substitutionMap[paramName] = dt;
    }

    // 3) determinar tipo de retorno concreto (se o retorno for parâmetro, substitui)
    DataType returnType = buildTypes->getType(templ_dt);
    if (returnType == BuildTypes::undefinedType)
    {
        // tentar encontrar em substitutionMap (templ_dt pode ser "T")
        auto it = substitutionMap.find(templ_dt);
        if (it != substitutionMap.end())
        {
            returnType = it->second;
        }
        else
        {
            yyerrorcpp("Unknown return type '" + templ_dt + "' in template instantiation.", this);
            return nullptr;
        }
    }

    // 4) gerar nome mangleado (string que identifica a instância)
    vector<string> paramsForMangle = concreteTypes;
    string instantiatedName = mangleName(this->getName(), buildTypes->name(returnType), paramsForMangle);

    // 5) Se já existe instância (cache) -> retornar (OBS: TemplateCall já verifica em program,
    //    mas é seguro verificar localmente também; aqui não fazemos lookup global para evitar
    //    dependência de API específica — quem chamou já verifica normalmente.)
    //    Se você preferir, mantenha um mapa local instantiations[mangled] = fn
    //    IMPLEMENTAÇÃO OPCIONAL:
    // if (instantiations.count(instantiatedName)) return instantiations[instantiatedName];

// Acessamos os parâmetros originais através de getParameters() da FunctionBase.
    const std::vector<Variable*>& originalParameters = this->getParameters().getParameters();

    // Step 6: Construção dos parâmetros concretos.
    FunctionParams *newFp = new FunctionParams();
    
    // Itera sobre o vetor de parâmetros originais (T u, T v)
    for (Variable *origVar : originalParameters)
    {
        // CORREÇÃO: Usa buildTypes->name() para obter o nome do tipo (ex: "T")
        std::string paramTypeName = buildTypes->name(origVar->getDataType()); 

        DataType concreteParamType = origVar->getDataType();
        
        // Se o tipo do parâmetro é genérico e precisa ser substituído (i.e., está no mapa)
        if (substitutionMap.count(paramTypeName))
        {
            // Substitui o tipo genérico pelo tipo concreto (ex: int8)
            concreteParamType = substitutionMap[paramTypeName];
        }
        
        // CRIAÇÃO DO NOVO NÓ VARIABLE (PARÂMETRO) COM O TIPO CONCRETO
        Variable *newVar = new Variable(origVar->getIdent().getFullName(),
                                        concreteParamType,
                                        origVar->getLoc());
        
        // Copia flags importantes (ex: isReference), se aplicável
        //newVar->setReference(origVar->isReference()); 
        
        newFp->append(newVar);
    }

    // 7) Construir vector<Node*> vazio para o corpo; popularemos usando ExpandTemplates
    vector<Node *> newBody;

    // 8) Construir a nova FunctionImpl (obs.: verifique assinatura do seu constructor)
    // ctor: FunctionImpl(DataType dt, string name, FunctionParams *fp, vector<Node*> &&stmts, location_t loc, location_t ef, bool constructor = false)
    // Aqui usamos this->loc como localização; se TemplateImpl armazenar endfunction, adapte.
    location_t loc = this->sloc; // ADAPTAR se o campo tiver outro nome
    location_t ef = this->sloc;  // ADAPTAR: se existir um endfunction, use-o

    FunctionImpl *newFunc = new FunctionImpl(returnType, instantiatedName, newFp, std::move(newBody), loc, ef, this->constructor);

    program->addChild(newFunc);
    program->addSymbol(newFunc);

    std::cout << "[TemplateImpl] Função instanciada registrada: " << instantiatedName << std::endl;

    std::unique_ptr<std::map<std::string, DataType>> subMapPtr = std::make_unique<std::map<std::string, DataType>>(substitutionMap);
    ExpandTemplates expander;
    expander.setSubstitutionMap(&substitutionMap);

    for (Node *origChild : this->node_children)
    {
        std::cout << "[TemplateImpl] Expandindo nó do corpo do template..." << std::endl;
        std::cout << "  - Tipo do nó: " << typeid(*origChild).name() << std::endl;

        DataType dt = origChild->getDataType();
        std::string dtName = buildTypes->name(dt);
        std::cout << "  - Tipo do nó antes da expansão: " << dtName << std::endl;

        Node *expanded = nullptr;
        try
        {
            expanded = origChild->accept(expander);
        }
        catch (...)
        {
            std::cout << "[TemplateImpl] Exceção ao expandir o nó." << std::endl;
            expanded = nullptr;
        }

        if (expanded)
        {
            std::cout << "[TemplateImpl] Nó expandido com sucesso." << std::endl;
            expanded->setScope(newFunc);
            newFunc->addChild(expanded);
        }
        else
        {
            std::cout << "[TemplateImpl] Falha ao expandir nó do tipo: " << typeid(*origChild).name() << std::endl;

            // CORREÇÃO CRÍTICA: Se a expansão não retornou um novo nó,
            // devemos usar o nó original. Mas, para evitar a asserção,
            // precisamos que o ExpandTemplates garanta a criação de novos Statements (como Return).
            // Se ele falhou e estamos aqui, é porque o nó original não foi substituído.
            // A melhor prática seria clonar, mas como é proibido, emitimos o warning
            // e dependemos da correção em ExpandTemplates para Return/Variable.
            yywarncpp("ExpandTemplates did not return a replacement for a node in template '" + this->getName() + "'. Manual clone may be required.", this);

            // Aqui, por precaução, vamos adicionar o original se a asserção de escopo for ignorada
            // ou se o nó for um nó que não possui escopo. NO SEU CASO, é melhor
            // não adicionar nada se ExpandTemplates falhou, confiando que Statements críticos foram criados.
        }
    }

    return newFunc;
}

Value *TemplateImpl::generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock)
{
    yywarncpp("Attempt to generate code for a template definition '" + this->getName() + "'. Skipping.", this);
    return nullptr;
}
