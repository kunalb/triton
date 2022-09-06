#include "triton/Analysis/Alias.h"
#include "triton/Analysis/Utility.h"
#include "triton/Dialect/TritonGPU/IR/Dialect.h"

namespace mlir {

AliasInfo AliasInfo::join(const AliasInfo &lhs, const AliasInfo &rhs) {
  if (lhs == rhs)
    return lhs;
  AliasInfo ret;
  for (auto value : lhs.allocs) {
    ret.insert(value);
  }
  for (auto value : rhs.allocs) {
    ret.insert(value);
  }
  return ret;
}

ChangeResult SharedMemoryAliasAnalysis::visitOperation(
    Operation *op, ArrayRef<LatticeElement<AliasInfo> *> operands) {
  AliasInfo aliasInfo;
  bool pessimistic = true;
  if (maybeSharedAllocationOp(op)) {
    // These ops will allocate a new shared memory buffer.
    auto result = op->getResult(0);
    if (isSharedEncoding(result)) {
      aliasInfo.insert(result);
      pessimistic = false;
    }
  } else {
    llvm::errs() << "op: " << op->getName() << "\n";
  }
  // XXX(Keren): triton ops don't support aliasing yet.
  // else if (auto viewOp = dyn_cast<triton::ViewOp>(op) ||
  //                         dyn_cast<triton::ExpandDimsOp>(op)) {
  //  // These ops will reate a new view of the same shared memory buffer.
  //  auto result = op->getResult(0);
  //  if (isSharedEncoding(result)) {
  //    aliasInfo = AliasInfo(operands[0]->getValue());
  //    pessimistic = false;
  //  }
  //}
  if (pessimistic) {
    return markAllPessimisticFixpoint(op->getResults());
  }
  // Join all latice elements
  ChangeResult result = ChangeResult::NoChange;
  for (Value value : op->getResults()) {
    result |= getLatticeElement(value).join(aliasInfo);
  }
  return result;
}

AliasResult SharedMemoryAliasAnalysis::alias(Value lhs, Value rhs) {
  // TODO: implement
  return AliasResult::MayAlias;
}

ModRefResult SharedMemoryAliasAnalysis::getModRef(Operation *op,
                                                  Value location) {
  // TODO: implement
  return ModRefResult::getModAndRef();
}

} // namespace mlir