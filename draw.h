/* date = February 11th 2022 4:23 pm */

#ifndef DRAW_H
#define DRAW_H

u32 push_vtx(Vec2 pos, Color col, Graphics_Context_Atlas *graphics_ctx)
{
    Vertex* vtx_write = graphics_ctx->draw_vertices;
    u32 vtx_idx = graphics_ctx->draw_vertices_count++;
    
	vtx_write[vtx_idx].pos = pos;
    vtx_write[vtx_idx].col = col;
	vtx_write[vtx_idx].uv = graphics_ctx->font.white_rect_pos;
    return vtx_idx;
}

void push_idx(u32 vtx_idx, Graphics_Context_Atlas *graphics_ctx)
{
    graphics_ctx->draw_indices[graphics_ctx->draw_indices_count++] = vtx_idx;
}

void draw_character(i32 codepoint, Color col, Rect bounds, Graphics_Context_Atlas *graphics_ctx)
{
    Font* font = &graphics_ctx->font;
    Glyph *glyph = &font->glyphs[font->codepoint_to_idx[codepoint]];
    
    auto top_left = bounds.origin;
    auto top_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y };
    auto bottom_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y + bounds.dim.y};
    auto bottom_left = Vec2{ bounds.origin.x, bounds.origin.y + bounds.dim.y};
    
    Vec2 white_rect_pos = font->white_rect_pos;
    
    u32 vtx_idx = graphics_ctx->draw_vertices_count;
    Vertex *vtx_write = graphics_ctx->draw_vertices + graphics_ctx->draw_vertices_count;
    
    vtx_write[0].pos = bottom_left;
    vtx_write[0].col = col;
	vtx_write[0].uv = Vec2{glyph->U0, glyph->V0};
    
	vtx_write[1].pos = top_right;
    vtx_write[1].col = col;
	vtx_write[1].uv = Vec2{glyph->U1, glyph->V1};
    
	vtx_write[2].pos = top_left;
    vtx_write[2].col = col;
	vtx_write[2].uv = Vec2{glyph->U0, glyph->V1};
    
	vtx_write[3].pos = bottom_right;
    vtx_write[3].col = col;
	vtx_write[3].uv = Vec2{glyph->U1, glyph->V0};
    
    graphics_ctx->draw_vertices_count += 4;
    
    u32 *idx_write = graphics_ctx->draw_indices + graphics_ctx->draw_indices_count;
    
    idx_write[0] = vtx_idx;
    idx_write[1] = vtx_idx + 1;
    idx_write[2] = vtx_idx + 2;
    idx_write[3] = vtx_idx;
    idx_write[4] = vtx_idx + 3;
    idx_write[5] = vtx_idx + 1;
    
    graphics_ctx->draw_indices_count += 6;
}

//TODO c'est de la merde cette api mdr

void draw_line(Vec2 start, Vec2 end, Color col, real32 width, Graphics_Context_Atlas *graphics_ctx)
{
    const Vec2 normal = vec2_normalize(start, end);
	const Vec2 perpendicular = Vec2(normal.y, -normal.x) ;
    const Vec2 hn = vec2_mult_scalar(0.5f, perpendicular);
    
    auto a = vec2_minus(end, hn);
    auto b = vec2_minus(start, hn);
    auto c = vec2_add(start, hn);
    auto d = vec2_add(end, hn);
    
    Vec2 white_rect_pos = graphics_ctx->font.white_rect_pos;
    
    u32 vtx_idx = graphics_ctx->draw_vertices_count;
    Vertex *vtx_write = graphics_ctx->draw_vertices + graphics_ctx->draw_vertices_count;
    
    vtx_write[0].pos = a;
    vtx_write[0].col = col;
	vtx_write[0].uv = white_rect_pos;
    
	vtx_write[1].pos = b;
    vtx_write[1].col = col;
	vtx_write[1].uv = white_rect_pos;
    
	vtx_write[2].pos = c;
    vtx_write[2].col = col;
	vtx_write[2].uv = white_rect_pos;
    
	vtx_write[3].pos = d;
    vtx_write[3].col = col;
	vtx_write[3].uv = white_rect_pos;
    
    graphics_ctx->draw_vertices_count += 4;
    
    u32 *idx_write = graphics_ctx->draw_indices + graphics_ctx->draw_indices_count;
    
    idx_write[0] = vtx_idx;
    idx_write[1] = vtx_idx + 1;
    idx_write[2] = vtx_idx + 2;
    idx_write[3] = vtx_idx;
    idx_write[4] = vtx_idx + 2;
    idx_write[5] = vtx_idx + 3;
    
    graphics_ctx->draw_indices_count += 6;
}

void draw_rectangle(Rect bounds, Color color, Graphics_Context_Atlas *graphics_ctx)
{
    auto top_left = bounds.origin;
    auto top_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y };
    auto bottom_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y + bounds.dim.y};
    auto bottom_left = Vec2{ bounds.origin.x, bounds.origin.y + bounds.dim.y};
    
    draw_line(top_left, top_right, Color_Front, 1.0f, graphics_ctx);
    draw_line(top_left, bottom_left, Color_Front, 1.0f, graphics_ctx);
    draw_line(top_right, bottom_right, Color_Front, 1.0f, graphics_ctx);
    draw_line(bottom_right, bottom_left, Color_Front, 1.0f, graphics_ctx);
}
void fill_rectangle(Rect bounds, Color col, Graphics_Context_Atlas *graphics_ctx)
{
    auto top_left = bounds.origin;
    auto top_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y };
    auto bottom_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y + bounds.dim.y};
    auto bottom_left = Vec2{ bounds.origin.x, bounds.origin.y + bounds.dim.y};
    
    Vec2 white_rect_pos = graphics_ctx->font.white_rect_pos;
    
    u32 vtx_idx = graphics_ctx->draw_vertices_count;
    Vertex *vtx_write = graphics_ctx->draw_vertices + graphics_ctx->draw_vertices_count;
    
    vtx_write[0].pos = bottom_left;
    vtx_write[0].col = col;
	vtx_write[0].uv = white_rect_pos;
    
	vtx_write[1].pos = top_right;
    vtx_write[1].col = col;
	vtx_write[1].uv = white_rect_pos;
    
	vtx_write[2].pos = top_left;
    vtx_write[2].col = col;
	vtx_write[2].uv = white_rect_pos;
    
	vtx_write[3].pos = bottom_right;
    vtx_write[3].col = col;
	vtx_write[3].uv = white_rect_pos;
    
    graphics_ctx->draw_vertices_count += 4;
    
    u32 *idx_write = graphics_ctx->draw_indices + graphics_ctx->draw_indices_count;
    
    idx_write[0] = vtx_idx;
    idx_write[1] = vtx_idx + 1;
    idx_write[2] = vtx_idx + 2;
    idx_write[3] = vtx_idx;
    idx_write[4] = vtx_idx + 3;
    idx_write[5] = vtx_idx + 1;
    
    graphics_ctx->draw_indices_count += 6;
    
}

real32 measure_text_width(String text, Font *font)
{
    real32 width = 0;
    for(char *c = &text.data[0]; c < text.data + text.size; c++)
    {
        width += font->codepoint_to_advancex[*c];
    }
    return width;
}

void draw_text(const String& text, Rect bounds, Color col, Graphics_Context_Atlas *graphics_ctx)
{
    Font *font = &graphics_ctx->font;
    
    real32 text_width = measure_text_width(text, font);
    real32 width_remaining = bounds.dim.x - text_width;
    
    real32 x;
    real32 x_end;
    
    if(width_remaining > 0.0f)
    {
        real32 left_margin  = width_remaining / 2.0f;
        x = bounds.origin.x + left_margin;
        x_end = x + text_width;
    }
    else
    {
        x = bounds.origin.x;
        x_end = bounds.origin.x + bounds.dim.x;
    }
    
    real32 y;
    real32 height_remaining = bounds.dim.y - font->font_size;
    if(height_remaining > 0.0f)
    {
        real32 height_margin = height_remaining / 2.0f;
        y = bounds.origin.y + height_margin + font->ascent;
    }
    else 
    {
        y = bounds.origin.y + font->ascent;
    }
    for(auto i = 0; i < text.size; i++)
    {
        char c = text.str[i];
        real32 advancex = font->codepoint_to_advancex[c];
        if(x + advancex > x_end)
        {
            break;
        }
        
        Glyph *glyph = &font->glyphs[font->codepoint_to_idx[c]];
        
        auto top_left = Vec2{x + glyph->X0, y + glyph->Y0};
        auto top_right = Vec2{x + glyph->X1, y + glyph->Y0};
        auto bottom_right = Vec2{x + glyph->X1, y + glyph->Y1};
        auto bottom_left = Vec2{x + glyph->X0, y + glyph->Y1};
        
        Vec2 white_rect_pos = graphics_ctx->font.white_rect_pos;
        
        u32 vtx_idx = graphics_ctx->draw_vertices_count;
        Vertex *vtx_write = graphics_ctx->draw_vertices + graphics_ctx->draw_vertices_count;
        
        vtx_write[0].pos = bottom_left;
        vtx_write[0].col = col;
        vtx_write[0].uv = Vec2{glyph->U0, glyph->V0};
        
        vtx_write[1].pos = top_right;
        vtx_write[1].col = col;
        vtx_write[1].uv = Vec2{glyph->U1, glyph->V1};
        
        vtx_write[2].pos = top_left;
        vtx_write[2].col = col;
        vtx_write[2].uv = Vec2{glyph->U0, glyph->V1};
        
        vtx_write[3].pos = bottom_right;
        vtx_write[3].col = col;
        vtx_write[3].uv = Vec2{glyph->U1, glyph->V0};
        
        graphics_ctx->draw_vertices_count += 4;
        
        u32 *idx_write = graphics_ctx->draw_indices + graphics_ctx->draw_indices_count;
        
        idx_write[0] = vtx_idx;
        idx_write[1] = vtx_idx + 1;
        idx_write[2] = vtx_idx + 2;
        idx_write[3] = vtx_idx;
        idx_write[4] = vtx_idx + 3;
        idx_write[5] = vtx_idx + 1;
        
        graphics_ctx->draw_indices_count += 6;
        
        x += glyph->advance_x;
    }
}


void draw_slider(Rect slider_bounds, 
                 real32 normalized_value, 
                 Graphics_Context_Atlas *graphics_ctx)
{
    real32 slider_x = slider_bounds.origin.x + normalized_value * slider_bounds.dim.x; 
    Rect slider_rect = {
        Vec2{slider_x, slider_bounds.origin.y},
        Vec2{SLIDER_WIDTH, slider_bounds.dim.y}
    };
    fill_rectangle(slider_rect, Color_Front, graphics_ctx);
}





#endif //DRAW_H
