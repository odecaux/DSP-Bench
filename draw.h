/* date = February 11th 2022 4:23 pm */

#ifndef DRAW_H
#define DRAW_H

//~ Fonts

#define WHITE_RECT_POS Vec2{ -1.0f, -1.0f }

//TODO types 
typedef struct {
    real32 X0, Y0, X1, Y1;     // Glyph corners
    real32 U0, V0, U1, V1;     // Texture coordinates
    i32 advance_x;
    
    u32 codepoint; 
} Glyph;

typedef struct{
    real32 font_size; 
    
    i32 *codepoint_to_idx;  
    real32 *codepoint_to_advancex; 
    u32 highest_codepoint;
    
    Glyph *glyphs;
    u32 glyph_count;
    
    i32 fallback_glyph;
    
    Vec2 white_rect_pos;
    real32 ascent;
    real32 descent;
    
    u32* atlas_pixels;
    Vec2 atlas_texture_dim;
    u32 atlas_texture_id;
} Font;



typedef struct {
    Vec2 pos;
    Vec2 uv;
    Color col;
} Vertex;

typedef struct {
    Rect clip_rect;
    u32 idx_offset;
    u32 idx_count;
    u32 vertex_offset;
} Draw_Command_Atlas;


typedef struct {
    Rect bounds;
    u32 sample_count;
    real32 *buffer;
} Draw_Command_FFT;

typedef struct {
    Rect bounds;
    u32 sample_count;
    real32 *buffer;
} Draw_Command_IR;

enum Draw_Command_Type{
    Draw_Command_Type_ATLAS,
    Draw_Command_Type_FFT,
    Draw_Command_Type_IR
};

typedef struct {
    Draw_Command_Type type;
    union{
        Draw_Command_Atlas atlas;
        Draw_Command_FFT fft;
        Draw_Command_IR ir;
    };
} Draw_Command;


typedef struct {
    Vertex *draw_vertices;
    u32 draw_vertices_count;
    //TODO on a size mais pas capacity
    
    u32 *draw_indices;
    u32 draw_indices_count;
    
    Draw_Command *draw_commands;
    u32 draw_command_count;
} Draw_Command_List;


/*j'ai besoin d'une stack pour push/pop les clip rects. comme ça je 
*/

typedef struct{
    Vec2 pos;
    Vec2 quad_pos;
} IR_Vertex;

typedef struct {
    Vec2 window_dim;
    Font font;
    Draw_Command_List command_list;
    Draw_Command_List popup_command_list;
    u32 popup_draw_command_count;
    real32 FIXME_zoom_state;
} Graphics_Context;

function void draw_reset(Draw_Command_List *command_list)
{
    command_list->draw_vertices_count = 0;
    command_list->draw_indices_count = 0;
    command_list->draw_command_count = 0;
}

function Rect draw_pull_last_clip(Draw_Command_List *cmd_list)
{
    ensure(cmd_list->draw_command_count > 0);
    Draw_Command *current_cmd = &cmd_list->draw_commands[cmd_list->draw_command_count - 1];
    ensure(current_cmd->type == Draw_Command_Type_ATLAS);
    return current_cmd->atlas.clip_rect;
}

//TODO rename
function void draw_push_atlas_command(Rect clip_rect, Draw_Command_List *cmd_list)
{
    if(cmd_list->draw_command_count > 0)
    {
        Draw_Command *old_cmd = &cmd_list->draw_commands[cmd_list->draw_command_count - 1];
        if(old_cmd->type == Draw_Command_Type_ATLAS && rect_equal(old_cmd->atlas.clip_rect, clip_rect))
            return;
    }
    Draw_Command *new_cmd = &cmd_list->draw_commands[cmd_list->draw_command_count++];
    *new_cmd = {
        .type = Draw_Command_Type_ATLAS,
        .atlas {
            .clip_rect = clip_rect,
            .idx_offset = cmd_list->draw_indices_count,
            .idx_count = 0,
            .vertex_offset = cmd_list->draw_vertices_count
        }
    };
}


function void draw_fft(Rect bounds, Analysis *analysis, Draw_Command_List *cmd_list)
{
    Draw_Command *new_cmd = &cmd_list->draw_commands[cmd_list->draw_command_count++];
    *new_cmd = Draw_Command {
        .type = Draw_Command_Type_FFT,
        .fft = {
            .bounds = bounds,
            .sample_count = analysis->ir_sample_count * 2,
            .buffer = analysis->magnitudes
        }
    };
}

function void draw_ir(Rect bounds, 
                      real32 zoom, 
                      Analysis *analysis, 
                      Draw_Command_List *cmd_list)
{
    ensure(zoom > 0.0f && zoom <= 1.0f);
    Draw_Command *new_cmd = &cmd_list->draw_commands[cmd_list->draw_command_count++];
    *new_cmd = Draw_Command {
        .type = Draw_Command_Type_IR,
        .ir = {
            .bounds = bounds,
            .sample_count = u32((real32)analysis->ir_sample_count * zoom),
            .buffer = analysis->IR_buffer[0]
        }
    };
}

function void draw_line(Vec2 start, Vec2 end, Color col, real32 width, Draw_Command_List *cmd_list)
{
    Draw_Command *current_cmd = &cmd_list->draw_commands[cmd_list->draw_command_count - 1];
    ensure(current_cmd->type == Draw_Command_Type_ATLAS);
    
    const Vec2 normal = vec2_normalize(start, end);
	const Vec2 perpendicular = Vec2(normal.y, -normal.x) ;
    const Vec2 hn = vec2_mult_scalar(width * 0.5f, perpendicular);
    
    auto a = vec2_minus(end, hn);
    auto b = vec2_minus(start, hn);
    auto c = vec2_add(start, hn);
    auto d = vec2_add(end, hn);
    
    u32 vtx_idx = cmd_list->draw_vertices_count;
    Vertex *vtx_write = cmd_list->draw_vertices + cmd_list->draw_vertices_count;
    
    vtx_write[0].pos = a;
    vtx_write[0].col = col;
	vtx_write[0].uv = WHITE_RECT_POS;
    
	vtx_write[1].pos = b;
    vtx_write[1].col = col;
	vtx_write[1].uv = WHITE_RECT_POS;
    
	vtx_write[2].pos = c;
    vtx_write[2].col = col;
	vtx_write[2].uv = WHITE_RECT_POS;
    
	vtx_write[3].pos = d;
    vtx_write[3].col = col;
	vtx_write[3].uv = WHITE_RECT_POS;
    
    cmd_list->draw_vertices_count += 4;
    
    u32 *idx_write = cmd_list->draw_indices + cmd_list->draw_indices_count;
    
    idx_write[0] = vtx_idx;
    idx_write[1] = vtx_idx + 1;
    idx_write[2] = vtx_idx + 2;
    idx_write[3] = vtx_idx;
    idx_write[4] = vtx_idx + 2;
    idx_write[5] = vtx_idx + 3;
    
    cmd_list->draw_indices_count += 6;
    current_cmd->atlas.idx_count += 6;
}

function void draw_rectangle(Rect bounds, real32 width, Color color, Draw_Command_List *cmd_list)
{
    bounds = rect_shrinked(bounds, width / 2, width / 2);
    auto top_left = bounds.origin;
    auto top_right = Vec2{ bounds.x + bounds.w, bounds.y };
    auto bottom_right = Vec2{ bounds.x + bounds.w, bounds.y + bounds.h};
    auto bottom_left = Vec2{ bounds.x, bounds.y + bounds.h};
    
    draw_line(top_left, top_right, color, width, cmd_list);
    draw_line(top_left, bottom_left, color, width, cmd_list);
    draw_line(top_right, bottom_right, color, width, cmd_list);
    draw_line(bottom_right, bottom_left, color, width, cmd_list);
}
function void fill_rectangle(Rect bounds, Color col, Draw_Command_List *cmd_list)
{
    Draw_Command *current_cmd = &cmd_list->draw_commands[cmd_list->draw_command_count - 1];
    ensure(current_cmd->type == Draw_Command_Type_ATLAS);
    
    auto top_left = bounds.origin;
    auto top_right = Vec2{ bounds.x + bounds.w, bounds.y };
    auto bottom_right = Vec2{ bounds.x + bounds.w, bounds.y + bounds.h};
    auto bottom_left = Vec2{ bounds.x, bounds.y + bounds.h};
    
    u32 vtx_idx = cmd_list->draw_vertices_count;
    Vertex *vtx_write = cmd_list->draw_vertices + cmd_list->draw_vertices_count;
    
    vtx_write[0].pos = bottom_left;
    vtx_write[0].col = col;
	vtx_write[0].uv = WHITE_RECT_POS;
    
	vtx_write[1].pos = top_right;
    vtx_write[1].col = col;
	vtx_write[1].uv = WHITE_RECT_POS;
    
	vtx_write[2].pos = top_left;
    vtx_write[2].col = col;
	vtx_write[2].uv = WHITE_RECT_POS;
    
	vtx_write[3].pos = bottom_right;
    vtx_write[3].col = col;
	vtx_write[3].uv = WHITE_RECT_POS;
    
    cmd_list->draw_vertices_count += 4;
    
    u32 *idx_write = cmd_list->draw_indices + cmd_list->draw_indices_count;
    
    idx_write[0] = vtx_idx;
    idx_write[1] = vtx_idx + 1;
    idx_write[2] = vtx_idx + 2;
    idx_write[3] = vtx_idx;
    idx_write[4] = vtx_idx + 3;
    idx_write[5] = vtx_idx + 1;
    
    cmd_list->draw_indices_count += 6;
    current_cmd->atlas.idx_count += 6;
}

function real32 measure_text_width(String text, real32 size, Font *font)
{
    real32 scale_factor = size / font->font_size;
    real32 width = 0;
    for(char *c = &text.data[0]; c < text.data + text.size; c++)
    {
        width += font->codepoint_to_advancex[*c] * scale_factor;
    }
    return width;
}

function void draw_text(const String& text, Rect bounds, Color col, real32 size, Font *font, Draw_Command_List *cmd_list)
{
    Draw_Command *current_cmd = &cmd_list->draw_commands[cmd_list->draw_command_count - 1];
    ensure(current_cmd->type == Draw_Command_Type_ATLAS);
    
    real32 text_width = measure_text_width(text, size, font);
    real32 width_remaining = bounds.w - text_width;
    
    real32 x;
    real32 x_end;
    
    if(width_remaining > 0.0f)
    {
        real32 left_margin  = width_remaining / 2.0f;
        x = bounds.x + left_margin;
        x_end = x + text_width;
    }
    else
    {
        x = bounds.x;
        x_end = bounds.x + bounds.w;
    }
    
    real32 scale_factor = size / font->font_size;
    real32 height_remaining = bounds.h - size;
    
    real32 y;
    if(height_remaining > 0.0f)
    {
        real32 height_margin = height_remaining / 2.0f;
        y = bounds.y + height_margin + font->ascent * scale_factor;
    }
    else 
    {
        y = bounds.y + font->ascent * scale_factor;
    }
    
    for(auto i = 0; i < text.size; i++)
    {
        char c = text.str[i];
        real32 advancex = font->codepoint_to_advancex[c] * scale_factor;
        if(x + advancex > x_end)
        {
            break;
        }
        
        i32 glyph_idx = font->codepoint_to_idx[c];
        Glyph *glyph;
        if(glyph_idx == -1)
        {
            glyph = &font->glyphs[font->fallback_glyph];
        }
        else 
        {
            glyph = &font->glyphs[glyph_idx];
        }
        
        auto left = x + glyph->X0 * scale_factor;
        auto right = x + glyph->X1 * scale_factor;
        auto top = y + glyph->Y0 * scale_factor;
        auto bottom = y + glyph->Y1 * scale_factor;
        
        auto top_left = Vec2{left, top};
        auto top_right = Vec2{right, top};
        auto bottom_right = Vec2{right, bottom};
        auto bottom_left = Vec2{left, bottom };
        
        u32 vtx_idx = cmd_list->draw_vertices_count;
        Vertex *vtx_write = cmd_list->draw_vertices + cmd_list->draw_vertices_count;
        
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
        
        cmd_list->draw_vertices_count += 4;
        
        u32 *idx_write = cmd_list->draw_indices + cmd_list->draw_indices_count;
        
        idx_write[0] = vtx_idx;
        idx_write[1] = vtx_idx + 1;
        idx_write[2] = vtx_idx + 2;
        idx_write[3] = vtx_idx;
        idx_write[4] = vtx_idx + 3;
        idx_write[5] = vtx_idx + 1;
        
        cmd_list->draw_indices_count += 6;
        current_cmd->atlas.idx_count += 6;
        
        x += glyph->advance_x * scale_factor;
    }
}





#endif //DRAW_H
