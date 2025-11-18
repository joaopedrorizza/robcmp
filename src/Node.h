
#pragma once

#include "SourceLocation.h"
#include "BuildTypes.h"

class Visitor;
class NamedNode;
class UserType;
class FunctionImpl;
class Variable;

class Node : public SourceLocation {
protected:
	map<string, NamedNode*> symbols;
	vector<Node*> node_children;
	DataType dt = BuildTypes::undefinedType;
	set<DataQualifier> qualifiers;
	Node *scope = nullptr;
	bool pointerToPointer = false;

public:
	Node(location_t l) : SourceLocation(l) {}
	Node(vector<Node*> &&children, location_t l);

	Node(const Node& o) :
		SourceLocation(o.getLoc()),
		qualifiers(o.qualifiers),
		pointerToPointer(o.pointerToPointer) {}

	virtual ~Node();

	virtual bool isConstExpr() {
		return false;
	}
	
	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock);
	
	virtual Value *generateChildren(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock);

	virtual std::vector<Node *> const& children() const;
	
	virtual Node* accept(Visitor &);

	virtual bool hasName() {
		return false;
	}

	virtual const string getName() const {
		return "";
	}

	Node *getScope() {
		return scope;
	}

	virtual void setScope(Node *s, bool replace = false);

	virtual Node* findSymbol(const string& name, bool recursive = true);
	
	virtual Node* findMember(const string& name);

	void addChild(Node *n, bool prepend = false);
	void removeChild(Node *n);

	virtual map<string, NamedNode*> const& getSymbols();

	virtual void addSymbol(const string& label, NamedNode *nm);
	virtual void addSymbol(NamedNode *nm);
	
	void setDataType(DataType dt) {
		this->dt = dt;
	}

	virtual DataType getDataType() {
		return dt;
	}

	virtual Type *getLLVMType();

	void setQualifier(DataQualifier dq) {
		qualifiers.insert(dq);
	}

	bool hasQualifier(DataQualifier dq) const {
		return qualifiers.count(dq) == 1;
	}

	virtual Value* getLLVMValue(Node *stem, FunctionImpl *gfunc = NULL) {
		return NULL;
	}

	virtual void setLeftValue(Variable *symbol) {}

	bool isPointerToPointer() {
		return pointerToPointer;
	}

	void setPointerToPointer(bool v) {
		pointerToPointer = v;
	}

	virtual Node* cloneShallow() const {
		return NULL;
	}
	
	Node* cloneTree() const {
        Node* n = cloneShallow();
		if (!n) {
			std::string name = typeid(*this).name();
			SourceLocation auxloc(*this);
			yyerrorcpp(string_format("You need to implement cloneShallow using Cloneable<> in the node type %s.", name.c_str()), &auxloc, true);
			assert(n);
		}
        n->node_children.reserve(node_children.size());
        for (Node* c : node_children)
            n->node_children.push_back(c->cloneTree());
        return n;
    }

	friend class UserType;
	friend class Program;
	friend class MemCopy;
	friend class PropagateTypes;
	friend class IdentifyVirtualDispatch;
	friend class ExpandTemplates;
};

template<typename Derived, typename Base = Node>
struct Cloneable : Base {
	using Base::Base;
    Base* cloneShallow() const override {
        return new Derived(static_cast<const Derived&>(*this));
    }
};

class NamedNode: public Node {
protected:
	string name;

public:
	NamedNode(const string &name, location_t loc) : Node(loc), name(name) {}

	NamedNode(const string &name, vector<Node*> &&children, location_t loc) :
		Node(std::move(children), loc), name(name) {}

	NamedNode(const NamedNode& n) : Node(n), name(n.name) {}

	virtual const string getName() const override {
		return name;
	}

	void setName(const string& nname) {
		name = nname;
	}

	bool hasName() override {
		return true;
	}
};

extern Node* getNodeForIntConst(int64_t, location_t);
extern Node* getNodeForUIntConst(uint64_t, location_t);
