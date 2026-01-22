#include "main.h"
#include "io/keyboard.h"
#include "io/mouse.h"

#include "graphics/graphics.h"
#include "graphics/imgui/imgui.h"
#include "graphics/imgui/imgui_impl_sdl3.h"

#include "graphics/vulkan/vulkan.h"

#include "game/beat-dungeon.h"

#include <filesystem>
#include <algorithm>
#include <vector>
#include <cmath>
#include <set>

extern board b;
struct SDL_Window;
extern SDL_Window* window;
extern graphics::vulkan::UniformBufferObject ubo;
extern mouse_t mouse;
extern bool pause_request;
extern bool unpause_request;
extern bool paused;

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

std::string level_selector(){
    std::string retMe;
    ImGui::Begin("Level Selector");
    
    std::size_t max_length = 0;
    for ( const auto& entry : std::filesystem::directory_iterator("levels") ) {
        max_length = std::max(entry.path().filename().string().size(),max_length);
    }
    
    float length = 10*max_length+20;
    
    std::set<std::string> files;
    for ( const auto& entry : std::filesystem::directory_iterator("levels") ) {
        files.insert(entry.path().filename().string());
    }
    
    for ( const auto& str : files ) {
        if(ImGui::Button(str.c_str(),{length,20})) retMe = "levels/"+str;
    }
    
    ImGui::End();
    return retMe;
}

void level_editor(){
    
    
    
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    float aspect_ratio = height/(float)width;
    
    float x0 = b.data.size()/-2.f;
    float y0 = b.data[0].size()/-2.f;
    glm::vec4 origin = ubo.proj*ubo.view*ubo.model*glm::vec4(x0-.5f,y0-.5f,20,1);
    
    float x1 = b.data.size()/2.f;
    float y1 = b.data[0].size()/2.f;
    glm::vec4 max_p = ubo.proj*ubo.view*ubo.model*glm::vec4(x1-.5f,y1-.5f,20,1);
    
    // should probably be deferred to the update function again, but ehh. Not necessary now. May be fine like this forever.
    if(std::string new_level = level_selector();new_level!=""){
        b.load_level(new_level);
    }
    
    ImGui::Begin("Level Editor");
        
        if(!paused&&ImGui::Button("Pause",{200,50}))
            pause_request = true;
        else if(paused&&ImGui::Button("Resume",{200,50}))
            unpause_request = true;
        else if(pause_request||unpause_request) ImGui::Button("##Dummy pause",{200,50});
        
        static int selected_x = 0;
        static int selected_y = 0;
        
        if(b.data.size()>0){
            ImGui::Text("board is %zu by %zu tiles.",b.data.size(),b.data[0].size());
        }
        
        {
        bool do_resize = ImGui::Button("Resize board");
        
        static int B_size[2] = {0,0};
        ImGui::InputInt2("##B_size",B_size);
        
        for(int i = 0; i < 2; i++)
            if(B_size[i]<1) B_size[i] = 1;
        
            if(do_resize){
                b.resize(B_size[0],B_size[1]);
                b.player.x = std::min((uint32_t)B_size[0]-1,b.player.x);
                b.player.y = std::min((uint32_t)B_size[1]-1,b.player.y);
                b.update_wall_borders();
            }
        }
        
        {
        bool do_move = ImGui::Button("Move Player");
        
        static int B_size[2] = {0,0};
        ImGui::InputInt2("##B_size2",B_size);
        
        for(int i = 0; i < 2; i++)
            if(B_size[i]<0) B_size[i] = 0;
        if(B_size[0]>=b.data.size()) B_size[0] = 0;
        if(B_size[1]>=b.data[0].size()) B_size[1] = 0;
        
        if(do_move){b.player.x = B_size[0];b.player.y = B_size[1];}
        }
        
        float m_x = 2*(mouse.mouseX/width-.5f);
        float m_y = 2*(mouse.mouseY/height-.5f);
        
        float o_x = origin.x/origin.w;
        float o_y = origin.y/origin.w;
        
        float M_x = max_p.x/max_p.w;
        float M_y = max_p.y/max_p.w;
        
        int X_block = (int)std::floor(b.data   .size()*(m_x-o_x)/(M_x-o_x));
        int Y_block = (int)std::floor(b.data[0].size()*(m_y-o_y)/(M_y-o_y));
        
        static char filename_buf[32];
        ImGui::Text("Level Name:");
        ImGui::InputText("##level-name",filename_buf,32);
        static bool save_failed = false;
        if(ImGui::Button("Save level")) save_failed = !b.save_level("levels/"+std::string(filename_buf));
        
        if(save_failed) ImGui::Text("Failed to save level.");
        
        if(X_block>=0&&X_block<b.data.size()&&Y_block>=0&&Y_block<b.data[0].size()){
            ImGui::Text("<%d,%d>",X_block,Y_block);
            
            switch(b.data[X_block][Y_block].type){
                case floor_t::pit         : ImGui::Text("type: pit");         break;
                case floor_t::floor       : ImGui::Text("type: floor");       break;
                case floor_t::wall        : ImGui::Text("type: wall");        break;
                case floor_t::plate       : ImGui::Text("type: plate");       break;
                case floor_t::door_open   : ImGui::Text("type: door_open");   break;
                case floor_t::door_closed : ImGui::Text("type: door_closed"); break;
                case floor_t::firepit_on  : ImGui::Text("type: firepit_on");  break;
                case floor_t::firepit_off : ImGui::Text("type: firepit_off"); break;
                case floor_t::exit        : ImGui::Text("type: exit");        break;
            }
        }else{
            ImGui::Text("<,>");
            ImGui::Text("This area is non-editable.");
        }
        
        
        static bool draw = false;
        
        if(!draw&&ImGui::Button("Draw",{200,50}))    draw = true;
        else// the else prevents them both appearing on the frame of transition.
        if( draw&&ImGui::Button("No Draw",{200,50})) draw = false;
        
        static floor_t new_type = floor_t::pit;
        std::string selected_type = to_str(new_type);
        
        ImGui::Text("Selected tile type: %s", selected_type.c_str());
        if(ImGui::Button("pit")) new_type = floor_t::pit;
        if(ImGui::Button("floor")) new_type = floor_t::floor;
        if(ImGui::Button("wall")) new_type = floor_t::wall;
        if(ImGui::Button("plate")) new_type = floor_t::plate;
        if(ImGui::Button("door_open")) new_type = floor_t::door_open;
        if(ImGui::Button("door_closed")) new_type = floor_t::door_closed;
        if(ImGui::Button("firepit_on")) new_type = floor_t::firepit_on;
        if(ImGui::Button("firepit_off")) new_type = floor_t::firepit_off;
        if(ImGui::Button("exit")) new_type = floor_t::exit;
        
        static bool edit_plate = false;
        static int wall_x;
        static int wall_y;
        static bool edit_wall = false;
        static int plate_x;
        static int plate_y;
        
        if(draw&&X_block>=0&&X_block<b.data.size()&&Y_block>=0&&Y_block<b.data[0].size()){
            
            
            
            
            if(mouse.leftDown){
                
                if(b.data[X_block][Y_block].type!=new_type){
                    
                    if(b.data[X_block][Y_block].type==floor_t::plate) delete (plate_t*)b.data[X_block][Y_block].cell_data;
                    if(b.data[X_block][Y_block].type==floor_t::wall ) delete (wall_t* )b.data[X_block][Y_block].cell_data;
                    
                    b.data[X_block][Y_block].type = new_type;
                    
                    if(b.data[X_block][Y_block].type==floor_t::plate) b.data[X_block][Y_block].cell_data = new plate_t{};
                    if(b.data[X_block][Y_block].type==floor_t::wall ) b.data[X_block][Y_block].cell_data = new wall_t{};
                    
                    
                    b.update_wall_borders();
                }
                
            }
            
            if(mouse.rightDown){
                
                edit_plate = b.data[X_block][Y_block].type==floor_t::plate;
                edit_wall  = b.data[X_block][Y_block].type==floor_t::wall;
                
                if(edit_plate){
                    plate_x = X_block;
                    plate_y = Y_block;
                }
                
                if(edit_wall){
                    wall_x = X_block;
                    wall_y = Y_block;
                }
                
            }
            
        }
        
        
    ImGui::End();
    
    if(plate_x>=b.data.size()||plate_y>b.data[0].size()||b.data[plate_x][plate_y].type!=floor_t::plate)
        edit_plate = false;
    
    if(edit_plate&&draw){
    ImGui::Begin("Plate editor");
        
        plate_t& P = *(plate_t*)b.data[plate_x][plate_y].cell_data;
        
        int target_x = P.x;
        int target_y = P.y;
        ImGui::InputInt("target x",&target_x,1,5);
        ImGui::InputInt("target y",&target_y,1,5);
        P.x = target_x;
        P.y = target_y;
        
        int ticks = P.ticks_alive;
        ImGui::InputInt("ticks to live",&ticks,1,5);
        P.ticks_alive = ticks;
        
        int max_ticks = P.max_ticks;
        ImGui::InputInt("max ticks",&max_ticks,1,5);
        P.max_ticks = max_ticks;
        
    ImGui::End();
    }
    
    if(wall_x>=b.data.size()||wall_y>b.data[0].size()||b.data[wall_x][wall_y].type!=floor_t::wall)
        edit_wall = false;
    if(edit_wall&&draw){
    ImGui::Begin("Wall editor");
        wall_t& W = *(wall_t*)b.data[wall_x][wall_y].cell_data;
        
        int surroundings = W.surroundings;
        ImGui::InputInt("Surroundings",&surroundings,1,1);
        W.surroundings = surroundings;
        
    ImGui::End();
    }
}

void draw(){
    graphics::start_frame();
    
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    float aspect_ratio = height/(float)width;
    ubo.model = glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(FOV), width / (float) height, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
    
    std::vector<graphics::vulkan::Vertex> tris = draw_board(b,aspect_ratio);
    
    graphics::vulkan::upload_tris(tris,index_quads(tris));
    
    level_editor();
    
    graphics::end_frame();
}
