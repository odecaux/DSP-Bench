/* date = November 15th 2021 1:55 pm */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iterator>

#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Sema/Sema.h"


#include "clang/Serialization/PCHContainerOperations.h"
#include <clang/Rewrite/Core/Rewriter.h>
#include "clang/Basic/Builtins.h"
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

#include "clang/Frontend/TextDiagnosticPrinter.h"
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

#include "base.h"
#include "structs.h"

String allocate_and_copy_llvm_stringref(llvm::StringRef llvm_stringref)
{
    String new_string;
    new_string.size = llvm_stringref.size();
    new_string.str = new char[llvm_stringref.size()]; //TODO string type
    strcpy(new_string.str, llvm_stringref.data());
    return new_string;
}

static void print_parameter(Plugin_Descriptor_Parameter parameter)
{
    switch(parameter.type){
        case Int : {
            std::cout<<"Int type : min = " << parameter.int_param.min << ", max = " << parameter.int_param.max << "\n\n";
            
        }break;
        
        case Float : {
            std::cout<<"Float type : min = " << parameter.float_param.min << ", max = " << parameter.float_param.max << "\n\n";
            
        }break;
        
        case Enum : {
            std::cout<<"Enum type :\n";
            
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


template<typename Exec>
class Consumer : public clang::ASTConsumer
{
    public:
    explicit Consumer(Exec&& exec) : exec(exec), clang::ASTConsumer() { }
    
    virtual void HandleTranslationUnit(clang::ASTContext &Context)
    {
        exec(Context);
    }
    
    Exec exec;
};

template<typename Exec>
Consumer<Exec> make_consumer(Exec&&  exec)
{
    return Consumer<Exec>(exec); 
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
    
    Exec exec;
};


template<typename Exec>
MatchCallback<Exec> make_match_callback(Exec&&  exec)
{
    return MatchCallback<Exec>(exec); 
}

template<typename Matcher, typename Exec>
void match_ast(Matcher& matcher, Exec& exec, clang::ASTContext& ast_ctx)
{
    using namespace clang::ast_matchers;
    
    auto match_finder = MatchFinder{};
    
    
    auto matcher_callback = make_match_callback(exec);
    match_finder.addMatcher(matcher, &matcher_callback);
    match_finder.matchAST(ast_ctx);
} 



template<typename Matcher, typename Exec, typename Node>
void match_node(Matcher& matcher, Exec& exec, const Node& node, clang::ASTContext& ast_ctx)
{
    using namespace clang::ast_matchers;
    
    auto match_finder = MatchFinder{};
    
    
    auto matcher_callback = make_match_callback(exec);
    match_finder.addMatcher(matcher, &matcher_callback);
    match_finder.match<Node>(node, ast_ctx);
} 




extern "C" __declspec(dllexport) Plugin_Handle try_compile(const char* filename)
{
    
    clang::DiagnosticOptions diagnosticOptions;
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
    
    clang::TextDiagnosticPrinter *pTextDiagnosticPrinter =
        new clang::TextDiagnosticPrinter(
                                         llvm::outs(),
                                         &diagnosticOptions);
    
    clang::DiagnosticsEngine pDiagnosticsEngine (pDiagIDs,
                                                 &diagnosticOptions,
                                                 pTextDiagnosticPrinter);
    
    clang::LangOptions languageOptions;
    languageOptions.Bool = 1;
    languageOptions.CPlusPlus = 1;
    
    clang::FileSystemOptions fileSystemOptions;
    clang::FileManager fileManager(fileSystemOptions);
    
    clang::SourceManager sourceManager(pDiagnosticsEngine,
                                       fileManager);
    
    std::shared_ptr<clang::HeaderSearchOptions> headerSearchOptions(new clang::HeaderSearchOptions());
    
    const std::shared_ptr<clang::TargetOptions> targetOptions = std::make_shared<clang::TargetOptions>();
    targetOptions->Triple = llvm::sys::getDefaultTargetTriple();
    
    clang::TargetInfo *pTargetInfo = 
        clang::TargetInfo::CreateTargetInfo(
                                            pDiagnosticsEngine,
                                            targetOptions);
    
    clang::HeaderSearch headerSearch(headerSearchOptions,
                                     sourceManager, 
                                     pDiagnosticsEngine,
                                     languageOptions,
                                     pTargetInfo);
    clang::CompilerInstance compInst;
    
    
    std::shared_ptr<clang::PreprocessorOptions> pOpts( new clang::PreprocessorOptions());
    clang::Preprocessor preprocessor(pOpts,
                                     pDiagnosticsEngine,
                                     languageOptions,
                                     sourceManager,
                                     headerSearch,
                                     compInst);
    preprocessor.Initialize(*pTargetInfo);
    
    clang::PCHContainerOperations pchContainer{};
    
    clang::FrontendOptions frontendOptions;
    
    clang::InitializePreprocessor(preprocessor,
                                  *pOpts,
                                  pchContainer.getRawReader(),
                                  frontendOptions);
    
    clang::ApplyHeaderSearchOptions( preprocessor.getHeaderSearchInfo(),
                                    compInst.getHeaderSearchOpts(),
                                    preprocessor.getLangOpts(),
                                    preprocessor.getTargetInfo().getTriple());
    
    
    llvm::ErrorOr< const clang::FileEntry * > pFile = fileManager.getFile(filename);
    sourceManager.setMainFileID( sourceManager.createFileID( *pFile, clang::SourceLocation(), clang::SrcMgr::C_User));
    
    const clang::TargetInfo &targetInfo = *pTargetInfo;
    
    clang::IdentifierTable identifierTable(languageOptions);
    clang::SelectorTable selectorTable;
    
    clang::Builtin::Context builtinContext;
    builtinContext.InitializeTarget(targetInfo, nullptr);
    
    clang::ASTContext astContext(languageOptions,
                                 sourceManager,
                                 identifierTable,
                                 selectorTable,
                                 builtinContext,
                                 clang::TU_Complete);
    
    astContext.InitBuiltinTypes(*pTargetInfo);
    
    Plugin_Handle handle;
    
    bool error = false;
    
    auto visit_ast = [&](clang::ASTContext& ast_ctx){
        
        using namespace clang::ast_matchers;
        
        
        //~ does functions exists ?
        
        const clang::FunctionDecl* audio_callback_decl = nullptr;
        
        auto audio_callback_matcher = functionDecl(hasName("audio_callback")).bind("callback");
        auto audio_callback_lambda = [&](const auto& result)
        {
            if(error) return; 
            
            const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("callback");
            
            if(decl == nullptr) return;
            if(!decl->isThisDeclarationADefinition()) return;
            if(audio_callback_decl == nullptr)
            {
                audio_callback_decl = decl;
            }
            else
            {
                std::cout<<"too many audio_callback function\n";
                error = true;
            }
        };
        match_ast(audio_callback_matcher, audio_callback_lambda, ast_ctx);
        
        if(audio_callback_decl == nullptr)
        {
            error = true;
            std::cout<<"couldn't find an audio_callback function\n";
            return;
        }
        
        
        if(error)
            return;
        
        const clang::FunctionDecl* default_parameters_decl = nullptr;
        
        auto default_parameters_matcher = functionDecl(hasName("default_parameters")).bind("default_param");
        auto default_parameters_lambda = [&](const auto& result)
        {
            if(error) return; 
            
            const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("default_param");
            
            if(decl == nullptr) return;
            if(!decl->isThisDeclarationADefinition()) return;
            if(default_parameters_decl == nullptr)
            {
                default_parameters_decl = decl;
            }
            else
            {
                std::cout<<"too many default_parameters function\n";
                error = true;
            }
        };
        match_ast(default_parameters_matcher, default_parameters_lambda, ast_ctx);
        
        if(default_parameters_decl == nullptr)
        {
            error = true;
            std::cout<<"couldn't find an default_parameters function\n";
            return;
        }
        if(error)
            return;
        
        
        
        const clang::FunctionDecl* initialize_state_decl = nullptr;
        
        auto initialize_state_matcher = functionDecl(hasName("initialize_state")).bind("initialize_state");
        auto initialize_state_lambda = [&](const auto& result)
        {
            if(error) return; 
            
            const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("initialize_state");
            
            if(decl == nullptr) return;
            if(!decl->isThisDeclarationADefinition()) return;
            if(initialize_state_decl == nullptr)
            {
                initialize_state_decl = decl;
            }
            else
            {
                std::cout<<"too many initialize_state function\n";
                error = true;
            }
        };
        match_ast(initialize_state_matcher, initialize_state_lambda, ast_ctx);
        
        if(initialize_state_decl == nullptr)
        {
            error = true;
            std::cout<<"couldn't find an initialize_state function\n";
            return;
        }
        if(error)
            return;
        
        //~ Do they have the right parameters / return types ?
        
        auto audio_callback_has_right_signature_matcher = 
            functionDecl(hasName("audio_callback"), 
                         parameterCountIs(6),
                         hasBody(compoundStmt()),
                         
                         hasParameter(2, hasType(pointsTo(pointsTo(realFloatingPointType())))),
                         hasParameter(3, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), isUnsignedInteger()))))),
                         hasParameter(4, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), isUnsignedInteger()))))),
                         hasParameter(5, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), realFloatingPointType()))))),
                         
                         returns(voidType()))
            .bind("callback");
        
        bool has_audio_callback_the_right_signature = false;
        
        auto audio_callback_has_right_signature_lambda = [&](const auto& result)
        {
            if(error) return; 
            const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("callback");
            if(decl == nullptr) return;
            if(!decl->isThisDeclarationADefinition()) return;
            
            if(has_audio_callback_the_right_signature == false)
            {
                has_audio_callback_the_right_signature = true;
            }
            else
            {
                error = true;
            }
        };
        
        
        match_node(audio_callback_has_right_signature_matcher,
                   audio_callback_has_right_signature_lambda,
                   *audio_callback_decl,
                   ast_ctx);
        
        
        if(has_audio_callback_the_right_signature == false)
        {
            std::cout << "audio_callback doesn't have the right signature\n";
            return;
        }
        if(error) return;
        
        
        auto initialize_state_has_right_signature_matcher = 
            functionDecl(hasName("initialize_state"), 
                         parameterCountIs(4),
                         hasBody(compoundStmt()),
                         
                         hasParameter(1, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), isUnsignedInteger()))))),
                         hasParameter(2, varDecl(hasType(hasCanonicalType(allOf(isConstQualified(), realFloatingPointType())))))
                         ,
                         hasParameter(3, varDecl(hasType(hasCanonicalType(functionType())))))
            .bind("initialize_state");
        
        bool has_initialize_state_the_right_signature = false;
        
        auto initialize_state_has_right_signature_lambda = [&](const auto& result)
        {
            if(error) return; 
            const clang::FunctionDecl* decl = result.Nodes.getNodeAs<clang::FunctionDecl>("initialize_state");
            if(decl == nullptr) return;
            if(!decl->isThisDeclarationADefinition()) return;
            
            if(has_initialize_state_the_right_signature == false)
            {
                has_initialize_state_the_right_signature = true;
            }
            else
            {
                error = true;
            }
        };
        
        match_node(initialize_state_has_right_signature_matcher,
                   initialize_state_has_right_signature_lambda,
                   *initialize_state_decl,
                   ast_ctx);
        
        if(has_initialize_state_the_right_signature == false)
        {
            std::cout << "initialize_state doesn't have the right signature\n";
        }
        if(error) return;
        
        
        
        return; 
    };
    
    auto astConsumer = make_consumer(visit_ast);
    
    clang::Sema sema(preprocessor,
                     astContext,
                     astConsumer);
    
    pTextDiagnosticPrinter->BeginSourceFile(languageOptions, &preprocessor);
    clang::ParseAST(sema); 
    pTextDiagnosticPrinter->EndSourceFile();
    
    return {false};
}
























////////////////////////////////////////////////////////////////////////////
// Clang Driver needs version.dll for MSVC
////////////////////////////////////////////////////////////////////////////
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
//#pragma comment(lib, "clangFormat.lib")
#pragma comment(lib, "clangFrontend.lib")
#pragma comment(lib, "clangFrontendTool.lib")
#pragma comment(lib, "clangHandleCXX.lib")
#pragma comment(lib, "clangHandleLLVM.lib")
#pragma comment(lib, "clangIndex.lib")
#pragma comment(lib, "clangIndexSerialization.lib")
//#pragma comment(lib, "clangInterpreter.lib")
#pragma comment(lib, "clangLex.lib")
#pragma comment(lib, "clangParse.lib")
#pragma comment(lib, "clangRewrite.lib")
#pragma comment(lib, "clangRewriteFrontend.lib")
#pragma comment(lib, "clangSema.lib")
#pragma comment(lib, "clangSerialization.lib")
#pragma comment(lib, "clangStaticAnalyzerCheckers.lib")
#pragma comment(lib, "clangStaticAnalyzerCore.lib")
#pragma comment(lib, "clangStaticAnalyzerFrontend.lib")
//#pragma comment(lib, "clangTesting.lib")
#pragma comment(lib, "clangTooling.lib")
#pragma comment(lib, "clangToolingASTDiff.lib")
#pragma comment(lib, "clangToolingCore.lib")
#pragma comment(lib, "clangToolingInclusions.lib")
#pragma comment(lib, "clangToolingRefactoring.lib")
#pragma comment(lib, "clangToolingSyntax.lib")
#pragma comment(lib, "clangTransformer.lib")
#pragma comment(lib, "libclang.lib")
#pragma comment(lib, "lldCOFF.lib")
#pragma comment(lib, "lldCommon.lib")
#pragma comment(lib, "lldCore.lib")
#pragma comment(lib, "lldDriver.lib")
//#pragma comment(lib, "lldELF.lib")
#pragma comment(lib, "lldMachO.lib")
#pragma comment(lib, "lldMachO2.lib")
//#pragma comment(lib, "lldMinGW.lib")
#pragma comment(lib, "lldReaderWriter.lib")
//#pragma comment(lib, "lldWasm.lib")
#pragma comment(lib, "lldYAML.lib")
//#pragma comment(lib, "LLVM-C.lib")
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
#pragma comment(lib, "LLVMMCACustomBehaviourAMDGPU.lib")
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
