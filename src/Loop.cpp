
#include "Loop.h"
#include "FunctionImpl.h"

Loop::Loop(vector<Node*> &&stms, location_t loc) : Cloneable<Loop>(std::move(stms), loc) {}

Loop::Loop(const Loop& other, TypeSubs& ts)
    : Cloneable<Loop>(other, ts) 
{
    for (Node* child : other.children()) {
        if (child) {
            Node* cloned = child->cloneTree(ts);
            this->addChild(cloned);
        }
    }
}

Value *Loop::generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) {
	BasicBlock *bodyloop = BasicBlock::Create(global_context, "loop_body", 
		func->getLLVMFunction(), 0);
	
	RobDbgInfo.emitLocation(this);

	// go to loop
	Builder->SetInsertPoint(block);
	Builder->CreateBr(bodyloop);

	// alloc instructions inside bodyloop should go to allocblock to prevent repeatedly allocation
	Value *newb = generateChildren(func, bodyloop, allocblock); 

	// identify last while body block
	BasicBlock *endbody = bodyloop;
	if (newb->getValueID() == Value::BasicBlockVal)
		endbody = (BasicBlock*)newb;
	
	// if end block already has a terminator, don't generate branch
	if (!((BasicBlock*)endbody)->getTerminator()) {
		Builder->SetInsertPoint(endbody);
		Builder->CreateBr(bodyloop);
	}

	BasicBlock *endloop = BasicBlock::Create(global_context, "loop_end", 
		func->getLLVMFunction(), 0);
	return endloop;
}
