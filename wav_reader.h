/* date = December 1st 2021 6:53 pm */

#ifndef WAV_READER_H
#define WAV_READER_H



typedef struct {                               
    unsigned short      format;    // Audio format 1=CM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADCM 
    unsigned short      num_channels;      // Number of channels 1=Mono 2=Sterio                   
    unsigned long       sample_rate;  // Sampling Frequency in Hz                             
    unsigned long       bytes_per_sec;    // bytes per second 
    unsigned short      block_align;     // 2=16-bit mono, 4=16-bit stereo 
    unsigned short      bit_depth;  // Number of bits per sample      
} Wav_Format; 


typedef struct{
    char id[4];
    unsigned long size;
    char WAVE[4];
} Riff_Header;


typedef struct{
    char id[4];
    unsigned long size;
} Chunk_Header;

internal Chunk_Header read_header(HANDLE handle, u64* file_position)
{
    //printf("curret offset : %llu\n", *file_position);
    Chunk_Header header = {};
    DWORD bytes_read;
    BOOL result = ReadFile(handle, &header, sizeof(Chunk_Header), &bytes_read, NULL);
    
    if(bytes_read != sizeof(Chunk_Header))
    {
        printf("error reading chunk header\n");
        exit(0);
    }
    *file_position += bytes_read;
    return header;
    
}

internal void debug_header(Wav_Format *header)
{
    // Audio format 1=CM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADCM 
    printf("Audio Format               : %hu\n", header->format);
    printf("Number of channels         : %hu\n", header->num_channels);
    printf("Sampling Rate              : %hu\n", header->sample_rate);
    printf("Number of bytes per second : %lu\n", header->bytes_per_sec);
    printf("Block align                : %hu\n", header->block_align);
    printf("Number of bits used        : %lu\n", header->bit_depth);
}

internal Audio_File windows_load_wav(const char* filename)
{
    i64 file_size = file_get_size(filename);
    
    
    
    char* data = (char*)malloc(file_size); //TODO temporary, might be too much
    if(data == nullptr) exit(-1);
    
    HANDLE handle = CreateFileA(filename,
                                GENERIC_READ,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL ,
                                0);
    
    if(handle == INVALID_HANDLE_VALUE)
    {
        return { Wav_Could_Not_Open_File };
    }
    
    
    u64 total_bytes_read = 0;
    
    Riff_Header riff_header = {};
    {
        DWORD bytes_read;
        BOOL result = ReadFile(handle, &riff_header, sizeof(Riff_Header), &bytes_read, NULL);
        
        if(bytes_read != sizeof(Riff_Header))
        {
            free(data);
            return {Wav_File_Reading_Error};
        }
        total_bytes_read += bytes_read;
        
        
        if(strncmp(riff_header.id, "RIFF", 4) != 0 || strncmp(riff_header.WAVE, "WAVE", 4) != 0)
        {
            free(data);
            return {Wav_Not_A_RIFF};
        }
    }
    
    
    Wav_Format format = {};
    
    u64 audio_data_length = 0;
    
    
    while(total_bytes_read < file_size)
    {
        
        Chunk_Header chunk_header = read_header(handle, &total_bytes_read);
        if(strncmp(chunk_header.id, "fmt ", 4) == 0)
        {
            DWORD bytes_read;
            BOOL result = ReadFile(handle, &format, sizeof(Wav_Format), &bytes_read, NULL);
            if(bytes_read != sizeof(Wav_Format))
            {
                free(data);
                return {Wav_File_Reading_Error};
            }
            total_bytes_read += bytes_read;
        }
        else if(strncmp(chunk_header.id, "data", 4) == 0)
        {
            DWORD bytes_read;
            BOOL result = ReadFile(handle, data + audio_data_length, chunk_header.size, &bytes_read, NULL);
            if(bytes_read != chunk_header.size)
            {
                free(data);
                return {Wav_File_Reading_Error};
            }
            
            audio_data_length  +=  bytes_read;
            total_bytes_read += bytes_read;
        }
        else 
        {
            LARGE_INTEGER offset;
            offset.QuadPart = chunk_header.size;
            offset.LowPart = SetFilePointer (handle, 
                                             offset.LowPart, 
                                             &offset.HighPart, 
                                             1);
            
            if (offset.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                free(data);
                return { Wav_File_Reading_Error };
            }
            total_bytes_read += chunk_header.size;
        }
    }
    
    data = (char*) realloc(data, audio_data_length);
    //debug_header(&format);
    
    CloseHandle(handle);
    
    auto num_samples = 8 * audio_data_length / format.bit_depth;
    real32 *float_buffer;
    
    if(format.format == 1)
    {
        float_buffer = m_allocate_array(real32, num_samples); 
        
        if(format.bit_depth == 16)
            convertInt16ToFloat(data, float_buffer, num_samples);
        else if(format.bit_depth == 24)
            convertInt24ToFloat(data, float_buffer, num_samples);
        else if(format.bit_depth == 32)
            convertInt32ToFloat(data, float_buffer, num_samples);
        else
        {
            free(data);
            return {Wav_Invalid_Format};
        }
        free(data);
    }
    else if(format.format == 3)
    {
        float_buffer = (real32*)data;
    }
    else
    {
        free(data);
        return {Wav_Invalid_Format};
    }
    
    u32 num_channels = format.num_channels;
    u32 samples_by_channel = num_samples / num_channels;
    
    real32 **deinterleaved_buffer = m_allocate_array(real32*,num_channels);
    for(u32 channel = 0; channel < num_channels; channel++)
    {
        deinterleaved_buffer[channel] = m_allocate_array(real32, samples_by_channel);
    }
    
    deinterleave(deinterleaved_buffer, float_buffer, samples_by_channel, num_channels);
    free(float_buffer);
    
    return {
        Wav_Success, 
        num_channels,
        samples_by_channel,
        deinterleaved_buffer
    };
}

#endif //WAV_READER_H
