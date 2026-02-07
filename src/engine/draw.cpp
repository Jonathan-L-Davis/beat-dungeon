#include "main.h"
#include "io/keyboard.h"
#include "io/mouse.h"
#include "init.h"

#include "graphics/graphics.h"
#include "graphics/imgui/imgui.h"
#include "graphics/imgui/imgui_impl_sdl3.h"

#include "graphics/vulkan/vulkan.h"

#include "game/beat-dungeon.h"
#include "game/level-editor.h"

#include <filesystem>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <cmath>
#include <set>

extern board b;
struct SDL_Window;
extern SDL_Window* window;
extern graphics::vulkan::UniformBufferObject ubo;

extern gamestate_t gamestate;
extern bool unpause_request;
extern bool pause_request;

extern double load_anim_time;
extern double max_load_anim_time;

float FOV = 45;

std::vector<uint16_t> index_quads(std::vector<graphics::vulkan::Vertex> tris){
    std::vector<uint16_t> retMe;
    
    assert( (tris.size()%4)==0 );
    
    for( int i = 0; i < tris.size()/4; i++ ){
        retMe.push_back(i*4+0);
        retMe.push_back(i*4+1);
        retMe.push_back(i*4+2);
        retMe.push_back(i*4+2);
        retMe.push_back(i*4+3);
        retMe.push_back(i*4+0);
    }
    
    return retMe;
}

void set_uniforms(glm::vec3 pos, glm::vec3 target, glm::vec3 up){
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    float aspect_ratio = height/(float)width;
    ubo.model = glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt( pos, target, up);
    ubo.proj = glm::perspective(glm::radians(FOV), width / (float) height, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
}

extern std::array<glm::vec2,4> get_tex_coords(init_data_t::rect R, float w, float h);

void draw_load_screen(float percent){
    
    ImGui::Begin("Loading");
    ImGui::Text("Time elapsed: %.3f seconds",load_anim_time);
    ImGui::End();
    
    std::vector<graphics::vulkan::Vertex> tris;
    
    const init_data_t::atlas_t& atlas = init_data.atlas;
    
    {
        auto texel = get_tex_coords(atlas.sun,atlas.w,atlas.h);
        
        float x0 =  -15;
        float x1 =   15;
        float y0 =  -15;
        float y1 =   15;
        
        float dX = 0;
        float dY = 1/6.f;
        
        float x = 0, y = 0;
        
        float depth = 50;
        
        glm::vec3 color = {1,1,1};
        
        tris.push_back({{x0+x+dX, y0+y+dY, depth}, color, texel[0]});
        tris.push_back({{x1+x+dX, y0+y+dY, depth}, color, texel[1]});
        tris.push_back({{x1+x+dX, y1+y+dY, depth}, color, texel[2]});
        tris.push_back({{x0+x+dX, y1+y+dY, depth}, color, texel[3]});
    }
    
    {
        auto texel = get_tex_coords(atlas.sand,atlas.w,atlas.h);
        
        float x0 =  -500;
        float x1 =   500;
        float y0 =   48.f/6;
        float y1 =   50;
        
        float dX = 0;
        float dY = 0;
        
        float x = 0, y = 0;
        
        float depth = 50;
        
        glm::vec3 color = {1,1,1};
        
        tris.push_back({{x0+x+dX, y0+y+dY, depth}, color, texel[0]});
        tris.push_back({{x1+x+dX, y0+y+dY, depth}, color, texel[1]});
        tris.push_back({{x1+x+dX, y1+y+dY, depth}, color, texel[2]});
        tris.push_back({{x0+x+dX, y1+y+dY, depth}, color, texel[3]});
    }
    
    {
        float tile_size = 1.75;
        int num_layers = 3;
        int base_width = 16;
        for( int layer = 0; layer < num_layers; layer++ ){
            int layer_width = base_width-layer*3;// 3 is a magic constant based on how we are spacing the tiles/layers.
            
            int layer_height = layer;
            float layer_center = float(layer_width)/2-.5;
            
            {
                auto texel = get_tex_coords(atlas.pyramid_left,atlas.w,atlas.h);
                
                float x0 =  -.5*tile_size;
                float x1 =   .5*tile_size;
                float y0 =  -.5*tile_size;
                float y1 =   .5*tile_size;
                
                float x = (-layer_center)*tile_size, y = 10-layer_height*tile_size;
                
                float depth = 50;
                
                glm::vec3 color = {1,1,1};
                
                tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
                tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
                tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
                tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
            }
            {
                auto texel = get_tex_coords(atlas.shadow_left,atlas.w,atlas.h);
                
                float x0 =  -.5*tile_size;
                float x1 =   .5*tile_size;
                float y0 =   .5*tile_size;
                float y1 =  2.5*tile_size;
                
                float x = (-layer_center)*tile_size, y = 10+layer_height*tile_size*2;
                
                float depth = 50;
                
                glm::vec3 color = {1,1,1};
                
                tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
                tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
                tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
                tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
            }
            for(int i = 1; i < layer_width-1; i++){
                auto texel = get_tex_coords(atlas.pyramid_center,atlas.w,atlas.h);
                
                float x0 =  -.5*tile_size;
                float x1 =   .5*tile_size;
                float y0 =  -.5*tile_size;
                float y1 =   .5*tile_size;
                
                float x = (i-layer_center)*tile_size, y = 10-layer_height*tile_size;
                
                float depth = 50;
                
                glm::vec3 color = {1,1,1};
                
                tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
                tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
                tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
                tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
            {
                auto texel = get_tex_coords(atlas.shadow_center,atlas.w,atlas.h);
                
                float x0 =  -.5*tile_size;
                float x1 =   .5*tile_size;
                float y0 =   .5*tile_size;
                float y1 =  2.5*tile_size;
                
                float x = (i-layer_center)*tile_size, y = 10+layer_height*tile_size*2;
                
                float depth = 50;
                
                glm::vec3 color = {1,1,1};
                
                tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
                tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
                tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
                tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
            }
            }
            {
                auto texel = get_tex_coords(atlas.pyramid_right,atlas.w,atlas.h);
                
                float x0 =  -.5*tile_size;
                float x1 =   .5*tile_size;
                float y0 =  -.5*tile_size;
                float y1 =   .5*tile_size;
                
                float x = (layer_width-1-layer_center)*tile_size, y = 10-layer_height*tile_size;
                
                float depth = 50;
                
                glm::vec3 color = {1,1,1};
                
                tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
                tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
                tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
                tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
            }
            {
                auto texel = get_tex_coords(atlas.shadow_right,atlas.w,atlas.h);
                
                float x0 =  -.5*tile_size;
                float x1 =   .5*tile_size;
                float y0 =   .5*tile_size;
                float y1 =  2.5*tile_size;
                
                float x = (layer_width-1-layer_center)*tile_size, y = 10+layer_height*tile_size*2;
                
                float depth = 50;
                
                glm::vec3 color = {1,1,1};
                
                tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
                tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
                tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
                tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
            }
            
        }
    }
    
    {
        float tile_size = 1.75;
        int num_layers = 2;
        int base_width = 6;
        for( int layer = 0; layer < num_layers; layer++ ){
            int layer_width = base_width-layer*3;// 3 is a magic constant based on how we are spacing the tiles/layers.
            
            int layer_height = layer+4;
            float layer_center = float(layer_width)/2-.5;
            
            {
                auto texel = get_tex_coords(atlas.pyramid_left,atlas.w,atlas.h);
                
                float x0 =  -.5*tile_size;
                float x1 =   .5*tile_size;
                float y0 =  -.5*tile_size;
                float y1 =   .5*tile_size;
                
                float x = (-layer_center)*tile_size, y = 10-layer_height*tile_size;
                
                float depth = 50;
                
                glm::vec3 color = {1,1,1};
                
                tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
                tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
                tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
                tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
            }
            for(int i = 1; i < layer_width-1; i++){
                auto texel = get_tex_coords(atlas.pyramid_center,atlas.w,atlas.h);
                
                float x0 =  -.5*tile_size;
                float x1 =   .5*tile_size;
                float y0 =  -.5*tile_size;
                float y1 =   .5*tile_size;
                
                float x = (i-layer_center)*tile_size, y = 10-layer_height*tile_size;
                
                float depth = 50;
                
                glm::vec3 color = {1,1,1};
                
                tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
                tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
                tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
                tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
            }
            {
                auto texel = get_tex_coords(atlas.pyramid_right,atlas.w,atlas.h);
                
                float x0 =  -.5*tile_size;
                float x1 =   .5*tile_size;
                float y0 =  -.5*tile_size;
                float y1 =   .5*tile_size;
                
                float x = (layer_width-1-layer_center)*tile_size, y = 10-layer_height*tile_size;
                
                float depth = 50;
                
                glm::vec3 color = {1,1,1};
                
                tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
                tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
                tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
                tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
            }
            
        }
        {
            
            auto texel = get_tex_coords(atlas.pyramid_top,atlas.w,atlas.h);
            
            float x0 =  -.5*tile_size;
            float x1 =   .5*tile_size;
            float y0 =  -.5*tile_size;
            float y1 =   .5*tile_size;
            
            float x = 0, y = 10-5*tile_size;
            
            float depth = 50;
            
            glm::vec3 color = {1,1,1};
            
            tris.push_back({{x0+x, y0+y, depth}, color, texel[0]});
            tris.push_back({{x1+x, y0+y, depth}, color, texel[1]});
            tris.push_back({{x1+x, y1+y, depth}, color, texel[2]});
            tris.push_back({{x0+x, y1+y, depth}, color, texel[3]});
        }
    }
    
    graphics::vulkan::upload_tris(tris,index_quads(tris));
    
    set_uniforms( {0.0f, 0.0f, 0*49.f*load_anim_time/max_load_anim_time}, {0.0f, 0.0f, 100.0f}, {0.0f, -1.0f, 0.0f});// this will be updated based on animation time. we will zoom into the pyramid as we "load".
    
}

void draw_main_menu(){
    // these should be buttons done soon.
    
    draw_load_screen(1.0f);
    
    ImGui::Begin("Main Menu");
    if(ImGui::Button("Play game")){
        unpause_request = true;
        gamestate = gamestate_t::level_select;
    }
    if(ImGui::Button("Level Editor")){
        pause_request = true;
        gamestate = gamestate_t::level_editor;
        b.resize(0,0);
        b.resize(5,5);
    }
    
    if(ImGui::Button("Quit game")) quit = true;
    ImGui::End();
}

void render_board(){
    
    if(b.data.size()==0||b.data[0].size()==0)
        return;
    
    set_uniforms( {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f});
    
    std::vector<graphics::vulkan::Vertex> tris = draw_board(b);
    
    graphics::vulkan::upload_tris(tris,index_quads(tris));
}

void draw_pause_menu(){
    
    render_board();
    
    ImGui::Begin("Pause Menu");
    if(ImGui::Button("Resume")) { gamestate = gamestate_t::play; unpause_request = true; }
    if(ImGui::Button("Return to level selection.")) gamestate = gamestate_t::level_select;
    ImGui::End();
}

void draw_level_editor(){
    render_board();
    level_editor();
}

void draw_level_selector(){
    graphics::vulkan::upload_tris({},index_quads({}));
    if(std::string new_level = level_selector(); new_level!=""){
        b.load_level(new_level);
        gamestate = gamestate_t::play;
    }
}

void draw(){
    graphics::start_frame();
    
    
    
    switch(gamestate){
        case gamestate_t::load         : draw_load_screen(load_anim_time/max_load_anim_time);    break;
        case gamestate_t::main_menu    : draw_main_menu();      break;
        case gamestate_t::play         : render_board();        break;
        case gamestate_t::pause_menu   : draw_pause_menu();     break;
        case gamestate_t::level_editor : draw_level_editor();   break;
        case gamestate_t::level_select : draw_level_selector(); break;
    }
    
    graphics::end_frame();
}
