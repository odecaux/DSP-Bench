/* date = November 29th 2021 10:08 am */

#ifndef COMILER_H
#define COMILER_H


#include <clang/Lex/PreprocessorOptions.h>


#include <clang/Sema/Sema.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <clang/Serialization/PCHContainerOperations.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Basic/Builtins.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecordLayout.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/ParseAST.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/CodeGen/CodeGenAction.h>


#include "clang/Frontend/TextDiagnosticBuffer.h"
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Frontend/FrontendActions.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/InitializePasses.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Analysis/AliasAnalysis.h>


#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif
void* create_clang_context();

#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif
Plugin try_compile(const char* filename, void* clang_ctx_ptr, Plugin_Allocator *allocator);

#ifdef DEBUG
extern "C" __declspec(dllexport) 
#endif
void release_jit(Plugin *plugin);


#ifdef DEBUG 
extern "C" __declspec(dllexport)
#endif
void release_clang_ctx(void* clang_ctx_void);


#endif //COMILER_H
