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
#include <cstdint>
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

void entity_manager(int X_block,int Y_block, bool &draw, bool &place_entities){
    ImGui::Begin("Entity Manager");
    
    static bool place_player = false;
    static bool place_sax = false;
    static int sax_idx = 0;
    static bool place_notes = false;
    static int notes_idx = 0;
    static bool place_demon = false;
    static int demon_idx = 0;
    static bool place_fireball = false;
    static int fireball_idx = 0;
    
         if(place_player)   ImGui::Text("Placing Player");
    else if(place_sax)      ImGui::Text("Placing Saxophone");
    else if(place_notes)    ImGui::Text("Placing Notes");
    else if(place_demon)    ImGui::Text("Placing Demon");
    else if(place_fireball) ImGui::Text("Placing Fireball");
    else                    ImGui::Text("Not placing any entities.");
    ImGui::Separator();
    
    if(place_entities&&X_block>=0&&X_block<b.data.size()&&Y_block>=0&&Y_block<b.data[0].size()){
        if(place_player&&mouse.leftDown){
            b.player.x = X_block;
            b.player.y = Y_block;
            place_player=false;
        }
        
        if(place_sax&&mouse.leftDown){
            sax_t new_sax{};
            new_sax.x = X_block;
            new_sax.y = Y_block;
            b.saxophones.push_back(new_sax);
            place_sax = false;
        }
        
        if(place_notes&&mouse.leftDown){
            notes_t new_notes{};
            new_notes.x = X_block;
            new_notes.y = Y_block;
            b.notes.push_back(new_notes);
            place_notes = false;
        }
        
        if(place_demon&&mouse.leftDown){
            demon_t new_demon{};
            new_demon.x = X_block;
            new_demon.y = Y_block;
            b.demons.push_back(new_demon);
            place_demon = false;
        }
        
        if(place_fireball&&mouse.leftDown){
            fireball_t new_fireball{};
            new_fireball.x = X_block;
            new_fireball.y = Y_block;
            b.fireballs.push_back(new_fireball);
            place_fireball = false;
        }
    }
    
    if(ImGui::Button("Place player",{125,25})){
        place_player = !place_player;
        place_sax = false;
        place_notes = false;
        place_demon = false;
        place_fireball = false;
        draw = false;
        place_entities = place_player;
    }
    
    ImGui::Separator();
    
    int num_saxophones = b.saxophones.size();
    ImGui::Text("# Saxophones: %d", num_saxophones);
    if(ImGui::Button("Place saxophone",{125,25})){
        place_player = false;
        place_sax = !place_sax;
        place_notes = false;
        place_demon = false;
        place_fireball = false;
        draw = false;
        place_entities = place_sax;
    }
    
    if(b.saxophones.size()&&ImGui::Button("Delete saxophone",{125,25})){
        b.saxophones.erase(b.saxophones.begin()+sax_idx);
    }
    
    if(b.saxophones.size()>0){
        sax_idx++;
        ImGui::InputInt("Sax Index",&sax_idx,1,1);
        sax_idx--;
        if(sax_idx>=b.saxophones.size()) sax_idx = b.saxophones.size()-1;
        if(sax_idx<0) sax_idx = 0;
        
        int sax_n = sax_idx+1;
        ImGui::Text("Edit sax #%d",sax_n);
        
        sax_t& sax = b.saxophones[sax_idx];
        int movement = sax.movement;
        ImGui::InputInt("movement##sax",&movement,1,5);
        movement %= 16;
        sax.movement = movement;
        
        int pos_x = sax.x;
        int pos_y = sax.y;
        ImGui::InputInt("x position##sax",&pos_x,1,5);
        ImGui::InputInt("y position##sax",&pos_y,1,5);
        sax.x = pos_x;
        sax.y = pos_y;
        
        int shot_cooldown = sax.shot_cooldown;
        ImGui::InputInt("shot_cooldown##sax",&shot_cooldown,1,5);
        sax.shot_cooldown = shot_cooldown;
        
    }
    
    ImGui::Separator();
    
    int num_notes = b.notes.size();
    ImGui::Text("# Notes: %d", num_notes);
    if(ImGui::Button("Place notes",{125,25})){
        place_player = false;
        place_sax = false;
        place_notes = !place_notes;
        place_demon = false;
        place_fireball = false;
        draw = false;
        place_entities = place_notes;
    }
    
    if(b.notes.size()&&ImGui::Button("Delete notes",{125,25})){
        b.notes.erase(b.notes.begin()+notes_idx);
    }
    
    if(b.notes.size()>0){
        notes_idx++;
        ImGui::InputInt("Notes Index",&notes_idx,1,1);
        notes_idx--;
        if(notes_idx>=b.notes.size()) notes_idx = b.notes.size()-1;
        if(notes_idx<0) notes_idx = 0;
        
        int notes_n = notes_idx+1;
        ImGui::Text("Edit notes #%d",notes_n);
        
        notes_t& notes = b.notes[notes_idx];
        int movement = notes.movement;
        ImGui::InputInt("movement##notes",&movement,1,5);
        movement %= 16;
        notes.movement = movement;
        
        int pos_x = notes.x;
        int pos_y = notes.y;
        ImGui::InputInt("x position##notes",&pos_x,1,5);
        ImGui::InputInt("y position##notes",&pos_y,1,5);
        notes.x = pos_x;
        notes.y = pos_y;
        
    }
    
    ImGui::Separator();
    
    int num_demons = b.demons.size();
    ImGui::Text("# Demons: %d", num_demons);
    if(ImGui::Button("Place demon",{125,25})){
        place_player = false;
        place_sax = false;
        place_notes = false;
        place_demon = !place_demon;
        place_fireball = false;
        draw = false;
        place_entities = place_demon;
    }
    
    if(b.demons.size()&&ImGui::Button("Delete demon",{125,25})){
        b.demons.erase(b.demons.begin()+demon_idx);
    }
    
    if(b.demons.size()>0){
        demon_idx++;
        ImGui::InputInt("Demon Index",&demon_idx,1,1);
        demon_idx--;
        if(demon_idx>=b.saxophones.size()) demon_idx = b.demons.size()-1;
        if(demon_idx<0) demon_idx = 0;
        
        int demon_n = demon_idx+1;
        ImGui::Text("Edit demon #%d",demon_n);
        
        demon_t& demon = b.demons[demon_idx];
        int movement = demon.movement;
        ImGui::InputInt("movement##demon",&movement,1,5);
        movement %= 16;
        demon.movement = movement;
        
        int pos_x = demon.x;
        int pos_y = demon.y;
        ImGui::InputInt("x position##demon",&pos_x,1,5);
        ImGui::InputInt("y position##demon",&pos_y,1,5);
        demon.x = pos_x;
        demon.y = pos_y;
        
        int shot_cooldown = demon.shot_cooldown;
        ImGui::InputInt("shot_cooldown##demon",&shot_cooldown,1,5);
        demon.shot_cooldown = shot_cooldown;
        
    }
    
    ImGui::Separator();
    
    int num_fireballs = b.fireballs.size();
    ImGui::Text("# Fireballs: %d", num_fireballs);
    if(ImGui::Button("Place fireball",{125,25})){
        place_player = false;
        place_sax = false;
        place_notes = false;
        place_demon = false;
        place_fireball = !place_fireball;
        draw = false;
        place_entities = place_fireball;
    }
    
    if(b.fireballs.size()&&ImGui::Button("Delete fireball",{125,25})){
        b.fireballs.erase(b.fireballs.begin()+fireball_idx);
    }
    
    if(b.fireballs.size()>0){
        fireball_idx++;
        ImGui::InputInt("Fireball Index",&fireball_idx,1,1);
        notes_idx--;
        if(fireball_idx>=b.notes.size()) fireball_idx = b.fireballs.size()-1;
        if(fireball_idx<0) fireball_idx = 0;
        
        int fireball_n = fireball_idx+1;
        ImGui::Text("Edit fireball #%d",fireball_n);
        
        fireball_t& fireball = b.fireballs[fireball_idx];
        int movement = fireball.movement;
        ImGui::InputInt("movement##fireballs",&movement,1,5);
        movement %= 16;
        fireball.movement = movement;
        
        int pos_x = fireball.x;
        int pos_y = fireball.y;
        ImGui::InputInt("x position##fireballs",&pos_x,1,5);
        ImGui::InputInt("y position##fireballs",&pos_y,1,5);
        fireball.x = pos_x;
        fireball.y = pos_y;
        
    }
    
    ImGui::End();
}

void level_editor(){
    
    
    
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    float aspect_ratio = height/(float)width;
    
    float x0 = b.data.size()/-2.f;
    float y0 = b.data[0].size()/-2.f;
    glm::vec4 origin = ubo.proj*ubo.view*ubo.model*glm::vec4(x0-.5f,y0-.5f,b.depth,1);
    
    float x1 = b.data.size()/2.f;
    float y1 = b.data[0].size()/2.f;
    glm::vec4 max_p = ubo.proj*ubo.view*ubo.model*glm::vec4(x1-.5f,y1-.5f,b.depth,1);
    
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
        
        float depth = b.depth;
        ImGui::InputFloat("Depth",&depth,1,5);
        if(depth < 1) depth = 1;
        if(depth > 100) depth = 100;
        b.depth = depth;
        
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
        
        
        if(ImGui::Button("Clear board")){
            auto X_size = b.data.size();
            auto Y_size = X_size;
            Y_size = 0;
            if(X_size>0) Y_size = b.data[0].size();
            b.resize(0,0);// initializes everything to pits.
            b.resize(X_size,Y_size);
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
                case tile_t::door_open   : ImGui::Text("type: door open");   break;
                case tile_t::door_closed : ImGui::Text("type: door closed"); break;
                case tile_t::firepit_on  : ImGui::Text("type: firepit on");  break;
                case tile_t::firepit_off : ImGui::Text("type: firepit off"); break;
                case tile_t::bird_bath   : ImGui::Text("type: bird bath");   break;
                case tile_t::exit        : ImGui::Text("type: exit");        break;
            }
        }else{
            ImGui::Text("<,>");
            ImGui::Text("This area is non-editable.");
        }
        
        
        static bool draw = false;
        static bool place_entities = false;
        if(draw) ImGui::Text("Drawing level tiles.");
        if(place_entities) ImGui::Text("Placing entities.");
        
        if(!draw&&ImGui::Button("Draw",{200,50})){
            draw = true;
            place_entities = false;
        }else// the else prevents them both appearing on the frame of transition.
        if( draw&&ImGui::Button("No Draw",{200,50})) draw = false;
        
        static tile_t new_type = tile_t::pit;
        std::string selected_type = to_str(new_type);
        
        ImGui::Text("Selected tile type: %s", selected_type.c_str());
        if(ImGui::Button("pit",{100,25})) new_type = tile_t::pit;
        if(ImGui::Button("floor",{100,25})) new_type = tile_t::floor;
        if(ImGui::Button("wall",{100,25})) new_type = tile_t::wall;
        if(ImGui::Button("plate",{100,25})) new_type = tile_t::plate;
        if(ImGui::Button("door open",{100,25})) new_type = tile_t::door_open;
        if(ImGui::Button("door closed",{100,25})) new_type = tile_t::door_closed;
        if(ImGui::Button("firepit on",{100,25})) new_type = tile_t::firepit_on;
        if(ImGui::Button("firepit off",{100,25})) new_type = tile_t::firepit_off;
        if(ImGui::Button("bird bath",{100,25})) new_type = tile_t::bird_bath;
        if(ImGui::Button("exit",{100,25})) new_type = tile_t::exit;
        
        static bool edit_plate = false;
        static bool edit_door = false;
        static bool edit_wall = false;
        static bool edit_firepit = false;
        static bool edit_pit = false;
        static bool edit_exit = false;
        static bool edit_floor = false;
        static int edit_x;
        static int edit_y;
        static tile_t expected_tile_type = tile_t::floor;
        
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
                        case tile_t::bird_bath  : delete (firepit_t*)b.data[X_block][Y_block].cell_data; break;
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
                        case tile_t::bird_bath:
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
                
                edit_x = X_block;
                edit_y = Y_block;
                
                edit_plate = b.data[X_block][Y_block].type==tile_t::plate;
                edit_door = b.data[X_block][Y_block].type==tile_t::door_open||b.data[X_block][Y_block].type==tile_t::door_closed;
                edit_wall = b.data[X_block][Y_block].type==tile_t::wall;
                edit_firepit = b.data[X_block][Y_block].type==tile_t::firepit_on||b.data[X_block][Y_block].type==tile_t::firepit_off;
                edit_pit = b.data[X_block][Y_block].type==tile_t::pit;
                edit_exit = b.data[X_block][Y_block].type==tile_t::exit;
                edit_floor = b.data[X_block][Y_block].type==tile_t::floor;
                
                if(edit_pit) expected_tile_type = tile_t::pit;
                if(edit_floor) expected_tile_type = tile_t::floor;
                if(edit_wall) expected_tile_type = tile_t::wall;
                if(edit_firepit) expected_tile_type = tile_t::firepit_on;
                if(edit_door) expected_tile_type = tile_t::door_closed;
                if(edit_exit) expected_tile_type = tile_t::exit;
                if(edit_plate) expected_tile_type = tile_t::plate;
                
            }
            
        }
        
        
    ImGui::End();
    
    entity_manager(X_block,Y_block,draw,place_entities);
    
    if(edit_x>=b.data.size()||edit_y>b.data[0].size()||!tile_kinds_match(b.data[edit_x][edit_y].type,expected_tile_type)){
        edit_plate   = false;
        edit_door    = false;
        edit_wall    = false;
        edit_firepit = false;
        edit_pit     = false;
        edit_floor   = false;
        edit_exit    = false;
    }
    
    if(edit_plate&&draw){
    ImGui::Begin("Plate editor");
        
        plate_t& P = *(plate_t*)b.data[edit_x][edit_y].cell_data;
        
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
        
        door_t& D = *(door_t*)b.data[edit_x][edit_y].cell_data;
        
        int character = D.character;
        ImGui::InputInt("character",&character,1,5);
        if(character< 0) character = 0;
        if(character>32) character = 32;
        D.character = character;
        
        int variation = D.variation;
        ImGui::InputInt("Variation",&variation,1,5);
        if(variation< 0) variation = 0;
        if(variation>3) variation = 3;
        D.variation = variation;
        
    ImGui::End();
    }
    
    if(edit_pit&&draw){
    ImGui::Begin("Pit editor");
        
        pit_t& P = *(pit_t*)b.data[edit_x][edit_y].cell_data;
        
        int character = P.character;
        ImGui::InputInt("character",&character,1,5);
        if(character< 0) character = 0;
        if(character>32) character = 32;
        P.character = character;
        
    ImGui::End();
    }
    
    if(edit_firepit&&draw){
    ImGui::Begin("Firepit editor");
        
        firepit_t& F = *(firepit_t*)b.data[edit_x][edit_y].cell_data;
        
        int variation = F.variation;
        ImGui::InputInt("Variation",&variation,1,5);
        if(variation< 0) variation = 0;
        if(variation>3) variation = 3;
        F.variation = variation;
        
    ImGui::End();
    }
    
    if(edit_exit&&draw){
    ImGui::Begin("Exit editor");
        
        exit_t& E = *(exit_t*)b.data[edit_x][edit_y].cell_data;
        
        int variation = E.variation;
        ImGui::InputInt("Variation",&variation,1,5);
        if(variation< 0) variation = 0;
        if(variation>3) variation = 3;
        E.variation = variation;
        
    ImGui::End();
    }
    
    if(edit_floor&&draw){
    ImGui::Begin("Floor editor");
        
        floor_t& F = *(floor_t*)b.data[edit_x][edit_y].cell_data;
        
        int variation = F.variation;
        ImGui::InputInt("Variation",&variation,1,5);
        if(variation< 0) variation = 0;
        if(variation>3) variation = 3;
        F.variation = variation;
        
    ImGui::End();
    }
    
    if(edit_wall&&draw){
    ImGui::Begin("Wall editor");
        wall_t& W = *(wall_t*)b.data[edit_x][edit_y].cell_data;
        
        int character = W.character;
        ImGui::InputInt("character",&character,1,5);
        if(character< 0) character = 0;
        if(character>32) character = 32;
        W.character = character;
        
        int variation = W.variation;
        ImGui::InputInt("variation",&variation,1,5);
        if(character< 0) variation = 0;
        if(character>3) variation = 3;
        W.variation = variation;
        
    ImGui::End();
    }
}