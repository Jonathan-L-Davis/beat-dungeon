#include "beat-dungeon.h"
#include "init.h"

#include <glm/glm.hpp>
#include <array>

#include "engine/io/keyboard.h"

constexpr uint32_t version = 0;

bool board::load_level(std::string file_name){
    
    std::vector<uint8_t> file = load_file(file_name);
    
    uint32_t x,y;
    read_from_buffer(file,4,x);
    read_from_buffer(file,8,y);
    
    assert(x<=32&&"Board is larger than expected.");
    assert(y<=32&&"Board is larger than expected.");
    
    data.resize(x);
    for(int i = 0; i < data.size(); i++)
        data[i].resize(y);
    
    int idx = 12;
    for(int i = 0; i < data.size(); i++){
        for(int j = 0; j < data[i].size(); j++){
            read_from_buffer( file, idx, *(uint8_t*)&data[i][j].type);idx++;
            if(data[i][j].type==floor_t::plate){
                data[i][j].cell_data = new plate_t;
                plate_t& plate = *((plate_t*)data[i][j].cell_data);
                read_from_buffer(file, idx, plate.x);idx++;
                read_from_buffer(file, idx, plate.y);idx++;
                read_from_buffer(file, idx, plate.ticks_alive);idx++;
                read_from_buffer(file, idx, plate.max_ticks);idx++;
            }
        }
    }
    
    return false;
}

bool board::save_level(std::string file_name){
    
    std::vector<uint8_t> file;
    append_to_buffer(file,version);
    
    if(data.size()==0) return false;
    if(data[0].size()==0) return false;
    
    int x = data.size();
    int y = data[0].size();
    append_to_buffer(file,(uint32_t)x);
    append_to_buffer(file,(uint32_t)y);
    
    assert(x<=32&&"Board is larger than expected.");
    assert(y<=32&&"Board is larger than expected.");
    
    data.resize(x);
    for(int i = 0; i < data.size(); i++)
        data[i].resize(y);
    
    for(int i = 0; i < data.size(); i++){
        for(int j = 0; j < data[i].size(); j++){
            cell_t& c = data[i][j];
            
            append_to_buffer(file,(uint8_t)c.type);
            
            if(c.type==floor_t::plate){
                plate_t& plate = *((plate_t*)c.cell_data);
                append_to_buffer(file, (uint8_t)plate.x);
                append_to_buffer(file, (uint8_t)plate.y);
                append_to_buffer(file, (uint8_t)plate.ticks_alive);
                append_to_buffer(file, (uint8_t)plate.max_ticks);
            }
        }
    }
    
    save_file(file_name,file);
    
    return false;
}

void board::resize(int x, int y){
    
    for(int i = 0; i < data.size(); i++){
        for(int j = 0; j < data[0].size(); j++){
            if( (i>=x || j>=y) && data[i][j].type == floor_t::plate ){
                delete (plate_t*) data[i][j].cell_data;
                data[i][j].cell_data = nullptr;
            }
        }
    }
    
    int old_x = data.size();
    int old_y = 0;
    if(old_x>0) old_y = data[0].size();
    
    data.resize(x);
    for(int i = 0; i < data.size(); i++)
        data[i].resize(y);
    
    for(int i = 0; i < data.size(); i++){
        for(int j = 0; j < data[0].size(); j++){
            if( (i>=old_x || j>=old_y) ){
                data[i][j].cell_data = nullptr;
                data[i][j].type = floor_t::floor;
            }
        }
    }
}

bool board::is_flingable(uint32_t x,uint32_t y){
    
    if(x>=data.size()) return false;
    
    if(y>=data[0].size()) return false;
    
    if(data[x][y].type==floor_t::wall) return true;
    if(data[x][y].type==floor_t::firepit_off) return true;
    if(data[x][y].type==floor_t::door_closed) return true;
    
    return false;
}

bool board::is_walkable(uint32_t x,uint32_t y){
    
    if(x>=data.size()) return false;
    
    if(y>=data[0].size()) return false;
    
    if(data[x][y].type==floor_t::floor) return true;
    if(data[x][y].type==floor_t::plate) return true;
    if(data[x][y].type==floor_t::door_open) return true;
    if(data[x][y].type==floor_t::exit) return true;
    
    return false;
}

void board::step(int beat, uint8_t movement){
    step_player(movement);
    if(beat%4==0) step_plates();
}

void board::step_plates(){
    for( int i = 0; i < data.size(); i++ ){
        for( int j = 0; j < data[i].size(); j++ ){
            cell_t& cell = data[i][j];
            if(cell.type==floor_t::plate){
                plate_t& plate = *(plate_t*)(cell.cell_data);
                // if plate is stepped on, make it alive.
                if(player.x==i&&player.y==j){// will need to loop through all entities later to do this as well.
                    if(plate.ticks_alive==0){
                        if( data[plate.x][plate.y].type==floor_t::door_open )
                            data[plate.x][plate.y].type = floor_t::door_closed;
                        if( data[plate.x][plate.y].type==floor_t::door_closed )
                            data[plate.x][plate.y].type = floor_t::door_open;
                    }
                    plate.ticks_alive = plate.max_ticks;
                }
            }
        }
    }
}

void board::step_player(uint8_t movement){
    constexpr uint32_t m = 0xFFFF'FFFF;// minus one, but for avoiding signed int undefined behavior.
    
    if(data.size()==0) return;
    
    bool move_up    =  (movement&1)&&!(movement&2)&&!(movement&4)&&!(movement&8);
    bool move_left  = !(movement&1)&& (movement&2)&&!(movement&4)&&!(movement&8);
    bool move_down  = !(movement&1)&&!(movement&2)&& (movement&4)&&!(movement&8);
    bool move_right = !(movement&1)&&!(movement&2)&&!(movement&4)&& (movement&8);
    
    /// easiest to explain with a picture. You can turn without hitting a wall if there is a corner to turn around.
    player.fling_lU = move_up    && player.moving_left  && is_flingable(player.x+1,player.y+m) && is_walkable(player.x  ,player.y+m);
    player.fling_rU = move_up    && player.moving_right && is_flingable(player.x+m,player.y+m) && is_walkable(player.x  ,player.y+m);
    player.fling_uL = move_left  && player.moving_up    && is_flingable(player.x+m,player.y+1) && is_walkable(player.x+m,player.y  );
    player.fling_dL = move_left  && player.moving_down  && is_flingable(player.x+m,player.y+m) && is_walkable(player.x+m,player.y  );
    
    player.fling_lD = move_down  && player.moving_left  && is_flingable(player.x+1,player.y+1) && is_walkable(player.x  ,player.y+1);
    player.fling_rD = move_down  && player.moving_right && is_flingable(player.x+m,player.y+1) && is_walkable(player.x  ,player.y+1);
    player.fling_uR = move_right && player.moving_up    && is_flingable(player.x+1,player.y+1) && is_walkable(player.x+1,player.y  );
    player.fling_dR = move_right && player.moving_down  && is_flingable(player.x+1,player.y+m) && is_walkable(player.x+1,player.y  );
    
    bool fling_up    = player.fling_lU||player.fling_rU;
    bool fling_left  = player.fling_dL||player.fling_uL;
    bool fling_down  = player.fling_lD||player.fling_rD;
    bool fling_right = player.fling_dR||player.fling_uR;
    
    player.moving_up    &= !fling_left && !fling_right;
    player.moving_left  &= !fling_up   && !fling_down ;
    player.moving_down  &= !fling_left && !fling_right;
    player.moving_right &= !fling_up   && !fling_down ;
    
    bool sitting_still = !player.moving_up&&!player.moving_left&&!player.moving_down&&!player.moving_right;
    
    uint32_t old_x = player.x, old_y = player.y;
    uint32_t new_x = player.x, new_y = player.y;
    
    if((move_up&&sitting_still)||player.moving_up||fling_up){
        new_y -= 1;
        player.moving_up = true;
        player.moving_left = false;
        player.moving_down = false;
        player.moving_right = false;
    }
    
    if((move_left&&sitting_still)||player.moving_left||fling_left){
        new_x -= 1;
        player.moving_up = false;
        player.moving_left = true;
        player.moving_down = false;
        player.moving_right = false;
    }
    
    if((move_down&&sitting_still)||player.moving_down||fling_down){
        new_y += 1;
        player.moving_up = false;
        player.moving_left = false;
        player.moving_down = true;
        player.moving_right = false;
    }
    
    if((move_right&&sitting_still)||player.moving_right||fling_right){
        new_x += 1;
        player.moving_up = false;
        player.moving_left = false;
        player.moving_down = false;
        player.moving_right = true;
    }
    
    if(new_x>=data.size()||new_y>=data[0].size()){
        player.moving_up = false;
        player.moving_left = false;
        player.moving_down = false;
        player.moving_right = false;
        return;
    }
    
    if(!is_walkable(new_x,new_y)){
        player.moving_up = false;
        player.moving_left = false;
        player.moving_down = false;
        player.moving_right = false;
        
        return;
    }
    
    player.x = new_x;
    player.y = new_y;
    
}

std::array<glm::vec2,4> get_tex_coords(init_data_t::rect R, uint32_t w, uint32_t h){
    std::array<glm::vec2,4> retMe;
    
    retMe[0] = {float(R.x    )/w,float(R.y    )/h};
    retMe[1] = {float(R.x+R.w)/w,float(R.y    )/h};
    retMe[2] = {float(R.x+R.w)/w,float(R.y+R.h)/h};
    retMe[3] = {float(R.x    )/w,float(R.y+R.h)/h};
    
    return retMe;
}

std::vector<graphics::vulkan::Vertex> draw_board(board b, float aspect_ratio){
    
    auto atlas = init_data.atlas;
    std::array<glm::vec2,4> texel;
    glm::vec3 color = {1,1,1};
    std::vector<graphics::vulkan::Vertex> retMe;
    
    float x0 = -.5;
    float x1 =  .5;
    float y0 = -.5;
    float y1 =  .5;
    
    float dX = 0;
    float dY = 0;
    
    if(b.data.size()>0&&b.data[0].size()>0){
        dX = b.data.size()/-2.f;
        dY = b.data[0].size()/-2.f;
    }
    
    
    //float xDepth = (1920.f/10.f)/(16.f*b.data   .size());
    //float yDepth = (1080.f/10.f)/(16.f*b.data[0].size());
    
    float depth = 20;//std::min(xDepth,yDepth);
    
    for( int x = 0; x < b.data.size(); x++ ){
        for( int y = 0; y < b.data[x].size(); y++ ){
            
            bool skip = false;
            switch(b.data[x][y].type){
                case floor_t::wall       : texel = get_tex_coords(atlas.wall,atlas.w,atlas.h); break;
                case floor_t::floor      : texel = get_tex_coords(atlas.floor,atlas.w,atlas.h); break;
                case floor_t::plate      : texel = get_tex_coords(atlas.plate,atlas.w,atlas.h); break;
                case floor_t::firepit_on : texel = get_tex_coords(atlas.fire,atlas.w,atlas.h); break;
                case floor_t::firepit_off: texel = get_tex_coords(atlas.fire_out,atlas.w,atlas.h); break;
                case floor_t::door_open  : texel = get_tex_coords(atlas.door_open,atlas.w,atlas.h); break;
                case floor_t::door_closed: texel = get_tex_coords(atlas.door_closed,atlas.w,atlas.h); break;
                case floor_t::exit       : texel = get_tex_coords(atlas.exit,atlas.w,atlas.h); break;
                default: skip = true;
            }
            
            if(skip)continue;
            
            retMe.push_back({{x0+x+dX, y0+y+dY, depth}, color, texel[0]});
            retMe.push_back({{x1+x+dX, y0+y+dY, depth}, color, texel[1]});
            retMe.push_back({{x1+x+dX, y1+y+dY, depth}, color, texel[2]});
            retMe.push_back({{x0+x+dX, y1+y+dY, depth}, color, texel[3]});
        }
        
    }
    
    //*
    texel = get_tex_coords(atlas.player,atlas.w,atlas.h);
    retMe.push_back({{x0+b.player.x+dX, y0+b.player.y+dY, depth}, color, texel[0]});
    retMe.push_back({{x1+b.player.x+dX, y0+b.player.y+dY, depth}, color, texel[1]});
    retMe.push_back({{x1+b.player.x+dX, y1+b.player.y+dY, depth}, color, texel[2]});
    retMe.push_back({{x0+b.player.x+dX, y1+b.player.y+dY, depth}, color, texel[3]});//*/
    
    return retMe;
}

