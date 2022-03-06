/* date = November 29th 2021 10:08 am */

#ifndef COMILER_H
#define COMILER_H

/* date = November 15th 2021 1:55 pm */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iterator>


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

#include <clang/Frontend/TextDiagnosticPrinter.h>
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

#include "base.h"
#include "structs.h"



struct Clang_Context {
    bool succeeded = true;
    //clang::CompilerInstance compiler_instance;
    llvm::LLVMContext llvm_context;
    
    llvm::ModulePassManager module_pass_manager;
    llvm::ModuleAnalysisManager moduleAnalysisManager;
    llvm::CGSCCAnalysisManager cGSCCAnalysisManager;
    llvm::FunctionAnalysisManager functionAnalysisManager;
    llvm::LoopAnalysisManager loopAnalysisManager;
    
};

struct Decl_Handle{
    Compiler_Error error;
    union {
        const clang::FunctionDecl* fun;
        const clang::CXXRecordDecl* record;
    };
};

struct Plugin_Required_Decls{
    Compiler_Error error;
    Decl_Handle audio_callback;
    Decl_Handle default_parameters;
    Decl_Handle initialize_state;
    Decl_Handle parameters_struct;
    Decl_Handle state_struct;
};



Clang_Context* create_clang_context_impl();

internal String allocate_and_copy_llvm_stringref(llvm::StringRef llvm_stringref)
{
    String new_string;
    new_string.size = llvm_stringref.size();
    new_string.str = new char[llvm_stringref.size()]; 
    strncpy(new_string.str, llvm_stringref.data(), new_string.size);
    return new_string;
}

template<typename Exec>
class Consumer : public clang::ASTConsumer
{
    public:
    explicit Consumer(Exec&& exec) : exec(exec), clang::ASTConsumer() { }
    
    virtual void HandleTranslationUnit(clang::ASTContext &Context)
    {
        exec(Context);
    }
    
    Exec& exec;
};



template<typename Exec>
class Action : public clang::ASTFrontendAction {
    
    Exec& exec;
    
    public:
    
    explicit Action(Exec&& exec) : exec{exec} {}
    
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
        return std::make_unique<Consumer<Exec>>(exec);
    }
};


template<typename Exec>
internal Action<Exec> make_action(Exec&&  exec)
{
    return Action<Exec>(exec); 
}



template<typename Exec>
class MatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback
{
    public:
    explicit MatchCallback(Exec exec) : exec(exec) { }
    
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult& result)
    {
        exec(result);
    }
    
    Exec& exec;
};


template<typename Exec>
internal MatchCallback<Exec> make_match_callback(Exec&&  exec)
{
    return MatchCallback<Exec>(exec); 
}

template<typename Matcher, typename Exec>
internal void match_ast(Matcher& matcher, Exec& exec, clang::ASTContext& ast_ctx)
{
    using namespace clang::ast_matchers;
    
    auto match_finder = MatchFinder{};
    
    
    auto matcher_callback = make_match_callback(exec);
    match_finder.addMatcher(matcher, &matcher_callback);
    match_finder.matchAST(ast_ctx);
} 

template<typename Matcher, typename Exec, typename Node>
internal void match_node(Matcher& matcher, Exec& exec, const Node& node, clang::ASTContext& ast_ctx)
{
    using namespace clang::ast_matchers;
    
    auto match_finder = MatchFinder{};
    
    
    auto matcher_callback = make_match_callback(exec);
    match_finder.addMatcher(matcher, &matcher_callback);
    match_finder.match<Node>(node, ast_ctx);
} 



internal void print_parameter(Plugin_Descriptor_Parameter parameter)
{
    switch(parameter.type){
        case Int : {
            std::cout << "Int type : min = " << parameter.int_param.min << ", max = " << parameter.int_param.max << "\n\n";
            
        }break;
        
        case Float : {
            std::cout << "Float type : min = " << parameter.float_param.min << ", max = " << parameter.float_param.max << "\n\n";
            
        }break;
        
        case Enum : {
            std::cout << "Enum type :\n";
            
            auto enum_param = parameter.enum_param;
            for(auto i = 0; i < enum_param.num_entries; i++)
            {
                //TODO si cest pas NULL-terminated ça va faire une grosse erreur
                std::cout << "   " << enum_param.entries[i].name.str << " : " << enum_param.entries[i].value << "\n";
            }
            std::cout << "\n";
        }break;
    }
}



Plugin_Required_Decls find_decls(clang::ASTContext& ast_ctx);

Plugin_Descriptor parse_plugin_descriptor(const clang::CXXRecordDecl* parameters_struct_decl, 
                                          const clang::CXXRecordDecl* state_struct_decl);

std::unique_ptr<llvm::MemoryBuffer> 
rewrite_plugin_source(Plugin_Required_Decls decls,
                      clang::SourceManager& source_manager,
                      clang::LangOptions& language_options,
                      clang::FileID plugin_source_file_id);

Plugin_Handle jit_compile(llvm::MemoryBufferRef new_buffer, clang::CompilerInstance& compiler_instance,
                          Plugin_Descriptor& descriptor,
                          llvm::LLVMContext *llvm_context,
                          llvm::ModulePassManager& module_pass_manager,
                          llvm::ModuleAnalysisManager& module_analysis_manager,
                          Compiler_Errors *errors);

Plugin_Handle try_compile_impl(const char* filename, Clang_Context* clang_cts, Compiler_Errors *errors);


#if _WIN32
#pragma comment(lib, "version.lib")

////////////////////////////////////////////////////////////////////////////
// Clang library


#pragma comment(lib, "clangAnalysis.lib")
#pragma comment(lib, "clangAPINotes.lib")
#pragma comment(lib, "clangARCMigrate.lib")
#pragma comment(lib, "clangAST.lib")
#pragma comment(lib, "clangASTMatchers.lib")
#pragma comment(lib, "clangBasic.lib")
#pragma comment(lib, "clangCodeGen.lib")
#pragma comment(lib, "clangCrossTU.lib")
#pragma comment(lib, "clangDependencyScanning.lib")
#pragma comment(lib, "clangDirectoryWatcher.lib")
#pragma comment(lib, "clangDriver.lib")
#pragma comment(lib, "clangAstMatchers.lib")
//#pragma comment(lib, "clangDynamicASTMatchers.lib")
#pragma comment(lib, "clangEdit.lib")
#pragma comment(lib, "clangFrontend.lib")
#pragma comment(lib, "clangFrontendTool.lib")
#pragma comment(lib, "clangHandleCXX.lib")
#pragma comment(lib, "clangHandleLLVM.lib")
#pragma comment(lib, "clangIndex.lib")
#pragma comment(lib, "clangIndexSerialization.lib")
#pragma comment(lib, "clangLex.lib")
#pragma comment(lib, "clangParse.lib")
#pragma comment(lib, "clangRewrite.lib")
#pragma comment(lib, "clangRewriteFrontend.lib")
#pragma comment(lib, "clangSema.lib")
#pragma comment(lib, "clangSerialization.lib")
#pragma comment(lib, "clangStaticAnalyzerCheckers.lib")
#pragma comment(lib, "clangStaticAnalyzerCore.lib")
#pragma comment(lib, "clangStaticAnalyzerFrontend.lib")
#pragma comment(lib, "clangTooling.lib")
#pragma comment(lib, "clangToolingASTDiff.lib")
#pragma comment(lib, "clangToolingCore.lib")
#pragma comment(lib, "clangToolingInclusions.lib")
#pragma comment(lib, "clangToolingRefactoring.lib")
#pragma comment(lib, "clangToolingSyntax.lib")
#pragma comment(lib, "clangTransformer.lib")
#pragma comment(lib, "libclang.lib")
#pragma comment(lib, "LLVMAggressiveInstCombine.lib")

#pragma comment(lib, "LLVMAnalysis.lib")

#pragma comment(lib, "LLVMAsmParser.lib")
#pragma comment(lib, "LLVMAsmPrinter.lib")

#pragma comment(lib, "LLVMBinaryFormat.lib")
#pragma comment(lib, "LLVMBitReader.lib")
#pragma comment(lib, "LLVMBitstreamReader.lib")
#pragma comment(lib, "LLVMBitWriter.lib")

#pragma comment(lib, "LLVMCFGuard.lib")
#pragma comment(lib, "LLVMCFIVerify.lib")
#pragma comment(lib, "LLVMCodeGen.lib")
#pragma comment(lib, "LLVMCore.lib")
#pragma comment(lib, "LLVMCoroutines.lib")
#pragma comment(lib, "LLVMCoverage.lib")
#pragma comment(lib, "LLVMDebugInfoCodeView.lib")
#pragma comment(lib, "LLVMDebugInfoDWARF.lib")
#pragma comment(lib, "LLVMDebugInfoGSYM.lib")
#pragma comment(lib, "LLVMDebugInfoMSF.lib")
#pragma comment(lib, "LLVMDebugInfoPDB.lib")
#pragma comment(lib, "LLVMDemangle.lib")
#pragma comment(lib, "LLVMDlltoolDriver.lib")
#pragma comment(lib, "LLVMDWARFLinker.lib")
#pragma comment(lib, "LLVMDWP.lib")
#pragma comment(lib, "LLVMExecutionEngine.lib")

#pragma comment(lib, "LLVMExtensions.lib")
#pragma comment(lib, "LLVMFileCheck.lib")
#pragma comment(lib, "LLVMFrontendOpenACC.lib")
#pragma comment(lib, "LLVMFrontendOpenMP.lib")
#pragma comment(lib, "LLVMFuzzMutate.lib")
#pragma comment(lib, "LLVMGlobalISel.lib")

#pragma comment(lib, "LLVMInstCombine.lib")
#pragma comment(lib, "LLVMInstrumentation.lib")
#pragma comment(lib, "LLVMInterfaceStub.lib")
#pragma comment(lib, "LLVMInterpreter.lib")
#pragma comment(lib, "LLVMipo.lib")
#pragma comment(lib, "LLVMIRReader.lib")
#pragma comment(lib, "LLVMJITLink.lib")

#pragma comment(lib, "LLVMLibDriver.lib")
#pragma comment(lib, "LLVMLineEditor.lib")
#pragma comment(lib, "LLVMLinker.lib")
#pragma comment(lib, "LLVMLTO.lib")
#pragma comment(lib, "LLVMMC.lib")
#pragma comment(lib, "LLVMMCA.lib")
//#pragma comment(lib, "LLVMMCACustomBehaviourAMDGPU.lib")
#pragma comment(lib, "LLVMMCDisassembler.lib")
#pragma comment(lib, "LLVMMCJIT.lib")
#pragma comment(lib, "LLVMMCParser.lib")

#pragma comment(lib, "LLVMMIRParser.lib")


#pragma comment(lib, "LLVMObjCARCOpts.lib")
#pragma comment(lib, "LLVMObject.lib")
#pragma comment(lib, "LLVMObjectYAML.lib")
#pragma comment(lib, "LLVMOption.lib")
#pragma comment(lib, "LLVMOrcJIT.lib")
#pragma comment(lib, "LLVMOrcShared.lib")
#pragma comment(lib, "LLVMOrcTargetProcess.lib")

#pragma comment(lib, "LLVMPasses.lib")

#pragma comment(lib, "LLVMProfileData.lib")
#pragma comment(lib, "LLVMRemarks.lib")

#pragma comment(lib, "LLVMRuntimeDyld.lib")
#pragma comment(lib, "LLVMScalarOpts.lib")
#pragma comment(lib, "LLVMSelectionDAG.lib")
#pragma comment(lib, "LLVMSupport.lib")
#pragma comment(lib, "LLVMSymbolize.lib")

#pragma comment(lib, "LLVMTableGen.lib")
#pragma comment(lib, "LLVMTableGenGlobalISel.lib")
#pragma comment(lib, "LLVMTarget.lib")
#pragma comment(lib, "LLVMTextAPI.lib")
#pragma comment(lib, "LLVMTransformUtils.lib")
#pragma comment(lib, "LLVMVectorize.lib")

#pragma comment(lib, "LLVMWindowsManifest.lib")

#pragma comment(lib, "LTO.lib")

#pragma comment(lib, "LLVMX86AsmParser.lib")
#pragma comment(lib, "LLVMX86CodeGen.lib")
#pragma comment(lib, "LLVMX86Desc.lib")
#pragma comment(lib, "LLVMX86Disassembler.lib")
#pragma comment(lib, "LLVMX86Info.lib")

#endif //MSVC


#endif //COMILER_H
