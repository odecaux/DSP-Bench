
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"
#include "math.h"
#include "string.h"

#include <initguid.h>
#include "mmdeviceapi.h"
#include "audioclient.h"
#include "winuser.h"

#include "base.h"
#include "structs.h"
#include "memory.h"
#include "audio.h"
#include "plugin.h"

#include "FunctionDiscoveryKeys_devpkey.h"

const IID IID_IAudioClient  = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IMMEndpoint = __uuidof(IMMEndpoint);

REFERENCE_TIME sound_latency_fps = 60;
REFERENCE_TIME requested_sound_duration = 2*100;


struct Audio_Device_Wasapi{
    IMMDevice *mm_device;
    /*
    IAudioClient *client;
    union 
    {
        IAudioRenderClient *render_client;
        IAudioCaptureClient *capture_client;
    };
    */
};


void print_wave_format(WAVEFORMATEX *format)
{
    printf("num channels : %u\n",format->nChannels);
    printf("sample rate : %lu\n", format->nSamplesPerSec);
    printf("bit depth : %u\n", format->wBitsPerSample);
    
    printf("format : ");
    switch(format->wFormatTag){
        case WAVE_FORMAT_IEEE_FLOAT: {
            printf("float\n");
        }break;
        case WAVE_FORMAT_PCM: {
            printf("CM\n");
        }break;
        case WAVE_FORMAT_EXTENSIBLE: 
        {
            printf("extensible\n");
        } break;
        /*case WAVEFORMATIEEEFLOATEX:{
            printf("extented float\n");
        }break;
        case WAVEFORMATCMEX:{
            printf("extented pcm\n");
        }break;*/
    }
    
    printf("\n");
    
}

double from_reftimes(REFERENCE_TIME t)
{
    return double(t) / (1000.0 * 1000.0 * 10.0);
}

REFERENCE_TIME to_reftimes(double seconds)
{
    return REFERENCE_TIME(seconds * 10.0 * 1000.0 * 1000.0 + 0.5f);
}


WAVEFORMATEX format_for(u32 sample_rate, u32 channel_count)
{
    return {
        . wFormatTag = WAVE_FORMAT_IEEE_FLOAT ,
        . nChannels = (WORD)channel_count,
        . nSamplesPerSec = sample_rate,
        . nAvgBytesPerSec = channel_count * sizeof(real32) * sample_rate,
        . nBlockAlign = (WORD)channel_count * sizeof (real32),
        . wBitsPerSample = sizeof (real32) * 8,
        . cbSize = 0,
    };
}

#define if_failed(hr, message) if(FAILED(hr)) { MessageBox(0, message, 0, MB_OK); assert(false);}

bool format_check(IAudioClient *audio_client, WAVEFORMATEX *format)
{
    WAVEFORMATEX *closest_match = NULL;
    HRESULT hr = audio_client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, format, &closest_match);
    if (closest_match) {
        CoTaskMemFree(closest_match);
        closest_match = NULL;
    }
    return hr == S_OK;
}

IMMDeviceEnumerator *get_enumerator()
{
    IMMDeviceEnumerator *enumerator;
    auto hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator,
                               (void**)&enumerator);
    if_failed(hr, "failed to create instance\n");
    return enumerator;
}

IMMDevice *get_default_device(IMMDeviceEnumerator *enumerator, bool input)
{
    IMMDevice *device;
    EDataFlow data_flow = input ? eCapture : eRender;
    auto hr = enumerator->GetDefaultAudioEndpoint(data_flow, eMultimedia, &device);
    if_failed(hr ,"failed to get device\n");
    return device;
}

struct WasapiContext
{
    HANDLE audio_sample_ready_event;
    HANDLE shutdown_event;
    
    IAudioClient *audio_client;
    IAudioRenderClient *render_client;
    
    UINT32 system_num_allocated_samples;
    HANDLE audio_thread;
    
    real32** user_buffer;
    
    Audio_Format format;
    Audio_Thread_Context *audio_context;
};

DWORD audio_thread_fn(LPVOID Context)
{
    WasapiContext *wasapi_context = (WasapiContext*)Context;
    Audio_Format audio_format = wasapi_context->format;
    Audio_Thread_Context *audio_context = wasapi_context->audio_context;
    
    bool still_playing = true;
    HANDLE wait_array[2] = {wasapi_context->shutdown_event, wasapi_context->audio_sample_ready_event};
    
    CoInitializeEx(0, COINIT_MULTITHREADED);
    
    u64 samples_left_user_buffer = 0;
    
    while(still_playing)
    {
        
        //~
        // soundcard polling
        
        real32 *system_buffer;
        u32 total_samples_to_render;
        
        DWORD wait_result = WaitForMultipleObjects(2, wait_array, FALSE, INFINITE);
        
        if(wait_result == WAIT_OBJECT_0 + 0)
            still_playing = false;
        
        if(wait_result == WAIT_OBJECT_0 + 0 || wait_result == WAIT_OBJECT_0 + 1)
        {
            u32 padding;
            u32 system_allocated_samples;
            
            wasapi_context->audio_client->GetCurrentPadding(&padding);
            wasapi_context->audio_client->GetBufferSize(&system_allocated_samples);
            ensure(wasapi_context->system_num_allocated_samples == system_allocated_samples);
            
            total_samples_to_render = system_allocated_samples - padding;
            u32 total_samples_user_buffer = audio_format.num_samples;
            
            HRESULT hr = wasapi_context->render_client->GetBuffer(total_samples_to_render,(BYTE**) &system_buffer);
            ensure(hr != AUDCLNT_E_BUFFER_TOO_LARGE);
            
            if(hr == S_OK)
            {
                auto samples_left_to_render = total_samples_to_render ;
                
                if(samples_left_user_buffer != 0)
                {
                    if(samples_left_user_buffer > total_samples_to_render)
                    {
                        interleave(wasapi_context->user_buffer, 
                                   system_buffer, 
                                   audio_format.num_channels,
                                   total_samples_user_buffer - samples_left_user_buffer,
                                   samples_left_to_render);
                        
                        samples_left_user_buffer -= samples_left_to_render;
                        samples_left_to_render = 0;
                    }
                    else 
                    {
                        interleave(wasapi_context->user_buffer, 
                                   system_buffer, 
                                   audio_format.num_channels,
                                   total_samples_user_buffer - samples_left_user_buffer,
                                   samples_left_user_buffer);
                        
                        samples_left_to_render -= samples_left_user_buffer;
                        samples_left_user_buffer = 0;
                        
                    }
                }
                
                
                
                
                while(samples_left_to_render > total_samples_user_buffer)
                {
                    samples_left_user_buffer = total_samples_user_buffer;
                    render_audio(wasapi_context->user_buffer, audio_format, wasapi_context->audio_context);
                    
                    interleave(wasapi_context->user_buffer, 
                               system_buffer + (total_samples_to_render - samples_left_to_render) * audio_format.num_channels, 
                               audio_format.num_channels,
                               0, 
                               total_samples_user_buffer);
                    
                    samples_left_to_render -= total_samples_user_buffer;
                    samples_left_user_buffer = 0;
                }
                
                if(samples_left_to_render > 0)
                {
                    samples_left_user_buffer = total_samples_user_buffer;
                    render_audio(wasapi_context->user_buffer, audio_format, wasapi_context->audio_context);
                    
                    interleave(wasapi_context->user_buffer, 
                               system_buffer + (total_samples_to_render - samples_left_to_render) * audio_format.num_channels, 
                               audio_format.num_channels,
                               0, 
                               samples_left_to_render);
                    
                    samples_left_user_buffer -= samples_left_to_render;
                    samples_left_to_render = 0;
                }
                
            }
            else{
                printf("failed to get buffer\n");
            }
        }
        
        if(still_playing == false)
        {
            
            for(auto i = 0; i < total_samples_to_render; i++)
                system_buffer[i] = system_buffer[i] * (float) i / total_samples_to_render;
        }
        
        if(wait_result == WAIT_OBJECT_0 ||wait_result == WAIT_OBJECT_0 + 1)
            wasapi_context->render_client->ReleaseBuffer(total_samples_to_render, 0);
        
    }
    
    CoUninitialize();
    return S_OK;
}
#undef if_failed

#define if_failed(hr, message) if(FAILED(hr)) { MessageBox(0, message, 0, MB_OK); return false;}

Device_List get_device_list()
{
    HRESULT hr;
    Device_List device_list = {
        .default_input_idx = -1,
        .default_output_idx = -1
    };
    
    IMMDeviceEnumerator *enumerator = get_enumerator();
    
    IMMDevice *default_output_device = get_default_device(enumerator, false);
    IMMDevice *default_input_device = get_default_device(enumerator, true);
    
    LPWSTR default_output_id;
    if(default_output_device) {
        default_output_device->GetId(&default_output_id);
    }
    
    LPWSTR default_input_id;
    if(default_input_device) {
        default_input_device->GetId(&default_input_id);
    }
    
    //input and output count
    {
        IMMDeviceCollection *output_collection;
        hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &output_collection);
        output_collection->GetCount(&device_list.output_count);
        output_collection->Release();
        
        IMMDeviceCollection *input_collection;
        hr = enumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &input_collection);
        input_collection->GetCount(&device_list.input_count);
        input_collection->Release();
    }
    device_list.outputs = m_allocate_array(Audio_Device, device_list.output_count, "output device list");
    device_list.inputs = m_allocate_array(Audio_Device, device_list.input_count, "input device list");
    
    IMMDeviceCollection *collection;
    hr = enumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &collection);
    
    u32 device_count;
    collection->GetCount(&device_count);
    ensure(device_count == (device_list.input_count + device_list.output_count));
    
    u32 input_idx = 0;
    u32 output_idx = 0;
    
    for(u32 i = 0; i < device_count; i++)
    {
        IMMDevice *mm_device;
        collection->Item(i, &mm_device);
        
        Audio_Device *device;
        {
            IMMEndpoint *endpoint;
            mm_device->QueryInterface(IID_IMMEndpoint, (void**)&endpoint);
            EDataFlow data_flow;
            endpoint->GetDataFlow(&data_flow);
            
            if(data_flow == eRender)
            {
                device = &device_list.outputs[output_idx++];
                device->flow = Flow_Output;
            }
            else 
            {
                device = &device_list.inputs[input_idx++];
                device->flow = Flow_Input;
            }
        }
        
        Audio_Device_Wasapi *wasapi_device = m_allocate_array(Audio_Device_Wasapi, 1, "private wasapi device");
        device->wasapi_device = (void*)wasapi_device;
        wasapi_device->mm_device = mm_device;
        
        LPWSTR id;
        mm_device->GetId(&id);
        
        if(default_output_device && wcscmp(default_output_id, id) == 0)
        {
            device_list.default_output_idx = i;
        }
        
        // NOTE(octave): copy id ?
        // NOTE(octave): release id ?
        
        IAudioClient *audio_client;
        mm_device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&audio_client);
        
        {
            REFERENCE_TIME default_device_period;
            REFERENCE_TIME min_device_period;
            audio_client->GetDevicePeriod(&default_device_period, &min_device_period);
            device->default_duration_s = from_reftimes(default_device_period);
            device->min_duration_s = from_reftimes(min_device_period);
        }
        
        IPropertyStore *store;
        mm_device->OpenPropertyStore(STGM_READ, &store);
        
        //name
        {
            PROPVARIANT variant; 
            PropVariantInit(&variant);
            store->GetValue(PKEY_Device_FriendlyName, &variant);
            
            device->name.size = WideCharToMultiByte(CP_ACP, 0, variant.pwszVal, -1, NULL, 0, NULL, NULL);
            device->name.str = m_allocate_array(char, device->name.size, "device name");
            
            WideCharToMultiByte(CP_ACP, 0, variant.pwszVal, -1, device->name.str, device->name.size, NULL, NULL);
            PropVariantClear(&variant);
        }
        store->Release();
        
        {
            auto format_44100 = format_for(44100, 2);
            device->support_44100 = format_check(audio_client, &format_44100);
            
            auto format_48000 = format_for(48000, 2);
            device->support_48000 = format_check(audio_client, &format_48000);
        }
        
        audio_client->Release();
    }
    
    collection->Release();
    enumerator->Release();
    return device_list;
}

bool audio_initialize(void **out_ctx, 
                      Audio_Format *out_format, 
                      Audio_Thread_Context* audio_context,
                      Arena *app_allocator,
                      Audio_Device *output_device)
{
    ensure(output_device->flow == Flow_Output);
    HRESULT hr;
    auto *ctx = (WasapiContext*) arena_allocate(app_allocator, sizeof(WasapiContext));
    *ctx = {};
    
    CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    
    ctx->audio_sample_ready_event = CreateEventEx(0,0,0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    ctx->shutdown_event = CreateEventEx(0,0,0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    
    Audio_Device_Wasapi *output_device_wasapi = (Audio_Device_Wasapi*) output_device->wasapi_device;
    
    hr = output_device_wasapi->mm_device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&ctx->audio_client);
    if_failed(hr, "failed to get audio client");
    
    //~ Format
    
    auto format_44100 = format_for(44100, 2);
    auto format_48000 = format_for(48000, 2);
    
    WAVEFORMATEX used_format;
    if(output_device->support_48000)
    {
        used_format = format_48000;
    }
    else if(output_device->support_44100)
    {
        used_format = format_44100;
    }
    else 
    {
        assert(false);
    }
    
    Audio_Format format = {
        .sample_rate = (real32)used_format.nSamplesPerSec,
        .num_channels = used_format.nChannels,
        .num_samples = 0, //later
        .bit_depth = used_format.wBitsPerSample
    };
    
    u64 device_period_num_samples_default  = u64(output_device->default_duration_s * format.sample_rate);
    u32 user_buffer_num_samples = next_power_of_two(device_period_num_samples_default); 
    
    hr = ctx->audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 
                                       AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 
                                       to_reftimes(output_device->default_duration_s),
                                       0,
                                       (WAVEFORMATEX*)&used_format, 
                                       0);
    if_failed(hr, "failed to initialize client");
    
    hr = ctx->audio_client->SetEventHandle(ctx->audio_sample_ready_event);
    if_failed(hr ,"failed to set device handlee\n");
    
    hr = ctx->audio_client->Start();
    if_failed(hr ,"failed to start audio client\n");
    
    hr = ctx->audio_client->GetService(IID_IAudioRenderClient, (void **) &ctx->render_client);
    if_failed(hr, "failed to get render client\n");
    
    
    //~ Buffer Sizes
    hr = ctx->audio_client->GetBufferSize(&ctx->system_num_allocated_samples);
    
    UINT32 padding;
    hr = ctx->audio_client->GetCurrentPadding(&padding);
    if_failed(hr, "failed to get padding\n");
    
    
    //~ User Buffer
    u64 reservoir_byte_size = (user_buffer_num_samples * used_format.nBlockAlign * format.num_channels);
    
    void* reservoir_buffer_base = m_allocate(reservoir_byte_size, "reservoir");
    
    real32** channel_indexer = m_allocate_array(real32*, format.num_channels, "channel");
    for(u64 i = 0; i < format.num_channels; ++i)
    {
        channel_indexer[i] = (real32*)((u64)reservoir_buffer_base + user_buffer_num_samples * used_format.nBlockAlign * i); 
    }
    ctx->user_buffer = {channel_indexer};
    
    //~ Fill first callback with silence
    
    BYTE *data; 
    hr = ctx->render_client->GetBuffer(ctx->system_num_allocated_samples - padding, &data);
    if_failed(hr, "failed to get initial buffer\n");
    hr = ctx->render_client->ReleaseBuffer(ctx->system_num_allocated_samples - padding, AUDCLNT_BUFFERFLAGS_SILENT);
    
    format.num_samples = user_buffer_num_samples;
    ctx->format = format;
    ctx->audio_context = audio_context;
    *out_format = format;
    
    //~ Thread
    ctx->audio_thread = CreateThread(0,0, audio_thread_fn, (void*)ctx, 0, 0);
    
    *out_ctx = ctx;
    return true;
}
#undef if_failed

void audio_uninitialize(void *ctx)
{
    
    WasapiContext *audio_context = (WasapiContext*)ctx;
    SetEvent(audio_context->shutdown_event); 
    WaitForSingleObject(audio_context->audio_thread, INFINITE); //on attend sa rÃ©ponse
    audio_context->audio_client->Stop();
    CloseHandle(audio_context->audio_client);
    
    //delete audio_context;
}
