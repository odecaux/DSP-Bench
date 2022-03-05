#include "stdlib.h"
#include "assert.h"
#include "stdio.h"

#include "../compiler.h"

#include "../base.h"
#include "../descriptor.h"


enum TEST_ENUM{
    A = 0,
    B = 1,
    C = 256
};

bool test_init_compiler()
{
    auto* clang_ctx = create_clang_context_impl();
    
    delete clang_ctx;
    printf("compiler creation passed\n");
    
    return true;
}


bool test_compiler_works()
{
    bool worked = false;
    auto* clang_ctx = create_clang_context_impl();
    
    
    char source[] = 
        "int main() { return 0; }"
        ;
    
    auto buffer = llvm::MemoryBuffer::getMemBuffer(source);
    
    auto new_file = clang::FrontendInputFile{
        buffer->getMemBufferRef(), 
        clang::InputKind{clang::Language::CXX}
    };
    
    auto& compiler_instance = clang_ctx->compiler_instance;
    
    compiler_instance.getFrontendOpts().Inputs.clear();
    compiler_instance.getFrontendOpts().Inputs.push_back(new_file);
    
    llvm::LLVMContext llvm_context;
    auto compile_action = std::make_unique<clang::EmitLLVMOnlyAction>(&llvm_context);
    
    if (compiler_instance.ExecuteAction(*compile_action)) 
    {
        printf("tried compiling\n");
        if(compiler_instance.getDiagnostics().getNumErrors() == 0)
        {
            std::cout << "compilation worked\n";
            worked = true;
        }
    }
    
    delete clang_ctx;
    return worked;
}

bool test_compile_file()
{
    bool worked = false;
    auto* clang_ctx = create_clang_context_impl();
    
    
    
    auto new_file = clang::FrontendInputFile{
        "../test/no_op.cpp", 
        clang::InputKind{clang::Language::CXX}
    };
    
    auto& compiler_instance = clang_ctx->compiler_instance;
    
    compiler_instance.getFrontendOpts().Inputs.clear();
    compiler_instance.getFrontendOpts().Inputs.push_back(new_file);
    
    llvm::LLVMContext llvm_context;
    auto compile_action = std::make_unique<clang::EmitLLVMOnlyAction>(&llvm_context);
    
    if (compiler_instance.ExecuteAction(*compile_action)) 
    {
        printf("tried compiling\n");
        if(compiler_instance.getDiagnostics().getNumErrors() == 0)
        {
            std::cout << "compilation worked\n";
            worked = true;
        }
    }
    
    delete clang_ctx;
    return worked;
}


bool test_parse_parameters()
{
    bool worked = false;
    auto* clang_ctx = create_clang_context_impl();
    
    
    auto new_file = clang::FrontendInputFile{
        "../test/plugin_with_parameters.cpp", 
        clang::InputKind{clang::Language::CXX}
    };
    
    auto& compiler_instance = clang_ctx->compiler_instance;
    
    compiler_instance.getFrontendOpts().Inputs.clear();
    compiler_instance.getFrontendOpts().Inputs.push_back(new_file);
    
    Plugin_Descriptor descriptor;
    
    auto visit_ast = [&](clang::ASTContext& ast_ctx){
        auto decls = find_decls(ast_ctx);
        if(!decls.worked)
        {
            return;
        }
        
        if(!parse_plugin_descriptor(decls.parameters_struct, decls.state_struct, descriptor))
        {
            return;
        }
        
        if(descriptor.num_parameters != 3)
            return;
        
        auto& first_param = descriptor.parameters[0];
        auto& first_param_int = first_param.int_param;
        
        auto& second_param = descriptor.parameters[1];
        auto& second_param_float = second_param.float_param;
        
        auto& third_param = descriptor.parameters[2];
        auto& third_param_enum = third_param.enum_param;
        
        if( /*compare name*/ 
           first_param.type != Int ||
           first_param_int.min != 0 ||
           first_param_int.max != 4 ||
           
           second_param.type != Float ||
           second_param_float.min != 0.0f ||
           second_param_float.max != 1.0f ||
           
           third_param.type != Enum ||
           third_param_enum.num_entries != 4)
            
        {
            return;
        }
        
        worked = true;
    };
    
    auto action = make_action(visit_ast);
    compiler_instance.ExecuteAction(action);
    
    
    delete clang_ctx;
    return worked;
}

bool test_initialization()
{
    bool worked = false;
    auto* clang_ctx = create_clang_context_impl();
    
    auto handle = try_compile_impl("../test/plugin_with_parameters.cpp", clang_ctx);
    
    if(handle.worked)
    {
        char* plugin_parameters_holder = (char*) malloc(handle.descriptor.parameters_struct.size);
        char* plugin_state_holder = (char*) malloc(handle.descriptor.state_struct.size);
        
        handle.default_parameters_f(plugin_parameters_holder);
        handle.initialize_state_f(plugin_parameters_holder, 
                                  plugin_state_holder, 
                                  1,
                                  44100.0f, 
                                  nullptr
                                  );
        
        int ish = *(int*)(plugin_parameters_holder);
        float gain = *(float*)(plugin_parameters_holder + 4);
        int truc_value = *(int*)(plugin_parameters_holder + 8) ;
        float state_gain = *(float*)(plugin_state_holder);
        
        if(ish == 0 &&
           float_cmp(gain , 0.9f, 0.001f) &&
           truc_value == 0 &&
           float_cmp(state_gain , 0.1f, 0.001f))
        {
            worked = true;
        }
        
        delete plugin_state_holder;
        delete plugin_parameters_holder;
    }
    
    delete clang_ctx;
    return worked;
}


bool test_no_op_callback()
{
    bool worked = false;
    auto* clang_ctx = create_clang_context_impl();
    
    auto handle = try_compile_impl("../test/no_op.cpp", clang_ctx);
    
    if(handle.worked)
    {
        float* buffer = (float*) malloc(sizeof(float) * 512);
        
        for(auto i = 0; i < 512; i++)
        {
            buffer[i] = (float) i;
        }
        
        bool all_samples_stayed_the_same = true;
        
        handle.audio_callback_f(nullptr, nullptr, &buffer, 1, 512, 44100.0f);
        
        for(auto i = 0; i < 512; i++)
        {
            if(buffer[i] != (float) i)
            {
                all_samples_stayed_the_same = false;
                break;
            }
            
            if(all_samples_stayed_the_same)
                worked = true;
            delete buffer;
        }
        
    }
    
    delete clang_ctx;
    return worked;
}


bool test_static_gain_callback()
{
    bool worked = false;
    auto* clang_ctx = create_clang_context_impl();
    
    auto handle = try_compile_impl("../test/static_gain_plugin.cpp", clang_ctx);
    
    if(handle.worked)
    {
        
        char* plugin_state_holder = (char*) malloc(handle.descriptor.state_struct.size);
        
        handle.initialize_state_f(nullptr, 
                                  plugin_state_holder, 
                                  1,
                                  44100.0f, 
                                  nullptr);
        
        float* buffer = (float*) malloc(sizeof(float) * 512);
        
        for(auto i = 0; i < 512; i++)
        {
            buffer[i] = (float) i;
        }
        
        bool all_samples_stayed_the_same = true;
        
        handle.audio_callback_f(nullptr, plugin_state_holder, &buffer, 1, 512, 44100.0f);
        
        for(auto i = 0; i < 512; i++)
        {
            if(!float_cmp((float) i * 0.1f, buffer[i], 0.001f))
            {
                all_samples_stayed_the_same = false;
                break;
            }
        }
        
        if(all_samples_stayed_the_same)
            worked = true;
        delete buffer;
        delete plugin_state_holder;
        
        
    }
    
    delete clang_ctx;
    return worked;
}

void test_normalization()
{
    
    {
        Parameter_Int int_param = {4,8};
        int int_value = 6;
        
        float normalized_int = normalize_parameter_int_value(int_param, int_value);
        assert(normalized_int == 0.5f);
        int denormalized_int = denormalize_int_value(int_param, normalized_int);
        assert(int_value == denormalized_int);
    }
    {
        Parameter_Float float_param = {4.0f,8.0f};
        float float_value = 6.0f;
        
        float normalized_float = normalize_parameter_float_value(float_param, float_value);
        assert(normalized_float == 0.5f);
        float denormalized_float = denormalize_float_value(float_param, normalized_float);
        assert(float_value == denormalized_float);
    }
    
    {
        Parameter_Enum_Entry enum_entries[3] = {
            {0},
            {1},
            {256}
        };
        Parameter_Enum enum_param = {&enum_entries[0], 3};
        int value = 256;
        u32 index = enum_value_to_index(enum_param, value);
        
        
        
        float normalized_index = normalize_parameter_enum_index(enum_param, index);
        float denormalized_index = denormalize_enum_index(enum_param, normalized_index);
        assert(denormalized_index == 2);
        int denormalized_value = enum_index_to_value(enum_param, denormalized_index);
        assert(denormalized_value == 256);
    }
    
    
    printf("normalization passed\n");
}


void test_ring_buffer_single_threaded_single_value()
{
    auto ring = 
        plugin_parameters_ring_buffer_initialize(1, 4);
    
    for(auto i = 0; i < 8; i++)
    {
        Plugin_Parameter_Value pushed_value;
        pushed_value.int_value = i;
        
        plugin_parameters_buffer_push(ring, &pushed_value);
        
        auto* pulled_value = plugin_parameters_buffer_pull(ring);
        assert(pulled_value->int_value == pushed_value.int_value);
    }
    
    
    for(auto i = 0; i < 8; i++)
    {
        Plugin_Parameter_Value pushed_value;
        pushed_value.float_value = 2.0f * i;
        
        plugin_parameters_buffer_push(ring, &pushed_value);
        
        auto* pulled_value = plugin_parameters_buffer_pull(ring);
        assert(pulled_value->float_value == pushed_value.float_value);
    }
    
    printf("pushing single value passed\n");
}


void test_ring_buffer_single_threaded_multiple_values(){
    auto ring = 
        plugin_parameters_ring_buffer_initialize(4, 4);
    
    for(auto i = 0; i < 8; i++)
    {
        Plugin_Parameter_Value pushed_values[4];
        pushed_values[0].int_value = i;
        pushed_values[1].float_value = (float) i;
        pushed_values[2].enum_value = A;
        pushed_values[3].enum_value = C;
        
        plugin_parameters_buffer_push(ring, &pushed_values[0]);
        
        auto* pulled_value = plugin_parameters_buffer_pull(ring);
        
        assert(pulled_value[0].int_value == i);
        assert(pulled_value[1].float_value == (float) i); 
        assert(pulled_value[2].enum_value == A);
        assert(pulled_value[3].enum_value == C);
    }
    
    
    for(auto i = 0; i < 8; i++)
    {
        Plugin_Parameter_Value pushed_value;
        pushed_value.float_value = 2.0f * i;
        
        plugin_parameters_buffer_push(ring, &pushed_value);
        
        auto* pulled_value = plugin_parameters_buffer_pull(ring);
        assert(pulled_value->float_value == pushed_value.float_value);
    }
    
    printf("pushing multiple values passed\n");
}

int main()
{
    
    //test_reuse_compiler()
    assert(test_init_compiler());
    assert(test_compiler_works());
    assert(test_compile_file());
    assert(test_parse_parameters());
    assert(test_initialization());
    assert(test_no_op_callback());
    assert(test_static_gain_callback());
    
    test_normalization();
    test_ring_buffer_single_threaded_single_value();
    test_ring_buffer_single_threaded_multiple_values();
    
    return 0;
}