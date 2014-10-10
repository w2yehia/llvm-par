//===----- CGOpenMPRuntime.cpp - Interface to OpenMP Runtimes -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This provides a class for OpenMP runtime code generation.
//
//===----------------------------------------------------------------------===//

#include "CGOpenMPRuntime.h"
#include "CodeGenFunction.h"
#include "clang/AST/Decl.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

using namespace clang;
using namespace CodeGen;

void CGOpenMPRegionInfo::EmitBody(CodeGenFunction &CGF, Stmt *S) {
  CodeGenFunction::OMPPrivateScope PrivateScope(CGF);
  CGF.EmitOMPFirstprivateClause(Directive, PrivateScope);
  if (PrivateScope.Privatize()) {
    // Emit implicit barrier to synchronize threads and avoid data races.
    auto Flags = static_cast<CGOpenMPRuntime::OpenMPLocationFlags>(
        CGOpenMPRuntime::OMP_IDENT_KMPC |
        CGOpenMPRuntime::OMP_IDENT_BARRIER_IMPL);
    CGF.CGM.getOpenMPRuntime().EmitOMPBarrierCall(CGF, Directive.getLocStart(),
                                                  Flags);
  }
  CGCapturedStmtInfo::EmitBody(CGF, S);
}

CGOpenMPRuntime::CGOpenMPRuntime(CodeGenModule &CGM)
    : CGM(CGM), DefaultOpenMPPSource(nullptr) {
  IdentTy = llvm::StructType::create(
      "ident_t", CGM.Int32Ty /* reserved_1 */, CGM.Int32Ty /* flags */,
      CGM.Int32Ty /* reserved_2 */, CGM.Int32Ty /* reserved_3 */,
      CGM.Int8PtrTy /* psource */, nullptr);
  // Build void (*kmpc_micro)(kmp_int32 *global_tid, kmp_int32 *bound_tid,...)
  llvm::Type *MicroParams[] = {llvm::PointerType::getUnqual(CGM.Int32Ty),
                               llvm::PointerType::getUnqual(CGM.Int32Ty)};
  Kmpc_MicroTy = llvm::FunctionType::get(CGM.VoidTy, MicroParams, true);
  KmpCriticalNameTy = llvm::ArrayType::get(CGM.Int32Ty, /*NumElements*/ 8);
}

llvm::Value *
CGOpenMPRuntime::GetOrCreateDefaultOpenMPLocation(OpenMPLocationFlags Flags) {
  llvm::Value *Entry = OpenMPDefaultLocMap.lookup(Flags);
  if (!Entry) {
    if (!DefaultOpenMPPSource) {
      // Initialize default location for psource field of ident_t structure of
      // all ident_t objects. Format is ";file;function;line;column;;".
      // Taken from
      // http://llvm.org/svn/llvm-project/openmp/trunk/runtime/src/kmp_str.c
      DefaultOpenMPPSource =
          CGM.GetAddrOfConstantCString(";unknown;unknown;0;0;;");
      DefaultOpenMPPSource =
          llvm::ConstantExpr::getBitCast(DefaultOpenMPPSource, CGM.Int8PtrTy);
    }
    auto DefaultOpenMPLocation = new llvm::GlobalVariable(
        CGM.getModule(), IdentTy, /*isConstant*/ true,
        llvm::GlobalValue::PrivateLinkage, /*Initializer*/ nullptr);
    DefaultOpenMPLocation->setUnnamedAddr(true);

    llvm::Constant *Zero = llvm::ConstantInt::get(CGM.Int32Ty, 0, true);
    llvm::Constant *Values[] = {Zero,
                                llvm::ConstantInt::get(CGM.Int32Ty, Flags),
                                Zero, Zero, DefaultOpenMPPSource};
    llvm::Constant *Init = llvm::ConstantStruct::get(IdentTy, Values);
    DefaultOpenMPLocation->setInitializer(Init);
    OpenMPDefaultLocMap[Flags] = DefaultOpenMPLocation;
    return DefaultOpenMPLocation;
  }
  return Entry;
}

llvm::Value *CGOpenMPRuntime::EmitOpenMPUpdateLocation(
    CodeGenFunction &CGF, SourceLocation Loc, OpenMPLocationFlags Flags) {
  // If no debug info is generated - return global default location.
  if (CGM.getCodeGenOpts().getDebugInfo() == CodeGenOptions::NoDebugInfo ||
      Loc.isInvalid())
    return GetOrCreateDefaultOpenMPLocation(Flags);

  assert(CGF.CurFn && "No function in current CodeGenFunction.");

  llvm::Value *LocValue = nullptr;
  OpenMPLocMapTy::iterator I = OpenMPLocMap.find(CGF.CurFn);
  if (I != OpenMPLocMap.end()) {
    LocValue = I->second;
  } else {
    // Generate "ident_t .kmpc_loc.addr;"
    llvm::AllocaInst *AI = CGF.CreateTempAlloca(IdentTy, ".kmpc_loc.addr");
    AI->setAlignment(CGM.getDataLayout().getPrefTypeAlignment(IdentTy));
    OpenMPLocMap[CGF.CurFn] = AI;
    LocValue = AI;

    CGBuilderTy::InsertPointGuard IPG(CGF.Builder);
    CGF.Builder.SetInsertPoint(CGF.AllocaInsertPt);
    CGF.Builder.CreateMemCpy(LocValue, GetOrCreateDefaultOpenMPLocation(Flags),
                             llvm::ConstantExpr::getSizeOf(IdentTy),
                             CGM.PointerAlignInBytes);
  }

  // char **psource = &.kmpc_loc_<flags>.addr.psource;
  llvm::Value *PSource =
      CGF.Builder.CreateConstInBoundsGEP2_32(LocValue, 0, IdentField_PSource);

  auto OMPDebugLoc = OpenMPDebugLocMap.lookup(Loc.getRawEncoding());
  if (OMPDebugLoc == nullptr) {
    SmallString<128> Buffer2;
    llvm::raw_svector_ostream OS2(Buffer2);
    // Build debug location
    PresumedLoc PLoc = CGF.getContext().getSourceManager().getPresumedLoc(Loc);
    OS2 << ";" << PLoc.getFilename() << ";";
    if (const FunctionDecl *FD =
            dyn_cast_or_null<FunctionDecl>(CGF.CurFuncDecl)) {
      OS2 << FD->getQualifiedNameAsString();
    }
    OS2 << ";" << PLoc.getLine() << ";" << PLoc.getColumn() << ";;";
    OMPDebugLoc = CGF.Builder.CreateGlobalStringPtr(OS2.str());
    OpenMPDebugLocMap[Loc.getRawEncoding()] = OMPDebugLoc;
  }
  // *psource = ";<File>;<Function>;<Line>;<Column>;;";
  CGF.Builder.CreateStore(OMPDebugLoc, PSource);

  return LocValue;
}

llvm::Value *CGOpenMPRuntime::GetOpenMPThreadID(CodeGenFunction &CGF,
                                                SourceLocation Loc) {
  assert(CGF.CurFn && "No function in current CodeGenFunction.");

  llvm::Value *ThreadID = nullptr;
  OpenMPThreadIDMapTy::iterator I = OpenMPThreadIDMap.find(CGF.CurFn);
  if (I != OpenMPThreadIDMap.end()) {
    ThreadID = I->second;
  } else {
    // Check if current function is a function which has first parameter
    // with type int32 and name ".global_tid.".
    if (!CGF.CurFn->arg_empty() &&
        CGF.CurFn->arg_begin()->getType()->isPointerTy() &&
        CGF.CurFn->arg_begin()
            ->getType()
            ->getPointerElementType()
            ->isIntegerTy() &&
        CGF.CurFn->arg_begin()
                ->getType()
                ->getPointerElementType()
                ->getIntegerBitWidth() == 32 &&
        CGF.CurFn->arg_begin()->hasName() &&
        CGF.CurFn->arg_begin()->getName() == ".global_tid.") {
      CGBuilderTy::InsertPointGuard IPG(CGF.Builder);
      CGF.Builder.SetInsertPoint(CGF.AllocaInsertPt);
      ThreadID = CGF.Builder.CreateLoad(CGF.CurFn->arg_begin());
    } else {
      // Generate "int32 .kmpc_global_thread_num.addr;"
      CGBuilderTy::InsertPointGuard IPG(CGF.Builder);
      CGF.Builder.SetInsertPoint(CGF.AllocaInsertPt);
      llvm::Value *Args[] = {EmitOpenMPUpdateLocation(CGF, Loc)};
      ThreadID = CGF.EmitRuntimeCall(
          CreateRuntimeFunction(OMPRTL__kmpc_global_thread_num), Args);
    }
    OpenMPThreadIDMap[CGF.CurFn] = ThreadID;
  }
  return ThreadID;
}

void CGOpenMPRuntime::FunctionFinished(CodeGenFunction &CGF) {
  assert(CGF.CurFn && "No function in current CodeGenFunction.");
  if (OpenMPThreadIDMap.count(CGF.CurFn))
    OpenMPThreadIDMap.erase(CGF.CurFn);
  if (OpenMPLocMap.count(CGF.CurFn))
    OpenMPLocMap.erase(CGF.CurFn);
}

llvm::Type *CGOpenMPRuntime::getIdentTyPointerTy() {
  return llvm::PointerType::getUnqual(IdentTy);
}

llvm::Type *CGOpenMPRuntime::getKmpc_MicroPointerTy() {
  return llvm::PointerType::getUnqual(Kmpc_MicroTy);
}

llvm::Constant *
CGOpenMPRuntime::CreateRuntimeFunction(OpenMPRTLFunction Function) {
  llvm::Constant *RTLFn = nullptr;
  switch (Function) {
  case OMPRTL__kmpc_fork_call: {
    // Build void __kmpc_fork_call(ident_t *loc, kmp_int32 argc, kmpc_micro
    // microtask, ...);
    llvm::Type *TypeParams[] = {getIdentTyPointerTy(), CGM.Int32Ty,
                                getKmpc_MicroPointerTy()};
    llvm::FunctionType *FnTy =
        llvm::FunctionType::get(CGM.VoidTy, TypeParams, true);
    RTLFn = CGM.CreateRuntimeFunction(FnTy, "__kmpc_fork_call");
    break;
  }
  case OMPRTL__kmpc_global_thread_num: {
    // Build kmp_int32 __kmpc_global_thread_num(ident_t *loc);
    llvm::Type *TypeParams[] = {getIdentTyPointerTy()};
    llvm::FunctionType *FnTy =
        llvm::FunctionType::get(CGM.Int32Ty, TypeParams, false);
    RTLFn = CGM.CreateRuntimeFunction(FnTy, "__kmpc_global_thread_num");
    break;
  }
  case OMPRTL__kmpc_critical: {
    // Build void __kmpc_critical(ident_t *loc, kmp_int32 global_tid,
    // kmp_critical_name *crit);
    llvm::Type *TypeParams[] = {
        getIdentTyPointerTy(), CGM.Int32Ty,
        llvm::PointerType::getUnqual(KmpCriticalNameTy)};
    llvm::FunctionType *FnTy =
        llvm::FunctionType::get(CGM.VoidTy, TypeParams, /*isVarArg*/ false);
    RTLFn = CGM.CreateRuntimeFunction(FnTy, "__kmpc_critical");
    break;
  }
  case OMPRTL__kmpc_end_critical: {
    // Build void __kmpc_end_critical(ident_t *loc, kmp_int32 global_tid,
    // kmp_critical_name *crit);
    llvm::Type *TypeParams[] = {
        getIdentTyPointerTy(), CGM.Int32Ty,
        llvm::PointerType::getUnqual(KmpCriticalNameTy)};
    llvm::FunctionType *FnTy =
        llvm::FunctionType::get(CGM.VoidTy, TypeParams, /*isVarArg*/ false);
    RTLFn = CGM.CreateRuntimeFunction(FnTy, "__kmpc_end_critical");
    break;
  }
  case OMPRTL__kmpc_barrier: {
    // Build void __kmpc_barrier(ident_t *loc, kmp_int32 global_tid);
    llvm::Type *TypeParams[] = {getIdentTyPointerTy(), CGM.Int32Ty};
    llvm::FunctionType *FnTy =
        llvm::FunctionType::get(CGM.VoidTy, TypeParams, /*isVarArg*/ false);
    RTLFn = CGM.CreateRuntimeFunction(FnTy, /*Name*/ "__kmpc_barrier");
    break;
  }
  }
  return RTLFn;
}

void CGOpenMPRuntime::EmitOMPParallelCall(CodeGenFunction &CGF,
                                          SourceLocation Loc,
                                          llvm::Value *OutlinedFn,
                                          llvm::Value *CapturedStruct) {
  // Build call __kmpc_fork_call(loc, 1, microtask, captured_struct/*context*/)
  llvm::Value *Args[] = {
      EmitOpenMPUpdateLocation(CGF, Loc),
      CGF.Builder.getInt32(1), // Number of arguments after 'microtask' argument
      // (there is only one additional argument - 'context')
      CGF.Builder.CreateBitCast(OutlinedFn, getKmpc_MicroPointerTy()),
      CGF.EmitCastToVoidPtr(CapturedStruct)};
  auto RTLFn = CreateRuntimeFunction(CGOpenMPRuntime::OMPRTL__kmpc_fork_call);
  CGF.EmitRuntimeCall(RTLFn, Args);
}

llvm::Value *CGOpenMPRuntime::GetCriticalRegionLock(StringRef CriticalName) {
  SmallString<256> Buffer;
  llvm::raw_svector_ostream Out(Buffer);
  Out << ".gomp_critical_user_" << CriticalName << ".var";
  auto RuntimeCriticalName = Out.str();
  auto &Elem = CriticalRegionVarNames.GetOrCreateValue(RuntimeCriticalName);
  if (Elem.getValue() != nullptr)
    return Elem.getValue();

  auto Lock = new llvm::GlobalVariable(
      CGM.getModule(), KmpCriticalNameTy, /*IsConstant*/ false,
      llvm::GlobalValue::CommonLinkage,
      llvm::Constant::getNullValue(KmpCriticalNameTy), Elem.getKey());
  Elem.setValue(Lock);
  return Lock;
}

void CGOpenMPRuntime::EmitOMPCriticalRegionStart(CodeGenFunction &CGF,
                                                 llvm::Value *RegionLock,
                                                 SourceLocation Loc) {
  // Prepare other arguments and build a call to __kmpc_critical
  llvm::Value *Args[] = {EmitOpenMPUpdateLocation(CGF, Loc),
                         GetOpenMPThreadID(CGF, Loc), RegionLock};
  auto RTLFn = CreateRuntimeFunction(CGOpenMPRuntime::OMPRTL__kmpc_critical);
  CGF.EmitRuntimeCall(RTLFn, Args);
}

void CGOpenMPRuntime::EmitOMPCriticalRegionEnd(CodeGenFunction &CGF,
                                               llvm::Value *RegionLock,
                                               SourceLocation Loc) {
  // Prepare other arguments and build a call to __kmpc_end_critical
  llvm::Value *Args[] = {EmitOpenMPUpdateLocation(CGF, Loc),
                         GetOpenMPThreadID(CGF, Loc), RegionLock};
  auto RTLFn =
      CreateRuntimeFunction(CGOpenMPRuntime::OMPRTL__kmpc_end_critical);
  CGF.EmitRuntimeCall(RTLFn, Args);
}

void CGOpenMPRuntime::EmitOMPBarrierCall(CodeGenFunction &CGF,
                                         SourceLocation Loc,
                                         OpenMPLocationFlags Flags) {
  // Build call __kmpc_barrier(loc, thread_id)
  llvm::Value *Args[] = {EmitOpenMPUpdateLocation(CGF, Loc, Flags),
                         GetOpenMPThreadID(CGF, Loc)};
  auto RTLFn = CreateRuntimeFunction(CGOpenMPRuntime::OMPRTL__kmpc_barrier);
  CGF.EmitRuntimeCall(RTLFn, Args);
}

