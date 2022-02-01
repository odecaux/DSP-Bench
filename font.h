/* date = February 1st 2022 2:35 pm */

#ifndef FONT_H
#define FONT_H


#define STB_TRUETYPE_IMPLEMENTATION 1 
#include "stb_truetype.h"

/*

#pragma pack(push, 1)
struct FntInfo
{
    signed short	FontSize;
    unsigned char	BitField;		// bit 0: smooth, bit 1: unicode, bit 2: italic, bit 3: bold, bit 4: fixedHeight, bits 5-7: reserved
    unsigned char	CharSet;
    unsigned short	StretchH;
    unsigned char	AA;
    unsigned char	PaddingUp, PaddingRight, PaddingDown, PaddingLeft;
    unsigned char	SpacingHoriz, SpacingVert;
    unsigned char	Outline;
    //char			FontName[];
};

struct FntCommon
{
    unsigned short	LineHeight;
    unsigned short	Base;
    unsigned short	ScaleW, ScaleH;
    unsigned short	Pages;
    unsigned char	BitField;
    unsigned char	Channels[4];
};

struct FntGlyph
{
    unsigned int	Id;
    unsigned short	X, Y;
    unsigned short	Width, Height;
    signed short	XOffset, YOffset;
    signed short	XAdvance;
    unsigned char	Page;
    unsigned char	Channel;
};

struct FntKerning
{
    unsigned int	IdFirst;
    unsigned int	IdSecond;
    signed short	Amount;
};

#pragma pack(pop)

typedef struct {
    unsigned char* file_buffer;
    u64 file_buffer_size;
    
    FntInfo *info;
    FntCommon *common;
    FntGlyph *glyphs;
    u32 glyphs_count;
    FntKerning *kerning;
    u32 kerning_count;
    i32 *index_lookup;
    i32 max_idx;
} Font;
*/

/*
internal Font load_fonts(const char* font_filename)
{
    Font new_font = {};
    HANDLE handle = CreateFileA(font_filename,
                                GENERIC_READ,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL ,
                                0);
    
    if(handle == INVALID_HANDLE_VALUE)
    {
        printf("error opening font file\n");
        exit(1);
    }
    
    LARGE_INTEGER file_size_quad;
    BOOL result = GetFileSizeEx(handle, &file_size_quad);
    u64 file_size = file_size_quad.QuadPart;
    
    unsigned char* font_file_buffer = (unsigned char*) malloc(file_size);
    
    auto success = ReadFile(handle, font_file_buffer, file_size, 0, 0);
    if(success == FALSE)
    {
        printf("error reading font file\n");
        exit(1);
    }
    
    CloseHandle(handle);
    
    if (file_size < 4 
        || font_file_buffer[0] != 'B' 
        || font_file_buffer[1] != 'M' 
        || font_file_buffer[2] != 'F' 
        || font_file_buffer[3] != 0x03)
    {
        printf("wrong header\n");
        exit(1);
    }
	new_font.file_buffer = font_file_buffer;
    new_font.file_buffer_size = file_size;
    
    FntInfo *info = nullptr;
    FntCommon *common = nullptr;
    FntGlyph *glyphs = nullptr;
    u32 glyphs_count = 0;
    FntKerning *kerning = nullptr;
    u32 kerning_count = 0;
    for (const unsigned char* p = font_file_buffer + 4 ; p < font_file_buffer + file_size; )
	{
		const unsigned char block_type = *(unsigned char*)p;
		p += sizeof(unsigned char);
		const u32 block_size = *(u32*)p;
		p += sizeof(u32);
        
		switch (block_type)
		{
            case 1: {
                assert(info == NULL);
                info = (FntInfo*)p;
            } break;
            
            case 2: {
                assert(common == NULL);
                common = (FntCommon*)p;
			}break;
            
            case 3: {
                for (const unsigned char* s = p;
                     s < p + block_size && s < font_file_buffer + file_size;
                     s = s + strlen((const char*)s) + 1)
                    
                    ; //Filenames.push_back((const char*)s);
                
			} break;
            
            case 4: {
                assert(glyphs == NULL && glyphs_count == 0);
                glyphs = (FntGlyph*)p;
                glyphs_count = block_size / sizeof(FntGlyph);
			} break;
            
            default: {
                assert(kerning == NULL && kerning_count == 0);
                kerning = (FntKerning*)p;
                kerning_count = block_size / sizeof(FntKerning);
			} break;
		}
		p += block_size;
	}
    
    new_font.info = info;
    new_font.common = common;
    new_font.glyphs = glyphs;
    new_font.glyphs_count = glyphs_count;
    new_font.kerning = kerning;
    new_font.kerning_count = kerning_count;
    
	ImU32 max_idx = 0;
	for (int i = 0; i != glyphs_count; i++)
		if (max_idx < glyphs[i].Id)
        max_idx = glyphs[i].Id;
    
    
    i32 *index_lookup = (int*) malloc(sizeof(i32) * (max_idx + 1));
    
	for (size_t i = 0; i < index_lookup.size(); i++)
		index_lookup[i] = -1;
	for (size_t i = 0; i < glyphs_count; i++)
		index_lookup[glyphs[i].Id] = (int)i;
    
    new_font.index_lookup = index_lookup;
    new_font.max_idx = max_idx;
    
    return new_font;
    
}
*/

struct Font{
    
};
internal Font load_fonts(const char* font_filename)
{
    Font new_font = {};
    HANDLE handle = CreateFileA(font_filename,
                                GENERIC_READ,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL ,
                                0);
    
    if(handle == INVALID_HANDLE_VALUE)
    {
        printf("error opening font file\n");
        exit(1);
    }
    
    LARGE_INTEGER file_size_quad;
    BOOL result = GetFileSizeEx(handle, &file_size_quad);
    u64 file_size = file_size_quad.QuadPart;
    
    unsigned char* font_file_buffer = (unsigned char*) malloc(file_size);
    
    auto success = ReadFile(handle, font_file_buffer, file_size, 0, 0);
    if(success == FALSE)
    {
        printf("error reading font file\n");
        exit(1);
    }
    
    CloseHandle(handle);
    
    stbtt_fontinfo font;
    if(!stbtt_InitFont(&font, font_file_buffer, stbtt_GetFontOffsetForIndex(font_file_buffer, 0)))
    {
        printf("stbtt failed at parsing font\n");
        exit(1);
    }
    
    int width, height, xoff, yoff;
    u8 *bitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, 20.0f), , &width, &height, &xoff, &yoff);
    
}


#endif //FONT_H
