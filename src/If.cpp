
#include "If.h"
#include "FunctionImpl.h"
#include "semantic/Visitor.h"

If::If(Node *e, vector<Node*> &&tst, location_t loc)
: Cloneable<If>(loc), expr(e)
{
    addChild(expr);

    thenst = new Node(std::move(tst), loc);
    addChild(thenst);

    elsest = nullptr;
}

If::If(Node *e, vector<Node*> &&tst, vector<Node*> &&est, location_t loc)
: If(e, std::move(tst), loc)
{
    elsest = new Node(std::move(est), loc);
    addChild(elsest);
}

If::If(const If& other, TypeSubs& ts)
    : Cloneable<If>(other, ts)  // importante!
{
    // ---- 1) Clone expr ----
    if (other.expr)
        expr = other.expr->cloneTree(ts);
    else
        expr = nullptr;

    if (expr)
        addChild(expr);

    // ---- 2) Clone thenst ----
    if (other.thenst)
    {
        // thenst é um Node que contém vários statements dentro
        vector<Node*> newThen;

        for (Node* st : other.thenst->children())
        {
            Node* cloned = st->cloneTree(ts);
            newThen.push_back(cloned);
        }

        thenst = new Node(std::move(newThen), other.getLoc());
        addChild(thenst);
    }
    else
        thenst = nullptr;

    // ---- 3) Clone elsest (se existir) ----
    if (other.elsest)
    {
        vector<Node*> newElse;

        for (Node* st : other.elsest->children())
        {
            Node* cloned = st->cloneTree(ts);
            newElse.push_back(cloned);
        }

        elsest = new Node(std::move(newElse), other.getLoc());
        addChild(elsest);
    }
    else
        elsest = nullptr;
}

Value *If::generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) {

	Value *exprv = expr->generate(func, block, allocblock);
	if (!exprv) {
		// This occurs when loading a var still not defined, so
		// we let it be False to recovery from the error
		exprv = ConstantInt::get(Type::getInt1Ty(global_context), 0);
	}
	
	BasicBlock *thenb = BasicBlock::Create(global_context, "if_then", 
		func->getLLVMFunction(), 0);
	Value *thennewb = thenst->generateChildren(func, thenb, allocblock);

	Value *elsenewb = NULL;
	BasicBlock *elseb = BasicBlock::Create(global_context, "if_else", 
		func->getLLVMFunction(), 0);
	if (elsest != 0) {
		elsenewb = elsest->generateChildren(func, elseb, allocblock);
	}

	BasicBlock *newblock = block;
	// A distinct block can return from boolean short-circuit evaluation
	Instruction* instr = dyn_cast<Instruction>(exprv);
	if (instr)
		newblock = instr->getParent();
	BranchInst::Create(thenb, elseb, exprv, newblock);

	BasicBlock *mergb = BasicBlock::Create(global_context, "if_cont", 
		func->getLLVMFunction(), 0);
		
	// identify last then block
	BasicBlock *lastthen = thenb;
	if (thennewb && thennewb->getValueID() == Value::BasicBlockVal)
		lastthen = (BasicBlock*)thennewb;

	// if then block already has a terminator, don't generate branch
	if (!lastthen->getTerminator())
		BranchInst::Create(mergb, lastthen); 
		
	// identify last else block
	BasicBlock *lastelse = elseb;
	if (elsenewb && elsenewb->getValueID() == Value::BasicBlockVal) 
		lastelse = (BasicBlock*)elsenewb;

	// if else block already has a terminator, don't generate branch
	if (!lastelse->getTerminator())
		BranchInst::Create(mergb, lastelse); 

	return mergb;
}

Node* If::accept(Visitor& v) {
	return v.visit(*this);
} 
