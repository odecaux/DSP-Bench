/* date = February 1st 2022 2:35 pm */

#ifndef FONT_H
#define FONT_H


#define STB_RECT_PACK_IMPLEMENTATION 1
#include "include/stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION 1 
#include "include/stb_truetype.h"

//TODO padding -> filtering ????
//NOTE hardcoded : unicode range, font size, 
internal Font load_fonts(const char* font_filename, 
                         Arena *app_allocator, 
                         Arena *scratch_allocator)
{
    char *current_allocator_position = arena_current(scratch_allocator);
    Font new_font = {};
    
    i64 font_file_size = file_get_size(font_filename);
    if(font_file_size <= 0)
    {
        exit(1);
    }
    
    u8* font_file_buffer = (u8*) arena_allocate(scratch_allocator, font_file_size);
    if(load_file_to_memory(font_filename, font_file_buffer) == false)
    {
        exit(1);
    }
    
    //1) check that the file is usable
    
    stbtt_fontinfo font_info = {};
    
    const int font_offset = stbtt_GetFontOffsetForIndex(font_file_buffer, /* TODO autre font dans le fichier ?*/ 0);
    
    ensure(font_offset >= 0 && "FontData is incorrect, or FontNo cannot be found.");
    if (!stbtt_InitFont(&font_info, font_file_buffer, font_offset))
    {
        printf("couldn't initialize stbtt\n");
        exit(0);
    }
    
    
    //2) register glyphs
    
    u32 min_codepoint = 0x0020;
    u32 max_codepoint = 0x00FF;
    i32 glyph_count = 0;
    
    i32 *codepoint_to_idx = (i32*)arena_allocate(app_allocator, sizeof(i32) * max_codepoint);
    memset(codepoint_to_idx, -1, max_codepoint * sizeof(i32));
    
    i32 *codepoint_list = (i32*) arena_allocate(scratch_allocator, sizeof(i32) * max_codepoint);
    
    for(u32 codepoint = min_codepoint; codepoint < max_codepoint; codepoint++)
    {
        if (!stbtt_FindGlyphIndex(&font_info, codepoint))    // It is actually in the font?
            continue;
        codepoint_to_idx[codepoint] = glyph_count;
        codepoint_list[glyph_count] = codepoint;
        glyph_count++;
    }
    
    if(glyph_count == 0)
    {
        printf("empty font ???\n");
        exit(0);
    }
    
    //3) on chope la taille de chaque glyph
    stbrp_rect *glyph_rects = (stbrp_rect*)arena_allocate(scratch_allocator, sizeof(stbrp_rect) * glyph_count);
    memset(glyph_rects, 0, glyph_count * sizeof(stbrp_rect));
    
    
    float PIXEL_SIZE = 22.0f;
    u8 oversample_h = 4;
    u8 oversample_v = 4;
    const float scale = stbtt_ScaleForPixelHeight(&font_info, PIXEL_SIZE);
    const int padding = 4;
    
    int total_surface = 0;
    
    for (int glyph_i = 0; glyph_i < glyph_count; glyph_i++)
    {
        int x0, y0, x1, y1;
        const int glyph_index_in_font = stbtt_FindGlyphIndex(&font_info, codepoint_list[glyph_i]);
        
        ensure(glyph_index_in_font != 0);
        stbtt_GetGlyphBitmapBoxSubpixel(&font_info, glyph_index_in_font, scale * oversample_h, scale * oversample_v, 0, 0, &x0, &y0, &x1, &y1);
        glyph_rects[glyph_i].w = (stbrp_coord)(x1 - x0 + padding + oversample_h - 1);
        glyph_rects[glyph_i].h = (stbrp_coord)(y1 - y0 + padding + oversample_v - 1);
        total_surface += glyph_rects[glyph_i].w * glyph_rects[glyph_i].h;
    }
    
    const int surface_sqrt = (int)sqrt((float)total_surface) + 1;
    int texture_width = (surface_sqrt >= 4096 * 0.7f) ? 4096 : (surface_sqrt >= 2048 * 0.7f) ? 2048 : (surface_sqrt >= 1024 * 0.7f) ? 1024 : 512;
    
    const int TEX_HEIGHT_MAX = 1024 * 32;
    
    //4) on pack
    stbtt_pack_context spc = {};
    stbtt_PackBegin(&spc, /* pixels */0, texture_width, TEX_HEIGHT_MAX, /*stride*/ 0, padding, /* malloc ctx*/ 0);
    
    stbrp_rect white_rect;
    white_rect.h = 2;
    white_rect.w = 2;
    stbrp_pack_rects((stbrp_context*)spc.pack_info, &white_rect, 1);
    
    stbrp_pack_rects((stbrp_context*)spc.pack_info, glyph_rects, glyph_count);
    
    //on a la hauteur finale
    int texture_height = 0;
    for (int glyph_i = 0; glyph_i < glyph_count; glyph_i++)
        if (glyph_rects[glyph_i].was_packed)
        texture_height = octave_max(texture_height, glyph_rects[glyph_i].y + glyph_rects[glyph_i].h);
    
    
    //texture_height = (Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (texture_height + 1) : ImUpperPowerOfTwo(texture_height);
    ///TODO c'est quoi cette histoire de power of two ?
    
    Vec2 uv_scale = {
        1.0f / texture_width, 
        1.0f / texture_height
    };
    
    //printf("texture size : %d, %d\n", texture_width, texture_height);
    //5) FINALLY RENDER TO A GRAYSCALE BITMAP
    
    u8 *grayscale_pixels = (u8*)arena_allocate(scratch_allocator, texture_width * texture_height); 
    memset(grayscale_pixels, 0, sizeof(u8) * texture_width * texture_height);
    
    spc.pixels = grayscale_pixels;
    spc.height = texture_height;
    
    
    stbtt_packedchar *packedchars = (stbtt_packedchar*)arena_allocate(scratch_allocator, sizeof(stbtt_packedchar) * glyph_count);
    memset(packedchars, 0, glyph_count * sizeof(stbtt_packedchar));
    
    stbtt_pack_range pack_range = {
        PIXEL_SIZE,
        0,
        codepoint_list,
        glyph_count,
        packedchars, //output
        oversample_h, oversample_v //TODO dans la doc ça dit "don't set these"
    }; 
    
    
    stbtt_PackFontRangesRenderIntoRects(&spc, &font_info, &pack_range, 1, glyph_rects);
    
    // End packing
    stbtt_PackEnd(&spc);
    
    const float font_scale = stbtt_ScaleForPixelHeight(&font_info, PIXEL_SIZE);
    int unscaled_ascent, unscaled_descent, unscaled_line_gap;
    stbtt_GetFontVMetrics(&font_info, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);
    
    const float ascent = (real32)floor(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
    const float descent = (real32)floor(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
    
    const float font_off_x = 0;//= cfg.GlyphOffset.x;
    const float font_off_y = 0; //cfg.GlyphOffset.y + IM_ROUND(dst_font->Ascent);
    
    Glyph* glyphs = (Glyph*)arena_allocate(app_allocator, sizeof(Glyph) * glyph_count);
    
    for (int glyph_i = 0; glyph_i < glyph_count; glyph_i++)
    {
        // Register glyph
        const int codepoint = codepoint_list[glyph_i];
        const stbtt_packedchar& pc = packedchars[glyph_i];
        stbtt_aligned_quad q;
        float unused_x = 0.0f, unused_y = 0.0f;
        stbtt_GetPackedQuad(packedchars, 
                            texture_width, texture_height, 
                            glyph_i, 
                            &unused_x, &unused_y, 
                            &q, 0);
        
        /*
        // Clamp & recenter if needed
        const float advance_x_original = advance_x;
        advance_x = ImClamp(advance_x, cfg->GlyphMinAdvanceX, cfg->GlyphMaxAdvanceX);
        if (advance_x != advance_x_original)
        {
            float char_off_x = (advance_x - advance_x_original) * 0.5f;
            x0 += char_off_x;
            x1 += char_off_x;
        }
        */
        
        Glyph& glyph = glyphs[glyph_i];
        glyph.codepoint = (u32)codepoint;
        glyph.X0 = q.x0 + font_off_x;
        glyph.Y0 = q.y0 + font_off_y;
        glyph.X1 = q.x1 + font_off_x;
        glyph.Y1 = q.y1 + font_off_y;
        glyph.U0 = q.s0;
        glyph.V0 = q.t0;
        glyph.U1 = q.s1;
        glyph.V1 = q.t1;
        glyph.advance_x = pc.xadvance;
        
        for(u16 x = 0; x < pc.x1 - pc.x0; x++)
        {
            for(u16 y = 0; y < (pc.y1 - pc.y0) / 2 ; y++)
            {
                u32 a_x = pc.x0 + x;
                u32 a_y = pc.y0 + y;
                u32 a_idx = a_x + texture_width * a_y;
                u32 b_x = pc.x0 + x;
                u32 b_y = pc.y1 - y - 1;
                u32 b_idx = b_x + texture_width * b_y;
                
                if(a_idx >= texture_width * texture_height)
                {
                    ensure(false && "a_idx");
                }
                
                if(b_idx >= texture_width * texture_height)
                {
                    ensure(false && "b_idx");
                }
                u8 swap = grayscale_pixels[a_idx];
                grayscale_pixels[a_idx] = grayscale_pixels[b_idx];
                grayscale_pixels[b_idx] = swap;
            }
        }
    }
    
    
    
    
    
    real32 *codepoint_to_advancex = (real32*)arena_allocate(app_allocator, sizeof(real32) *( max_codepoint + 1 ));
    memset(codepoint_to_advancex, -1.0f, (max_codepoint + 1) * sizeof(real32));
    
    for (int i = 0; i < glyph_count; i++)
    {
        int codepoint = (int) glyphs[i].codepoint;
        codepoint_to_advancex[codepoint] = glyphs[i].advance_x;
    }
    
    
    // Render 4 white pixels
    const int offset = (int)white_rect.x + (int)white_rect.y * texture_width;
    grayscale_pixels[offset] = 
        grayscale_pixels[offset + 1] = 
        grayscale_pixels[offset + texture_width] = 
        grayscale_pixels[offset + texture_width + 1] = 
        255;
    
    //Vec2 white_rect_pos = Vec2{ (white_rect.x + 0.5f) * uv_scale.x, (white_rect.y + 0.5f) * uv_scale.y};
    
    u32 *rgba_pixels = (u32*) arena_allocate(app_allocator, sizeof(u32) * texture_width * texture_height);
    
    
    for(u32 i = 0; i < texture_width * texture_height; i++)
    {
        u8 val = grayscale_pixels[i];
        rgba_pixels[i] = (u32)val << 24 | (u32)255 << 16 | (u32)255 << 8 | (u32)255;
    }
    
    
    arena_reset(scratch_allocator, current_allocator_position);
    return {
        .font_size = PIXEL_SIZE,
        .codepoint_to_idx = codepoint_to_idx,
        .codepoint_to_advancex = codepoint_to_advancex,
        .highest_codepoint = max_codepoint,
        .glyphs = glyphs,
        .glyph_count = (u32)glyph_count, //TODO ono
        //.white_rect_pos = white_rect_pos,
        .ascent = ascent,
        .descent = descent,
        .atlas_pixels = rgba_pixels, 
        .atlas_texture_dim = Vec2{ (real32)texture_width, (real32)texture_height },
        .atlas_texture_id = 0
    };
}




#endif //FONT_H
