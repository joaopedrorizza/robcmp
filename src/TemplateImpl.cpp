#include "TemplateImpl.h"
#include "Program.h"
#include "FunctionImpl.h"
#include "SourceLocation.h"
#include "BuildTypes.h"
#include "Return.h"
#include "Variable.h"
#include "Scalar.h"
#include "TemplateParamNode.h"
#include "semantic/PropagateTypes.h"
#include "semantic/SymbolizeTree.h"
#include "semantic/Visitor.h"
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

string TemplateImpl::mangleName(string baseName, string returnType_, vector<string> &params)
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
    if (!returnType_.empty())
    {
        os << "R" << encodeTypeString(returnType_);
    }
    return os.str();
}


static Node* cloneExprWithSubs(Node* e,
                               const std::map<std::string, DataType>& subMap,
                               Node* newScope)
{
    if (!e) return nullptr;

    // 1) Variable
    if (auto* v = dynamic_cast<Variable*>(e)) {
        auto* nv = new Variable(v->getName(), v->getDataType(), v->getLoc());
        nv->setScope(nullptr,true);
        if (auto it = subMap.find(v->getDataTypeName()); it != subMap.end())
            nv->setDataType(it->second);
        return nv;
    }

    // 2) Scalar
    if (auto* s = dynamic_cast<Scalar*>(e)) {
        Node* rhs = cloneExprWithSubs(s->getExpr(), subMap, newScope);
        auto* ns = new Scalar(s->getName(), rhs);
        ns->setScope(nullptr, true);
        if (auto it = subMap.find(s->getDataTypeName()); it != subMap.end())
            ns->setDataType(it->second);
        else
            ns->setDataType(s->getDataType());
        return ns;
    }

    // 3) Return (em expressão é raro; por segurança)
    if (auto* r = dynamic_cast<Return*>(e)) {
        Node* val = cloneExprWithSubs(r->value(), subMap, newScope);
        Return* nr = val ? new Return(val) : new Return(r->getLoc());
        nr->setScope(nullptr,true);
        if (val) {
            if (auto it = subMap.find(buildTypes->name(val->getDataType())); it != subMap.end()) {
                val->setDataType(it->second);
                nr->setDataType(it->second);
            } else {
                nr->setDataType(val->getDataType());
            }
        } else {
            nr->setDataType(tvoid);
        }
        return nr;
    }

    // 4) FunctionCall
    if (auto* fc = dynamic_cast<FunctionCall*>(e)) {
        auto* newParams = new ParamsCall();
        for (auto* p : fc->getParameters()) {
            Node* pc = cloneExprWithSubs(p, subMap, newScope);
            if (!pc) pc = p; // último recurso: mantém, mas sem mexer no scope
            newParams->append(pc);
        }
        auto* nfc = new FunctionCall(fc->getName(), newParams, fc->getLoc());
        nfc->setScope(nullptr,true);
        // opcional: substituir tipo de retorno se for genérico
        if (auto it = subMap.find(buildTypes->name(nfc->getDataType())); it != subMap.end())
            nfc->setDataType(it->second);
        else
            nfc->setDataType(fc->getDataType());
        return nfc;
    }

    // 5) TemplateCall — NÃO reusar alterando scope (pode já ter scope)
    if (auto* tc = dynamic_cast<TemplateCall*>(e)) {
        // Melhor: crie um "clone" de TemplateCall se você tiver construtores públicos.
        // Se não for possível agora, devolva o próprio ponteiro SEM mexer no scope:
        return tc; // cuidado: não chamar setScope(tc, ...) aqui!
    }

    // 6) Literais/Outros tipos tipados? (adicione casos conforme existirem)

    // 7) Fallback: por segurança, NÃO reusar e setar scope (evita o assert).
    // Em vez disso, retorne nullptr e trate onde for usado.
    return e;
}


FunctionImpl *TemplateImpl::generateFor(const vector<string> &concreteTypes, SourceLocation& loc)
{

    FunctionImpl *newFunc;

    // 1) validação
    if (params.size() != concreteTypes.size())
    {
        yyerrorcpp("Template instantiation failed: mismatched params", &loc);
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

    // 3) determinar tipo de retorno concreto (se o retorno for T, substitui)
    DataType newRt;
    auto resp = substitutionMap.find(returnType);
    if (resp == substitutionMap.end()) {
        newRt = buildTypes->getType(returnType);
    } else {
        newRt = resp->second;
    }

    // 4) gerar nome mangleado (string que identifica a instância)
    vector<string> paramsForMangle = concreteTypes;
    string instantiatedName = mangleName(this->getName(), buildTypes->name(newRt), paramsForMangle);

    //FunctionParams *newFp = new FunctionParams();
    if (Node *exists = program->findSymbol(instantiatedName)) {
        auto found = dynamic_cast<FunctionImpl*>(exists);
        return found; // já instanciado → reuse
    }

    // Cria um novo conjunto de parâmetros para a instância
    FunctionParams *newParams = new FunctionParams();

    for (Variable *var : parameters->getParameters()) {
        // Descobre o tipo concreto
        auto it = substitutionMap.find(var->getDataTypeName());
        if (it != substitutionMap.end()) {
            Variable *newVar = new Variable(var->getName(), it->second, var->getLoc());
            newParams->append(newVar);
        }
    }

    vector<Node*> newBody;
    for(Node *children : this->node_children) {
        newBody.push_back(children->cloneTree());
    }

    // 8) Construir a nova FunctionImpl (obs.: verifique assinatura do seu constructor)
    newFunc = new FunctionImpl(newRt, instantiatedName, newParams, std::move(newBody), this->getLoc(), this->getLoc(), this->constructor);
    SymbolizeTree st;
    newFunc->accept(st);

    return newFunc;
}

Value *TemplateImpl::generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock)
{
    // Templates aren't generated.
    return nullptr;
}
