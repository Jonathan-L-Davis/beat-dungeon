#include "level-editor.h"

#include "main.h"
#include "engine/io/keyboard.h"
#include "engine/io/mouse.h"

#include "graphics/graphics.h"
#include "graphics/imgui/imgui.h"
#include "graphics/imgui/imgui_impl_sdl3.h"

#include "graphics/vulkan/vulkan.h"

#include "game/beat-dungeon.h"
#include "game/level-editor.h"

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
                case tile_t::pit         : ImGui::Text("type: pit");         break;
                case tile_t::floor       : ImGui::Text("type: floor");       break;
                case tile_t::wall        : ImGui::Text("type: wall");        break;
                case tile_t::plate       : ImGui::Text("type: plate");       break;
                case tile_t::door_open   : ImGui::Text("type: door_open");   break;
                case tile_t::door_closed : ImGui::Text("type: door_closed"); break;
                case tile_t::firepit_on  : ImGui::Text("type: firepit_on");  break;
                case tile_t::firepit_off : ImGui::Text("type: firepit_off"); break;
                case tile_t::exit        : ImGui::Text("type: exit");        break;
            }
        }else{
            ImGui::Text("<,>");
            ImGui::Text("This area is non-editable.");
        }
        
        
        static bool draw = false;
        static bool place_entities = false;
        
        if(!draw&&ImGui::Button("Draw",{200,50})){
            draw = true;
            place_entities = false;
        }else// the else prevents them both appearing on the frame of transition.
        if( draw&&ImGui::Button("No Draw",{200,50})) draw = false;
        
        static tile_t new_type = tile_t::pit;
        std::string selected_type = to_str(new_type);
        
        ImGui::Text("Selected tile type: %s", selected_type.c_str());
        if(ImGui::Button("pit")) new_type = tile_t::pit;
        if(ImGui::Button("floor")) new_type = tile_t::floor;
        if(ImGui::Button("wall")) new_type = tile_t::wall;
        if(ImGui::Button("plate")) new_type = tile_t::plate;
        if(ImGui::Button("door_open")) new_type = tile_t::door_open;
        if(ImGui::Button("door_closed")) new_type = tile_t::door_closed;
        if(ImGui::Button("firepit_on")) new_type = tile_t::firepit_on;
        if(ImGui::Button("firepit_off")) new_type = tile_t::firepit_off;
        if(ImGui::Button("exit")) new_type = tile_t::exit;
        
        static bool edit_plate = false;
        static int plate_x;
        static int plate_y;
        static bool edit_door = false;
        static int door_x;
        static int door_y;
        static bool edit_wall = false;
        static int wall_x;
        static int wall_y;
        
        if(draw&&X_block>=0&&X_block<b.data.size()&&Y_block>=0&&Y_block<b.data[0].size()){
            
            
            
            
            if(mouse.leftDown){
                
                if(b.data[X_block][Y_block].type!=new_type){
                    
                    //release old cell data.
                    switch(b.data[X_block][Y_block].type){
                        case tile_t::pit        : delete (pit_t    *)b.data[X_block][Y_block].cell_data; break;
                        case tile_t::plate      : delete (plate_t  *)b.data[X_block][Y_block].cell_data; break;
                        case tile_t::floor      : delete (floor_t  *)b.data[X_block][Y_block].cell_data; break;
                        case tile_t::wall       : delete (wall_t   *)b.data[X_block][Y_block].cell_data; break;
                        case tile_t::door_open  : delete (door_t   *)b.data[X_block][Y_block].cell_data; break;
                        case tile_t::door_closed: delete (door_t   *)b.data[X_block][Y_block].cell_data; break;
                        case tile_t::firepit_on : delete (firepit_t*)b.data[X_block][Y_block].cell_data; break;
                        case tile_t::firepit_off: delete (firepit_t*)b.data[X_block][Y_block].cell_data; break;
                        case tile_t::exit       : delete (exit_t   *)b.data[X_block][Y_block].cell_data; break;
                    }
                    b.data[X_block][Y_block].type = new_type;
                    
                    // initialize new cell data.
                    switch(b.data[X_block][Y_block].type){
                        case tile_t::pit:{
                            plate_t* new_plate = new plate_t{};
                            b.data[X_block][Y_block].cell_data = new_plate;
                        }break;
                        case tile_t::plate:{
                            plate_t* new_plate = new plate_t{};
                            b.data[X_block][Y_block].cell_data = new_plate;
                        }break;
                        case tile_t::floor:{
                            floor_t* new_floor = new floor_t{};
                            new_floor->variation = rand()%4;
                            b.data[X_block][Y_block].cell_data = new_floor;
                        }break;
                        case tile_t::wall:{
                            b.data[X_block][Y_block].cell_data = new wall_t{};
                        }break;
                        case tile_t::door_open:{
                            b.data[X_block][Y_block].cell_data = new door_t{};
                        }break;
                        case tile_t::door_closed:{
                            b.data[X_block][Y_block].cell_data = new door_t{};
                        }break;
                        case tile_t::firepit_off:
                        case tile_t::firepit_on:{
                            firepit_t* new_firepit = new firepit_t{};
                            new_firepit->variation = rand()%4;
                            b.data[X_block][Y_block].cell_data = new_firepit;
                        }break;
                        case tile_t::exit:{
                            firepit_t* new_firepit = new firepit_t{};
                            new_firepit->variation = rand()%4;
                            b.data[X_block][Y_block].cell_data = new_firepit;
                        }break;
                    }
                    
                    
                    b.update_wall_borders();
                }
                
            }
            
            if(mouse.rightDown){
                
                edit_plate = b.data[X_block][Y_block].type==tile_t::plate;
                edit_door = b.data[X_block][Y_block].type==tile_t::door_open||b.data[X_block][Y_block].type==tile_t::door_closed;
                edit_wall  = b.data[X_block][Y_block].type==tile_t::wall;
                
                if(edit_plate){
                    plate_x = X_block;
                    plate_y = Y_block;
                }
                
                if(edit_door){
                    door_x = X_block;
                    door_y = Y_block;
                }
                
                if(edit_wall){
                    wall_x = X_block;
                    wall_y = Y_block;
                }
                
            }
            
        }
        
        
    ImGui::End();
    
    ImGui::Begin("Entity Manager");
    
    if(!place_entities&&ImGui::Button("Place Entites",{200,50})){
        draw = false;
        place_entities = true;
    }else// the else prevents them both appearing on the frame of transition.
    if( place_entities&&ImGui::Button("No Place Entites",{200,50})) place_entities = false;
    
    int num_saxophones = b.saxophones.size();
    ImGui::Text("Saxophones: %d", num_saxophones);
    static bool place_sax = false;
    static int sax_idx = 0;
    ImGui::InputInt("Sax Index",&sax_idx,1,1);
    if(sax_idx>=b.saxophones.size()) sax_idx = b.saxophones.size()-1;
    if(sax_idx<0) sax_idx = 0;
    if(ImGui::Button("Place saxophone")){
        place_sax = true;
    }
    if(b.saxophones.size()&&ImGui::Button("Delete saxophone")){
        b.saxophones.erase(b.saxophones.begin()+sax_idx);
    }
    
    if(place_entities&&X_block>=0&&X_block<b.data.size()&&Y_block>=0&&Y_block<b.data[0].size()){
        if(place_sax&&mouse.leftDown){
            sax_t new_sax{};
            new_sax.x = X_block;
            new_sax.y = Y_block;
            b.saxophones.push_back(new_sax);
            place_sax = false;
        }
    }
    
    if(b.saxophones.size()>0){
        int sax_n = sax_idx+1;
        ImGui::Text("Edit sax #%d",sax_n);
        
        sax_t& sax = b.saxophones[sax_idx];
        int movement = sax.movement;
        ImGui::InputInt("movement",&movement,1,5);
        movement %= 16;
        sax.movement = movement;
        
        int pos_x = sax.x;
        int pos_y = sax.y;
        ImGui::InputInt("target x",&pos_x,1,5);
        ImGui::InputInt("target y",&pos_y,1,5);
        sax.x = pos_x;
        sax.y = pos_y;
        
    }
    
    ImGui::End();
    
    if(plate_x>=b.data.size()||plate_y>b.data[0].size()||b.data[plate_x][plate_y].type!=tile_t::plate)
        edit_plate = false;
    
    if(edit_plate&&draw){
    ImGui::Begin("Plate editor");
        
        plate_t& P = *(plate_t*)b.data[plate_x][plate_y].cell_data;
        
        int character = P.character;
        ImGui::InputInt("character",&character,1,5);
        // validate char values
        P.character = character;
        
        int ticks = P.ticks_alive;
        ImGui::InputInt("ticks to live",&ticks,1,5);
        P.ticks_alive = ticks;
        
        int max_ticks = P.max_ticks;
        ImGui::InputInt("max ticks",&max_ticks,1,5);
        P.max_ticks = max_ticks;
        
    ImGui::End();
    }
    
    if(edit_door&&draw){
    ImGui::Begin("Door editor");
        
        door_t& D = *(door_t*)b.data[door_x][door_y].cell_data;
        
        int character = D.character;
        ImGui::InputInt("character",&character,1,5);
        if(character< 0) character = 0;
        if(character>26) character = 26;
        D.character = character;
        
    ImGui::End();
    }
    
    if(wall_x>=b.data.size()||wall_y>b.data[0].size()||b.data[wall_x][wall_y].type!=tile_t::wall)
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