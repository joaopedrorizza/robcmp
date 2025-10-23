#include "TemplateImpl.h"
#include "Program.h"
#include "FunctionImpl.h"
#include "SourceLocation.h"
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

// Retorna 'original' se não houver substituição.
// Funciona com DataType = int; usa subMap<string, DataType>.
static DataType substituteType(DataType original,
                               const std::map<std::string, DataType> &subMap)
{
    // 0) Se for indefinido, não tente nomear nem substituir
    if (original == BuildTypes::undefinedType)
        return original;

    // 1) Se é parâmetro de template (ex.: T, U, ...), pegue o NOME textual
    if (buildTypes->isTemplateType(original))
    {
        // Se seu BuildTypes CONSEGUE nomear esse DataType seguro, use:
        // (Se name() assertar para placeholders, comente este bloco e use o "fallback" logo abaixo)
        const char *maybeName = nullptr;
        // Tente obter o nome textual com segurança — se o seu name() não é seguro, pule.
        // Aqui assumo que para "tipos de template" você consegue nomear.
        maybeName = buildTypes->name(original);

        if (maybeName && maybeName[0] != '\0')
        {
            auto it = subMap.find(std::string(maybeName));
            if (it != subMap.end())
                return it->second; // T -> int8
        }

        // Fallback: se não deu pra obter nome, não substitui.
        return original;
    }

    // 2) Se for array/matriz de algo genérico, substitua o elemento
    if (buildTypes->isArrayOrMatrix(original))
    {
        DataType inner = buildTypes->getArrayElementType(original);
        DataType newInner = substituteType(inner, subMap);
        if (newInner != inner && newInner != BuildTypes::undefinedType)
        {
            unsigned char dims = buildTypes->dimensions(original);
            SourceLocation sloca = *(buildTypes->location(original));
            // Obter o nome do tipo concreto do elemento
            const char *innerName = buildTypes->name(newInner); // aqui newInner é concreto
            return buildTypes->getArrayType(innerName, sloca, dims, false);
        }
        return original;
    }

    // 3) Caso contrário, mantém
    return original;
}

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
    DataType returnType;

    if (templ_dt.empty())
    {
        returnType = buildTypes->getType("void");
    }
    else
    {
        returnType = buildTypes->getType(templ_dt);
        if (returnType == BuildTypes::undefinedType)
        {
            auto it = substitutionMap.find(templ_dt);
            if (it != substitutionMap.end())
            {
                returnType = it->second;
            }
            else if (templ_dt == "void")
            {
                returnType = buildTypes->getType("void");
            }
            else
            {
                yyerrorcpp("Unknown return type '" + templ_dt + "' in template instantiation.", this);
                return nullptr;
            }
        }
    }

    // 4) gerar nome mangleado (string que identifica a instância)
    vector<string> paramsForMangle = concreteTypes;
    string instantiatedName = mangleName(this->getName(), buildTypes->name(returnType), paramsForMangle);

    if (Node *exists = program->findSymbol(instantiatedName))
        return exists; // já instanciado → reuse

    // 5) Se já existe instância (cache) -> retornar (OBS: TemplateCall já verifica em program,
    //    mas é seguro verificar localmente também; aqui não fazemos lookup global para evitar
    //    dependência de API específica — quem chamou já verifica normalmente.)
    //    Se você preferir, mantenha um mapa local instantiations[mangled] = fn
    //    IMPLEMENTAÇÃO OPCIONAL:
    // if (instantiations.count(instantiatedName)) return instantiations[instantiatedName];

    // Acessamos os parâmetros originais através de getParameters() da FunctionBase.
    const std::vector<Variable *> &originalParameters = this->getParameters().getParameters();
    
    // Step 6: Construção dos parâmetros concretos com substituição real
    FunctionParams *newFp = new FunctionParams();

    for (Variable *origVar : originalParameters)
    {
        DataType origTy = origVar->getDataType();
        DataType concreteParamType = origTy;

        // Busca textual aproximada do nome do tipo original (T, U, etc.)
        std::string origTypeName = buildTypes->name(origTy);

        // Verifica se é um tipo genérico e tenta resolver via substitutionMap
        if (buildTypes->isTemplateType(origTy) || substitutionMap.count(origTypeName))
        {
            auto it = substitutionMap.find(origTypeName);
            if (it != substitutionMap.end())
            {
                concreteParamType = it->second;
                std::cout << "[DEBUG] Substituindo tipo de param "
                          << origVar->getIdent().getFullName()
                          << ": " << origTypeName
                          << " → " << buildTypes->name(concreteParamType)
                          << " (tid=" << concreteParamType << ")\n";
            }
            else if (substitutionMap.size() == 1)
            {
                // fallback seguro pra templates com um único parâmetro
                concreteParamType = substitutionMap.begin()->second;
                std::cout << "[DEBUG] Fallback: substituindo tipo genérico "
                          << origTypeName << " → "
                          << buildTypes->name(concreteParamType)
                          << " (tid=" << concreteParamType << ")\n";
            }
            else
            {
                yywarncpp("Não foi possível resolver tipo genérico '" + origTypeName + "'.", this);
            }
        }

        // Cria a variável concreta
        Variable *newVar = new Variable(origVar->getIdent().getFullName(),
                                        concreteParamType,
                                        origVar->getLoc());

        newFp->append(newVar);
    }

    // 7) Construir vector<Node*> vazio para o corpo; popularemos usando ExpandTemplates
    vector<Node *> newBody;

       // 8) Construir a nova FunctionImpl (obs.: verifique assinatura do seu constructor)
    location_t loc = this->sloc;
    location_t ef = this->sloc;

    FunctionImpl *newFunc = new FunctionImpl(returnType, instantiatedName, newFp, std::move(newBody), loc, ef, this->constructor);
    if (returnType != BuildTypes::undefinedType)
        newFunc->setDataType(returnType);
    else
        newFunc->setDataType(tvoid);

    // ==============================================================
    // 🧩 DEBUG: LOG COMPLETO - ANTES DA EXPANSÃO
    // ==============================================================

    std::cout << "\n\n========== [DEBUG: BEFORE TEMPLATE EXPANSION] ==========\n";
    std::cout << "Template base: " << this->getName() << std::endl;
    std::cout << "Instância gerada: " << instantiatedName << std::endl;

    std::cout << "\n-- Substitution Map --\n";
    for (auto &pair : substitutionMap)
        std::cout << "  " << pair.first << " -> " << buildTypes->name(pair.second)
                  << " (tid=" << pair.second << ")\n";

    std::cout << "\n-- Tipo de Retorno --\n";
    std::cout << "  templ_dt = '" << templ_dt << "'\n";
    std::cout << "  returnType = " << buildTypes->name(returnType)
              << " (tid=" << returnType << ")\n";

    std::cout << "\n-- Parâmetros Originais do Template --\n";
    for (Variable *origVar : this->getParameters().getParameters())
        std::cout << "  " << origVar->getIdent().getFullName() << " : "
                  << buildTypes->name(origVar->getDataType())
                  << " (tid=" << origVar->getDataType() << ")\n";

    std::cout << "\n-- Parâmetros Substituídos (newFunc) --\n";
    for (Variable *nv : newFunc->getParameters().getParameters())
        std::cout << "  " << nv->getIdent().getFullName() << " : "
                  << buildTypes->name(nv->getDataType())
                  << " (tid=" << nv->getDataType() << ")\n";

    std::cout << "\n-- Corpo Original do Template (this->node_children) --\n";
    if (this->node_children.empty())
        std::cout << "  (sem nós)\n";
    else
        for (Node *origChild : this->node_children)
            std::cout << "  Nó: " << typeid(*origChild).name()
                      << " @ " << (void *)origChild
                      << " tipo=" << buildTypes->name(origChild->getDataType())
                      << " (tid=" << origChild->getDataType() << ")\n";
    std::cout << "=========================================================\n\n";


    // ==============================================================
    // ⚙️ EXPANSÃO DO CORPO
    // ==============================================================

    ExpandTemplates expander(&substitutionMap);
    for (Node *origChild : this->node_children)
    {
        Node *expanded = nullptr;
        try
        {
            expanded = origChild ? origChild->accept(expander) : nullptr;
        }
        catch (...)
        {
            expanded = nullptr;
        }

        if (expanded)
        {
            expanded->setScope(newFunc);
            newFunc->addChild(expanded);
        }
        else
        {
            std::cout << "[TemplateImpl] Falha ao expandir nó: "
                      << (origChild ? typeid(*origChild).name() : "<null>") << std::endl;

            // fallback simples
            if (auto *ret = dynamic_cast<Return *>(origChild))
            {
                Node *newExpr = nullptr;
                if (!ret->children().empty() && ret->children()[0])
                {
                    Node *exp = ret->children()[0]->accept(expander);
                    newExpr = exp ? exp : ret->children()[0];
                }
                Node *nr = newExpr ? static_cast<Node *>(new Return(newExpr))
                                   : static_cast<Node *>(new Return(ret->getLoc()));
                nr->setScope(newFunc);
                newFunc->addChild(nr);
            }
        }
    }

    // ==============================================================
    // 🧠 DEBUG: LOG COMPLETO - DEPOIS DA EXPANSÃO
    // ==============================================================

    std::cout << "\n\n========== [DEBUG: AFTER TEMPLATE EXPANSION] ==========\n";
    std::cout << "Função instanciada: " << instantiatedName << std::endl;
    std::cout << "Tipo de retorno final: " << buildTypes->name(newFunc->getDataType())
              << " (tid=" << newFunc->getDataType() << ")\n";

    std::cout << "\n-- Parâmetros Finais --\n";
    for (Variable *nv : newFunc->getParameters().getParameters())
        std::cout << "  " << nv->getIdent().getFullName() << " : "
                  << buildTypes->name(nv->getDataType())
                  << " (tid=" << nv->getDataType() << ")\n";

    std::cout << "\n-- Corpo Expandido (newFunc->node_children) --\n";
    if (newFunc->children().empty())
        std::cout << "  (sem nós gerados)\n";
    else
        for (Node *child : newFunc->children())
            std::cout << "  Nó expandido: " << typeid(*child).name()
                      << " @ " << (void *)child
                      << " tipo=" << buildTypes->name(child->getDataType())
                      << " (tid=" << child->getDataType() << ")\n";

    std::cout << "=========================================================\n\n";


    // ==============================================================
    // 🔚 Registro da nova função
    // ==============================================================

    program->addChild(newFunc);
    program->addSymbol(newFunc);

    std::cout << "[TemplateImpl] Função instanciada registrada: " << instantiatedName << std::endl;

    return newFunc;

}

Value *TemplateImpl::generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock)
{
    yywarncpp("Attempt to generate code for a template definition '" + this->getName() + "'. Skipping.", this);
    return nullptr;
}
