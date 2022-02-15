/* date = February 11th 2022 4:23 pm */

#ifndef DRAW_H
#define DRAW_H

u32 push_vtx(Vec2 pos, Color col, GraphicsContext *graphics_ctx)
{
    Vertex* vtx_write = graphics_ctx->draw_vertices;
    u32 vtx_idx = graphics_ctx->draw_vertices_count++;
    
	vtx_write[vtx_idx].pos = pos;
    vtx_write[vtx_idx].col = col;
	vtx_write[vtx_idx].uv = graphics_ctx->font->white_rect_pos;
    return vtx_idx;
}

void push_idx(u32 vtx_idx, GraphicsContext *graphics_ctx)
{
    graphics_ctx->draw_indices[graphics_ctx->draw_indices_count++] = vtx_idx;
}


//TODO c'est de la merde cette api mdr

void draw_line(Vec2 start, Vec2 end, Color col, real32 width, GraphicsContext *graphics_ctx)
{
    const Vec2 normal = vec2_normalize(start, end);
	const Vec2 perpendicular = Vec2(normal.y, -normal.x) ;
    const Vec2 hn = vec2_mult_scalar(0.5f, perpendicular);
    
    auto a = vec2_minus(end, hn);
    auto b = vec2_minus(start, hn);
    auto c = vec2_add(start, hn);
    auto d = vec2_add(end, hn);
    
    Vec2 white_rect_pos = graphics_ctx->font->white_rect_pos;
    
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

void draw_rectangle(Rect bounds, Color color, GraphicsContext *graphics_ctx)
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
void fill_rectangle(Rect bounds, Color color, GraphicsContext *graphics_ctx)
{
    auto top_left = bounds.origin;
    auto top_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y };
    auto bottom_right = Vec2{ bounds.origin.x + bounds.dim.x, bounds.origin.y + bounds.dim.y};
    auto bottom_left = Vec2{ bounds.origin.x, bounds.origin.y + bounds.dim.y};
    
}

void draw_text(const String& text, Rect bounds, Color color, GraphicsContext *graphics_ctx)
{
    for(auto i = 0; i < text.size; i++)
    {
        char c = text.str[i];
        
        
    }
}


void draw_slider(Rect slider_bounds, 
                 real32 normalized_value, 
                 GraphicsContext *graphics_ctx)
{
    real32 slider_x = slider_bounds.origin.x + normalized_value * slider_bounds.dim.x; 
    Rect slider_rect = {
        Vec2{slider_x, slider_bounds.origin.y},
        Vec2{5.0f, slider_bounds.dim.y}
    };
    fill_rectangle(slider_rect, Color_Front, graphics_ctx);
    
}



#endif //DRAW_H
