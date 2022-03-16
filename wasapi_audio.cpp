
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
#include "plugin.h"
#include "audio.h"

const IID IID_IAudioClient  = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

REFERENCE_TIME sound_latency_fps = 60;
REFERENCE_TIME requested_sound_duration = 2*100;

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

u64 reftimes_to_num_samples(const REFERENCE_TIME t, const float sample_rate)
{
    return u64(sample_rate * float(t) * 0.0000001 + 0.5f);
}

REFERENCE_TIME num_samples_to_reftime(u64 num_samples, float sample_rate)
{
    return REFERENCE_TIME(num_samples * 10000.0f * 1000.0f / sample_rate + 0.5f);
}


struct WasapiContext{
    HANDLE audio_sample_ready_event;
    HANDLE shutdown_event;
    
    IMMDeviceEnumerator *enumerator;
    IMMDevice *output_device;
    IAudioClient *audio_client;
    IAudioRenderClient *render_client;
    
    UINT32 system_num_allocated_samples;
    HANDLE audio_thread;
    
    real32** user_buffer;
    
    Audio_Parameters param;
    Audio_Thread_Context *audio_context;
};

DWORD audio_thread_fn(LPVOID Context)
{
    WasapiContext *wasapi_context = (WasapiContext*)Context;
    Audio_Parameters audio_parameters = wasapi_context->param;
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
            octave_assert(wasapi_context->system_num_allocated_samples == system_allocated_samples);
            
            total_samples_to_render = system_allocated_samples - padding;
            u32 total_samples_user_buffer = audio_parameters.num_samples;
            
            HRESULT hr = wasapi_context->render_client->GetBuffer(total_samples_to_render,(BYTE**) &system_buffer);
            octave_assert(hr != AUDCLNT_E_BUFFER_TOO_LARGE);
            
            if(hr == S_OK)
            {
                auto samples_left_to_render = total_samples_to_render ;
                
                if(samples_left_user_buffer != 0)
                {
                    if(samples_left_user_buffer > total_samples_to_render)
                    {
                        interleave(wasapi_context->user_buffer, 
                                   system_buffer, 
                                   audio_parameters.num_channels,
                                   total_samples_user_buffer - samples_left_user_buffer,
                                   samples_left_to_render);
                        
                        samples_left_user_buffer -= samples_left_to_render;
                        samples_left_to_render = 0;
                    }
                    else 
                    {
                        interleave(wasapi_context->user_buffer, 
                                   system_buffer, 
                                   audio_parameters.num_channels,
                                   total_samples_user_buffer - samples_left_user_buffer,
                                   samples_left_user_buffer);
                        
                        samples_left_to_render -= samples_left_user_buffer;
                        samples_left_user_buffer = 0;
                        
                    }
                }
                
                
                
                
                while(samples_left_to_render > total_samples_user_buffer)
                {
                    samples_left_user_buffer = total_samples_user_buffer;
                    render_audio(wasapi_context->user_buffer, audio_parameters, wasapi_context->audio_context);
                    
                    interleave(wasapi_context->user_buffer, 
                               system_buffer + (total_samples_to_render - samples_left_to_render) * audio_parameters.num_channels, 
                               audio_parameters.num_channels,
                               0, 
                               total_samples_user_buffer);
                    
                    samples_left_to_render -= total_samples_user_buffer;
                    samples_left_user_buffer = 0;
                }
                
                if(samples_left_to_render > 0)
                {
                    samples_left_user_buffer = total_samples_user_buffer;
                    render_audio(wasapi_context->user_buffer, audio_parameters, wasapi_context->audio_context);
                    
                    interleave(wasapi_context->user_buffer, 
                               system_buffer + (total_samples_to_render - samples_left_to_render) * audio_parameters.num_channels, 
                               audio_parameters.num_channels,
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

#define if_failed(hr, message) if(FAILED(hr)) { MessageBox(0, message, 0, MB_OK); return false;}


bool audio_initialize(void **out_ctx, 
                      Audio_Parameters *out_parameters, 
                      Audio_Thread_Context* audio_context)
{
    auto *ctx = (WasapiContext*) m_allocate(sizeof(WasapiContext), "Wasapi Context");
    
    CoInitializeEx(NULL, COINIT_MULTITHREADED); 
    
    ctx->audio_sample_ready_event = CreateEventEx(0,0,0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    ctx->shutdown_event = CreateEventEx(0,0,0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    
    
    auto hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator,
                               (void**)&ctx->enumerator);
    
    if_failed(hr, "failed to create instance\n");
    
    hr = ctx->enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &ctx->output_device);
    if_failed(hr ,"failed to get default device\n");
    
    
    hr = ctx->output_device->Activate(IID_IAudioClient, 
                                      CLSCTX_ALL, 
                                      0, 
                                      (void**) &ctx->audio_client);
    if_failed(hr ,"failed to active audio client\n");
    
    //~ Format
    
    WAVEFORMATEX wave_format_asked{
        . wFormatTag = WAVE_FORMAT_IEEE_FLOAT ,
        . nChannels = 2,
        . nSamplesPerSec = 44100,
        . nAvgBytesPerSec = 2 * sizeof(real32) * 44100,
        . nBlockAlign = 2 * sizeof (real32),
        . wBitsPerSample = sizeof (real32) * 8,
        . cbSize = 0,
    };
    
    WAVEFORMATEX* closest = nullptr;
    hr = ctx->audio_client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &wave_format_asked, &closest);
    if_failed(hr, "unsupported format\n");
    
    WAVEFORMATEXTENSIBLE wave_format_given;
    if(closest == nullptr)
    {
        memcpy (&wave_format_given, &wave_format_asked, sizeof(WAVEFORMATEX));
        //printf("ok\n");
    }
    else
    {
        if(closest->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {
            //printf("extensible\n");
            memcpy (&wave_format_given, closest, sizeof(WAVEFORMATEXTENSIBLE));
        }
        else
        {
            memcpy (&wave_format_given, closest, sizeof(WAVEFORMATEX));
        }
        CoTaskMemFree(closest);
    }
    /*
    printf("\nformat asked :\n");
    print_wave_format(&wave_format_asked);
    
    printf("\nformat given :\n");
    print_wave_format(&wave_format_given.Format);
    */
    Audio_Parameters param;
    
    param.sample_rate = (real32)wave_format_given.Format.nSamplesPerSec;
    param.num_channels = wave_format_given.Format.nChannels;
    param.bit_depth = wave_format_given.Format.wBitsPerSample;
    
    
    //~ eriods
    REFERENCE_TIME default_period, min_period;
    ctx->audio_client->GetDevicePeriod(&default_period, &min_period);
    
    u64 device_period_num_samples_min = reftimes_to_num_samples(min_period, param.sample_rate);
    u64 device_period_num_samples_default  = reftimes_to_num_samples(default_period, param.sample_rate);
    
    //printf("min device period: %llu samples\ndefault device period : %llu samples\n", device_period_num_samples_min, device_period_num_samples_default);
    
    
    //~ Initialisation
    
    hr = ctx->audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 
                                       AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 
                                       default_period,
                                       0,
                                       (WAVEFORMATEX*)&wave_format_given, 
                                       0);
    if(hr != S_OK)
    {
        if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED )  // AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED
        {
            UINT32 num_frames;
            hr = ctx->audio_client->GetBufferSize(&num_frames);
            if(hr == AUDCLNT_E_NOT_INITIALIZED)
                printf("client not initialized when asked for buffer size\n");
            if_failed(hr, "failed to get buffer size again\n");
            ctx->audio_client->Release();
            
            //printf("new buffer size %lu\n", num_frames);
            
            REFERENCE_TIME new_buffer_period = num_samples_to_reftime(num_frames, param.sample_rate);
            
            hr = ctx->output_device->Activate(IID_IAudioClient, 
                                              CLSCTX_ALL, 
                                              0, 
                                              (void**) &ctx->audio_client);
            if_failed(hr, "failed to re-acquire client");
            
            hr = ctx->audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 
                                               AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 
                                               new_buffer_period,
                                               0,
                                               (WAVEFORMATEX*)&wave_format_given, 
                                               0);
            if_failed(hr, "failed to re-initialize client");
            
        }
        else if(hr == AUDCLNT_E_INVALID_DEVICE_PERIOD)
        {
            printf("invalid device period\n"); return false;
        }
        else if(hr == AUDCLNT_E_UNSUPPORTED_FORMAT)
        {
            printf("unsupported format\n"); return false;
        }
        else if(hr == AUDCLNT_E_BUFFER_SIZE_ERROR)
        {
            printf("buffer size error\n"); return false;
        }
        else if(hr == AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL)
        {
            printf("duration et priod no equal\n"); return false;
        }
        else if(hr == E_INVALIDARG)
        {
            printf("invalid arguments\n"); return false;
        }
        else
        {
            printf("other error, sorry\n"); return false;
        }
    }
    
    
    //printf("\n\ninitialized\n");
    
    
    hr = ctx->audio_client->SetEventHandle(ctx->audio_sample_ready_event);
    if_failed(hr ,"failed to set device handlee\n");
    
    hr = ctx->audio_client->Start();
    if_failed(hr ,"failed to start audio client\n");
    
    hr = ctx->audio_client->GetService(IID_IAudioRenderClient, (void **)& ctx->render_client);
    if_failed(hr, "failed to get render client\n");
    
    
    //~ Buffer Sizes
    
    hr = ctx->audio_client->GetBufferSize(&ctx->system_num_allocated_samples);
    
    UINT32 padding;
    hr = ctx->audio_client->GetCurrentPadding(&padding);
    if_failed(hr, "failed to get padding\n");
    
    //printf("system num allocated samples : %d, padding %d\n", ctx->system_num_allocated_samples, padding);
    //printf("system buffer duration : %d ms\n", 1000 * ctx->system_num_allocated_samples / (i32)param.sample_rate);
    
    u32 user_buffer_num_samples = next_power_of_two(device_period_num_samples_default); 
    
    
    //~ User Buffer
    
    u64 reservoir_byte_size = (user_buffer_num_samples * wave_format_given.Format.nBlockAlign * param.num_channels);
    
    void* reservoir_buffer_base = malloc(reservoir_byte_size);
    
    //printf("reservoir num samples : %lu\n", user_buffer_num_samples);
    
    
    real32** channel_indexer = (real32**) malloc(sizeof(real32*) * param.num_channels);
    for(u64 i = 0; i < param.num_channels; ++i)
    {
        channel_indexer[i] = (real32*)((u64)reservoir_buffer_base + user_buffer_num_samples * wave_format_given.Format.nBlockAlign * i); 
    }
    ctx->user_buffer = {channel_indexer};
    
    
    //~ Fill first callback with silence
    
    BYTE *data; 
    hr = ctx->render_client->GetBuffer(ctx->system_num_allocated_samples - padding, &data);
    if_failed(hr, "failed to get initial buffer\n");
    hr = ctx->render_client->ReleaseBuffer(ctx->system_num_allocated_samples - padding, AUDCLNT_BUFFERFLAGS_SILENT);
    
    
    
    param.num_samples = user_buffer_num_samples;
    ctx->param = param;
    ctx->audio_context = audio_context;
    *out_parameters = param;
    
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
