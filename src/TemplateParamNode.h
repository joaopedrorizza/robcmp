#pragma once

#include "Node.h"
#include "BuildTypes.h"

/**
 * @brief Enumeração para diferenciar o tipo de parâmetro do template.
 */
enum TemplateParamKind {
    TP_TYPE,      // Parâmetro de Tipo (e.g., 'typename T', 'class U')
    TP_NON_TYPE,  // Parâmetro de Não-Tipo (e.g., 'int N', 'size_t M')
    // TP_TEMPLATE, // Parâmetros de Template para templates aninhados (mais avançado)
};

class TemplateParamNode : public Node {
public:
    std::string name;
    TemplateParamKind kind;

    // Apenas para TP_NON_TYPE: Armazena o tipo concreto do parâmetro de valor (int, size_t, etc.)
    // Para TP_TYPE, esta DataType pode ser nula ou um tipo placeholder.
    DataType nonTypeParamType; 

    // Construtor para Parâmetros de Tipo (typename T, class U)
    TemplateParamNode(TemplateParamKind k, const std::string &n, location_t loc)
        : Node(loc), name(n), kind(k), nonTypeParamType(BuildTypes::undefinedType) {}

    // Construtor para Parâmetros de Não-Tipo (int N)
    TemplateParamNode(TemplateParamKind k, const std::string &n, DataType dt, location_t loc)
        : Node(loc), name(n), kind(k), nonTypeParamType(dt) {}

   const std::string getName() const override;
    virtual Node* accept(Visitor &v) override;
};
