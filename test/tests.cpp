#include "stdlib.h"
#include "assert.h"
#include "stdio.h"

#include "../base.h"
#include "../descriptor.h"

enum TEST_ENUM{
    A = 0,
    B = 1,
    C = 256
};

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
    //Plugin_Descriptor_Parameter parameters[2];
    Plugin_Descriptor descriptor;
    descriptor.num_parameters = 1;
    
    test_normalization();
    test_ring_buffer_single_threaded_single_value();
    test_ring_buffer_single_threaded_multiple_values();
    
}