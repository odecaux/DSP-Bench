/* date = December 1st 2021 6:53 pm */

#ifndef WAV_READER_H
#define WAV_READER_H



typedef struct  WAV_HEADER{
    /*char                RIFF[4];        // RIFF Header      Magic header
    unsigned long       chunk_size;      // RIFF Chunk Size  
    char                WAVE[4];        // WAVE Header      
    char                format_chunk_id[4];         // FMT header       
    unsigned long       format_chunk_size;  // Size of the format_chunk_id chunk                                
    */unsigned short      AudioFormat;    // Audio format 1=CM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADCM 
    unsigned short      num_channels;      // Number of channels 1=Mono 2=Sterio                   
    unsigned long       sample_rate;  // Sampling Frequency in Hz                             
    unsigned long       byteserSec;    // bytes per second 
    unsigned short      blockAlign;     // 2=16-bit mono, 4=16-bit stereo 
    unsigned short      bit_depth;  // Number of bits per sample      
    
}Wav_Format; 



typedef struct{
    char id[4];
    unsigned long size;
    char WAVE[4];
} Riff_Header;


typedef struct{
    char id[4];
    unsigned long size;
} Chunk_Header;

// Function prototypes 
typedef struct{
    Wav_Format header;
    u64 audio_data_length;
    char* data;
} WavData;

internal Riff_Header read_riff_header(HANDLE handle, u64* file_position)
{
    //printf("curret offset : %llu\n", *file_position);
    Riff_Header header = {};
    DWORD bytes_read;
    BOOL result = ReadFile(handle, &header, sizeof(Riff_Header), &bytes_read, NULL);
    
    if(bytes_read != sizeof(Riff_Header))
    {
        printf("error reading riff header\n");
        exit(0);
    }
    *file_position += bytes_read;
    return header;
}


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
    
    //std::cout << "File is                    :" << file_length<< " bytes." << "\n";
    
    
    // Display the sampling Rate form the header
    std::cout << "Sampling Rate              :" << header->sample_rate << "\n";
    std::cout << "Number of bits used        :" << header->bit_depth << "\n";
    std::cout << "Number of channels         :" << header->num_channels << "\n";
    std::cout << "Number of bytes per second :" << header->byteserSec << "\n";
    std::cout << "Audio Format               :" << header->AudioFormat << "\n";
    // Audio format 1=CM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADCM 
    
    
    std::cout << "Block align                :" << header->blockAlign << "\n";
    
    
}

internal WavData windows_load_wav(const char* filename)
{
    
    HANDLE handle = CreateFileA(
                                filename,
                                GENERIC_READ,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL ,
                                0);
    
    /*
    if(file_handle == HFILE_ERROR || opened_info.nErrCode != 0)
    {
        switch(opened_info.nErrCode)
        {
            case 2: {
                printf("file not found\n");
                exit(0);
            }break;
            case 5: {
                printf("access denied\n");
                exit(0);
            }break;
            default: {
                printf("couldn't open file error code : %d\n", opened_info.nErrCode);
                exit(0);
            }break;
        }
    }
    */
    
    if(handle == INVALID_HANDLE_VALUE)
    {
        printf("error opening\n");
        exit(1);
    }
    
    
    
    u64 file_size;
    { 
        LARGE_INTEGER file_size_quad;
        BOOL result = GetFileSizeEx(handle, &file_size_quad);
        file_size = file_size_quad.QuadPart;
        
        if(result == 0)
        {
            printf("couldn't get file size\n");
            exit(0);
        }
    }
    
    char* data = (char*)malloc(file_size); //TODO temporary, might be too much
    if(data == nullptr) exit(-1);
    
    u64 total_bytes_read = 0;
    
    Riff_Header riff_header =  read_riff_header(handle, &total_bytes_read);
    
    if(strncmp(riff_header.id, "RIFF", 4) != 0 || strncmp(riff_header.WAVE, "WAVE", 4) != 0)
    {
        printf("not a RIFF file\n");
        exit(-1);
    }
    
    
    
    Wav_Format format = {};
    
    u64 total_data_size = 0;
    
    
    while(total_bytes_read < file_size)
    {
        
        Chunk_Header chunk_header = read_header(handle, &total_bytes_read);
        if(strncmp(chunk_header.id, "fmt ", 4) == 0)
        {
            DWORD bytes_read;
            BOOL result = ReadFile(handle, &format, sizeof(Wav_Format), &bytes_read, NULL);
            if(bytes_read != sizeof(Wav_Format))
            {
                exit(-1);
            }
            total_bytes_read += bytes_read;
        }
        else if(strncmp(chunk_header.id, "data", 4) == 0)
        {
            DWORD bytes_read;
            BOOL result = ReadFile(handle, data + total_data_size, chunk_header.size, &bytes_read, NULL);
            if(bytes_read != chunk_header.size)
            {
                exit(-1);
            }
            
            total_data_size  +=  bytes_read;
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
            
            if (offset.LowPart == INVALID_SET_FILE_POINTER && GetLastError() 
                != NO_ERROR)
            {
                printf("error seeking\n");
                exit(-1);
            }
            
            total_bytes_read += chunk_header.size;
        }
    }
    
    data = (char*) realloc(data, total_data_size);
    //debug_header(&format);
    
    CloseHandle(handle);
    return {format, total_data_size, data};
    
}

#endif //WAV_READER_H
