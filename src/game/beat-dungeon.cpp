#include "beat-dungeon.h"
#include "init.h"

#include <glm/glm.hpp>
#include <array>

#include "engine/io/keyboard.h"

#include "audio/audio.h"

constexpr uint32_t format_version = 2;

constexpr int sax_cooldown = 4;
constexpr int sax_range = 6;
constexpr int drum_cooldown = 4;
constexpr int drum_range = 6;
constexpr int demon_cooldown = 2;
constexpr int demon_range = 6;
constexpr int camel_cooldown = 8;
constexpr int camel_range = 6;

constexpr glm::vec3 plate_colors[3] = {{.577,.025,0},{1,.25,0},{1,1,0}};
constexpr glm::vec3 heiroglyphic_colors[3] = {{.094,.396,.063},{1,.5,0},{1,1,0}};

std::string to_str(tile_t strMe){
    switch(strMe){
        case tile_t::pit         : return "pit";
        case tile_t::floor       : return "floor";
        case tile_t::wall        : return "wall";
        case tile_t::plate       : return "plate";
        case tile_t::door_open   : return "door_open";
        case tile_t::door_closed : return "door_closed";
        case tile_t::firepit_on  : return "firepit_on";
        case tile_t::firepit_off : return "firepit_off";
        case tile_t::exit        : return "exit";
    }
}

bool tile_is_door(tile_t a){
    return a==tile_t::door_open||a==tile_t::door_closed;
}

bool tile_is_firepit(tile_t a){
    return a==tile_t::firepit_on||a==tile_t::firepit_off;
}

bool tile_kinds_match(tile_t a,tile_t b){
    if(a==b) return true;
    
    if(tile_is_door(a)&&tile_is_door(b)) return true;
    
    if(tile_is_firepit(a)&&tile_is_firepit(b)) return true;
    
    return false;
}

bool board::load_level(std::string file_name){
    level = file_name;
    
    std::vector<uint8_t> file = load_file(file_name);
    
    return load_level(file);
}

bool board::load_level(std::vector<uint8_t>& file){
    
    if(file.size()==0) return false;
    
    int idx = 0;
    uint32_t file_version;
    read_from_buffer(file,idx,file_version);
    //assert(file_version =< 2 && "Oh no! You tried to load a file with version greater than exists!");
    
    uint32_t x,y;
    read_from_buffer(file,idx,x);
    read_from_buffer(file,idx,y);
    
    depth = 20.f;
    if(file_version>=2) read_from_buffer(file,idx,depth);
    
    assert(x<=32&&"Board is larger than expected.");
    assert(y<=32&&"Board is larger than expected.");
    
    data.resize(x);
    for(int i = 0; i < data.size(); i++)
        data[i].resize(y);
    
    for(int i = 0; i < data.size(); i++){
        for(int j = 0; j < data[i].size(); j++){
            read_from_buffer( file, idx, *(uint8_t*)&data[i][j].type);
            switch(data[i][j].type){
                case tile_t::pit:{
                    data[i][j].cell_data = new pit_t{};
                    pit_t& pit = *((pit_t*)data[i][j].cell_data);
                    if(file_version>=1){
                        read_from_buffer(file, idx, pit.character);
                    }
                }break;
                case tile_t::firepit_on:
                case tile_t::firepit_off:{
                    data[i][j].cell_data = new firepit_t{};
                    firepit_t& firepit = *((firepit_t*)data[i][j].cell_data);
                    if(file_version>=1){
                        read_from_buffer(file, idx, firepit.variation);
                    }
                }break;
                case tile_t::floor:{
                    data[i][j].cell_data = new floor_t{};
                    floor_t& floor = *((floor_t*)data[i][j].cell_data);
                    if(file_version>=1){
                        read_from_buffer(file, idx, floor.variation);
                    }
                }break;
                case tile_t::exit:{
                    data[i][j].cell_data = new exit_t{};
                    exit_t& exit = *((exit_t*)data[i][j].cell_data);
                    if(file_version>=1){
                        read_from_buffer(file, idx, exit.variation);
                    }
                }break;
                case tile_t::plate:{
                    data[i][j].cell_data = new plate_t{};
                    plate_t& plate = *((plate_t*)data[i][j].cell_data);
                    if(file_version==0)idx+=2;// skipping over the xy coords now. They no longer exist.
                    if(file_version>=1){ read_from_buffer(file, idx, plate.character); }
                    read_from_buffer(file, idx, plate.ticks_alive);
                    read_from_buffer(file, idx, plate.max_ticks);
                }break;
                case tile_t::wall:{
                    data[i][j].cell_data = new wall_t{};
                    wall_t wall = *((wall_t*)data[i][j].cell_data);
                    if(file_version>=1){
                        read_from_buffer(file, idx, wall.character);
                        read_from_buffer(file, idx, wall.variation);
                    }
                }break;
                case tile_t::door_closed:
                case tile_t::door_open:{
                    data[i][j].cell_data = new door_t{};
                    door_t& door = *((door_t*)data[i][j].cell_data);
                    if(file_version>=1){
                        read_from_buffer(file, idx, door.character);
                    }
                    if(file_version>=2){
                        read_from_buffer(file, idx, door.variation);
                    }
                }break;
            }
        }
    }
    
    player = player_t();
    
    read_from_buffer(file, idx, player.x);
    read_from_buffer(file, idx, player.y);
    
    saxophones = {};
    drums      = {};
    notes      = {};
    demons     = {};
    fireballs  = {};
    
    update_wall_borders();
    
    if(file_version==0) return true;
    
    // load saxophones
    uint32_t num_saxophones = 0;
    read_from_buffer(file, idx, num_saxophones);
    for(int i = 0; i < num_saxophones; i++){
        sax_t new_sax;
        read_from_buffer(file,idx,new_sax.x);
        read_from_buffer(file,idx,new_sax.y);
        read_from_buffer(file,idx,new_sax.movement);
        read_from_buffer(file,idx,new_sax.shot_cooldown);
        saxophones.push_back(new_sax);
    }
    
    if(file_version>=2){
        
        // load drums
        uint32_t num_drums = 0;
        read_from_buffer(file, idx, num_drums);
        for(int i = 0; i < num_drums; i++){
            drum_t new_drum;
            read_from_buffer(file,idx,new_drum.x);
            read_from_buffer(file,idx,new_drum.y);
            read_from_buffer(file,idx,new_drum.movement);
            read_from_buffer(file,idx,new_drum.shot_cooldown);
            drums.push_back(new_drum);
        }
        
        // load demons
        uint32_t num_demons = 0;
        read_from_buffer(file, idx, num_demons);
        for(int i = 0; i < num_demons; i++){
            demon_t new_demon;
            read_from_buffer(file,idx,new_demon.x);
            read_from_buffer(file,idx,new_demon.y);
            read_from_buffer(file,idx,new_demon.movement);
            read_from_buffer(file,idx,new_demon.shot_cooldown);
            demons.push_back(new_demon);
        }
        
        // load camels
        uint32_t num_camels = 0;
        read_from_buffer(file, idx, num_camels);
        for(int i = 0; i < num_camels; i++){
            camel_t new_camel;
            read_from_buffer(file,idx,new_camel.x);
            read_from_buffer(file,idx,new_camel.y);
            read_from_buffer(file,idx,new_camel.movement);
            read_from_buffer(file,idx,new_camel.shot_cooldown);
            camels.push_back(new_camel);
        }
        
    }
    // load notes
    uint32_t num_notes = 0;
    read_from_buffer(file, idx, num_notes);
    for(int i = 0; i < num_notes; i++){
        notes_t new_notes;
        read_from_buffer(file,idx,new_notes.x);
        read_from_buffer(file,idx,new_notes.y);
        read_from_buffer(file,idx,new_notes.movement);
        notes.push_back(new_notes);
    }
    
    if(file_version>=2){
        uint32_t num_fireballs = 0;
        read_from_buffer(file, idx, num_fireballs);
        for(int i = 0; i < num_fireballs; i++){
            fireball_t new_fireball;
            read_from_buffer(file,idx,new_fireball.x);
            read_from_buffer(file,idx,new_fireball.y);
            read_from_buffer(file,idx,new_fireball.movement);
            fireballs.push_back(new_fireball);
        }
        
        uint32_t num_spitballs = 0;
        read_from_buffer(file, idx, num_spitballs);
        for(int i = 0; i < num_spitballs; i++){
            spitball_t new_spitball;
            read_from_buffer(file,idx,new_spitball.x);
            read_from_buffer(file,idx,new_spitball.y);
            read_from_buffer(file,idx,new_spitball.movement);
            spitballs.push_back(new_spitball);
        }
    }
    
    return true;
}

bool board::save_level(std::string file_name){
    
    if(file_name=="") file_name = level;
    level = file_name;
    
    std::vector<uint8_t> file;
    
    save_level(file);
    
    return save_file(file_name,file);
}

bool board::save_level(std::vector<uint8_t>& file){
    
    
    append_to_buffer(file,format_version);
    
    if(data.size()==0) return false;
    if(data[0].size()==0) return false;
    
    int x = data.size();
    int y = data[0].size();
    append_to_buffer(file,(uint32_t)x);
    append_to_buffer(file,(uint32_t)y);
    append_to_buffer(file,depth);
    
    for(int i = 0; i < data.size(); i++){
        for(int j = 0; j < data[i].size(); j++){
            cell_t& c = data[i][j];
            
            append_to_buffer(file,(uint8_t)c.type);
            switch(c.type){
                case tile_t::pit:{
                    pit_t& pit = *((pit_t*)c.cell_data);
                    append_to_buffer(file, (uint8_t)pit.character);
                }break;
                case tile_t::firepit_on:
                case tile_t::firepit_off:{
                    firepit_t& firepit = *((firepit_t*)c.cell_data);
                    append_to_buffer(file, (uint8_t)firepit.variation);
                }break;
                case tile_t::floor:{
                    floor_t& floor = *((floor_t*)c.cell_data);
                    append_to_buffer(file, (uint8_t)floor.variation);
                }break;
                case tile_t::exit:{
                    exit_t& exit = *((exit_t*)c.cell_data);
                    append_to_buffer(file, (uint8_t)exit.variation);
                }break;
                case tile_t::plate:{
                    plate_t& plate = *((plate_t*)c.cell_data);
                    append_to_buffer(file, (uint8_t)plate.character);
                    append_to_buffer(file, (uint8_t)plate.ticks_alive);
                    append_to_buffer(file, (uint8_t)plate.max_ticks);
                }break;
                case tile_t::wall:{
                    wall_t& wall = *((wall_t*)c.cell_data);
                    append_to_buffer(file, (uint8_t)wall.character);
                    append_to_buffer(file, (uint8_t)wall.variation);// may not be used, but doesn't hurt to store. ( probably for a subtle cracking texture.
                }break;
                case tile_t::door_closed:
                case tile_t::door_open:{
                    door_t& door = *((door_t*)c.cell_data);
                    append_to_buffer(file, (uint8_t)door.character);
                    append_to_buffer(file, (uint8_t)door.variation);
                }break;
            }
        }
    }
    
    append_to_buffer(file,player.x);
    append_to_buffer(file,player.y);
    
    // save saxophones
    uint32_t num_saxophones = saxophones.size();
    append_to_buffer(file, num_saxophones);
    for(const sax_t& old_sax: saxophones){
        append_to_buffer(file,old_sax.x);
        append_to_buffer(file,old_sax.y);
        append_to_buffer(file,old_sax.movement);
        append_to_buffer(file,old_sax.shot_cooldown);
    }
    
    // save drummers
    uint32_t num_drums = drums.size();
    append_to_buffer(file, num_drums);
    for(const drum_t old_drummer: drums){
        append_to_buffer(file,old_drummer.x);
        append_to_buffer(file,old_drummer.y);
        append_to_buffer(file,old_drummer.movement);
        append_to_buffer(file,old_drummer.shot_cooldown);
    }
    
    // save demons
    uint32_t num_demons = demons.size();
    append_to_buffer(file, num_demons);
    for(const demon_t old_demon: demons){
        append_to_buffer(file,old_demon.x);
        append_to_buffer(file,old_demon.y);
        append_to_buffer(file,old_demon.movement);
        append_to_buffer(file,old_demon.shot_cooldown);
    }
    
    // save camels
    uint32_t num_camels = camels.size();
    append_to_buffer(file, num_camels);
    for(const camel_t old_camel: camels){
        append_to_buffer(file,old_camel.x);
        append_to_buffer(file,old_camel.y);
        append_to_buffer(file,old_camel.movement);
        append_to_buffer(file,old_camel.shot_cooldown);
    }
    
    // save notes
    uint32_t num_notes = notes.size();
    append_to_buffer(file, num_notes);
    for(const notes_t old_notes: notes){
        append_to_buffer(file,old_notes.x);
        append_to_buffer(file,old_notes.y);
        append_to_buffer(file,old_notes.movement);
    }
    
    // save fireballs
    uint32_t num_fireballs = fireballs.size();
    append_to_buffer(file, num_fireballs);
    for(const fireball_t old_fireball: fireballs){
        append_to_buffer(file,old_fireball.x);
        append_to_buffer(file,old_fireball.y);
        append_to_buffer(file,old_fireball.movement);
    }
    
    // save spitballs
    uint32_t num_spitballs = spitballs.size();
    append_to_buffer(file, num_spitballs);
    for(const spitball_t old_spitball: spitballs){
        append_to_buffer(file,old_spitball.x);
        append_to_buffer(file,old_spitball.y);
        append_to_buffer(file,old_spitball.movement);
    }
    
    return true;
}

void board::resize(int x, int y){
    
    for(int i = 0; i < data.size(); i++){
        for(int j = 0; j < data[0].size(); j++){
            if( (i>=x || j>=y) )
            switch(data[i][j].type){
                case tile_t::plate:
                    delete (plate_t*) data[i][j].cell_data;
                    data[i][j].cell_data = nullptr;
                break;
                case tile_t::door_open:
                case tile_t::door_closed:
                    delete (door_t*) data[i][j].cell_data;
                    data[i][j].cell_data = nullptr;
                break;
                case tile_t::firepit_on:
                case tile_t::firepit_off:
                    delete (plate_t*) data[i][j].cell_data;
                    data[i][j].cell_data = nullptr;
                break;
                case tile_t::wall:
                    delete (wall_t*) data[i][j].cell_data;
                    data[i][j].cell_data = nullptr;
                break;
                case tile_t::exit:
                    delete (exit_t*) data[i][j].cell_data;
                    data[i][j].cell_data = nullptr;
                break;
                case tile_t::pit:
                    delete (pit_t*) data[i][j].cell_data;
                    data[i][j].cell_data = nullptr;
                break;
                case tile_t::floor:
                    delete (floor_t*) data[i][j].cell_data;
                    data[i][j].cell_data = nullptr;
                break;
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
                data[i][j].cell_data = new pit_t{};
                data[i][j].type = tile_t::pit;
            }
        }
    }
    
    // delete all enemies out of bounds
    for(int i = saxophones.size()-1; i >= 0; i--){
        if(saxophones[i].x>=x||saxophones[i].y>=y){
            saxophones.erase(saxophones.begin()+i);
        }
    }
    
    for(int i = drums.size()-1; i >= 0; i--){
        if(drums[i].x>=x||drums[i].y>=y){
            drums.erase(drums.begin()+i);
        }
    }
    
    for(int i = demons.size()-1; i >= 0; i--){
        if(demons[i].x>=x||demons[i].y>=y){
            demons.erase(demons.begin()+i);
        }
    }
    
    for(int i = notes.size()-1; i >= 0; i--){
        if(notes[i].x>=x||notes[i].y>=y){
            notes.erase(notes.begin()+i);
        }
    }
    
    for(int i = fireballs.size()-1; i >= 0; i--){
        if(fireballs[i].x>=x||fireballs[i].y>=y){
            fireballs.erase(fireballs.begin()+i);
        }
    }//*/
    
}

bool board::is_flingable(uint32_t x,uint32_t y){
    
    if(x>=data.size()) return false;
    
    if(y>=data[0].size()) return false;
    
    if(data[x][y].type==tile_t::wall) return true;
    if(data[x][y].type==tile_t::firepit_off) return true;
    if(data[x][y].type==tile_t::door_closed) return true;
    
    return false;
}

bool board::is_walkable(uint32_t x,uint32_t y){
    
    if(x>=data.size()) return false;
    
    if(y>=data[0].size()) return false;
    
    if(data[x][y].type==tile_t::floor) return true;
    if(data[x][y].type==tile_t::plate) return true;
    if(data[x][y].type==tile_t::door_open) return true;
    if(data[x][y].type==tile_t::exit) return true;
    
    return false;
}

bool board::is_visible(uint32_t x,uint32_t y){
    
    if(x>=data.size()) return false;
    
    if(y>=data[0].size()) return false;
    
    if(data[x][y].type==tile_t::floor) return true;
    if(data[x][y].type==tile_t::plate) return true;
    if(data[x][y].type==tile_t::door_open) return true;
    if(data[x][y].type==tile_t::exit) return true;
    if(data[x][y].type==tile_t::pit) return true;
    if(data[x][y].type==tile_t::firepit_on) return true;
    if(data[x][y].type==tile_t::firepit_off) return true;
    
    return false;
}

void board::step(int beat, uint8_t movement){
    
    if(player.dead || player.won){
        player.anim_phase += 1;
        if(player.anim_phase >= 16){
            player.anim_phase = 0;
            //player.dead = false;
            //player.won  = false;
            //load_level(level);
        }
        return;
    }
    
    step_plates();
    step_notes(beat);
    step_fireball(beat);
    step_saxophone(beat);
    step_demons(beat);
    step_player(beat,movement);
}

void board::step_plates(){
    for( int i = 0; i < data.size(); i++ ){
        for( int j = 0; j < data[i].size(); j++ ){
            cell_t& cell = data[i][j];
            if(cell.type==tile_t::plate){
                plate_t& plate = *(plate_t*)(cell.cell_data);
                // if plate is stepped on, make it alive.
                uint8_t character = plate.character;
                bool stepped_on = (player.x==i&&player.y==j);
                
                // E for everyone. (Actually stands for entity here lol)
                for(const auto& E:saxophones) if(E.x==i&&E.y==j) stepped_on = true;
                for(const auto& E:drums) if(E.x==i&&E.y==j) stepped_on = true;
                for(const auto& E:demons) if(E.x==i&&E.y==j) stepped_on = true;
                for(const auto& E:notes) if(E.x==i&&E.y==j) stepped_on = true;
                for(const auto& E:fireballs) if(E.x==i&&E.y==j) stepped_on = true;
                
                if(stepped_on){// will need to loop through all entities later to do this as well.
                    if(plate.ticks_alive==0){// hopefully prevents double triggering
                        for( int x = 0; x < data.size(); x++ )
                            for( int y = 0; y < data[x].size(); y++ )
                                if( data[x][y].type==tile_t::door_open && (*(door_t*)data[x][y].cell_data).character==character )
                                    data[x][y].type = tile_t::door_closed;
                                else
                                if( data[x][y].type==tile_t::door_closed && (*(door_t*)data[x][y].cell_data).character==character )
                                    data[x][y].type = tile_t::door_open;
                    }
                    plate.ticks_alive = plate.max_ticks;
                }
                
                if(plate.ticks_alive>0){
                    plate.ticks_alive--;
                    // put back to original state.
                    if(plate.ticks_alive==0){
                        for( int x = 0; x < data.size(); x++ )
                            for( int y = 0; y < data[x].size(); y++ )
                                if( data[x][y].type==tile_t::door_open && (*(door_t*)data[x][y].cell_data).character==character )
                                    data[x][y].type = tile_t::door_closed;
                                else
                                if( data[x][y].type==tile_t::door_closed && (*(door_t*)data[x][y].cell_data).character==character )
                                    data[x][y].type = tile_t::door_open;
                    }
                }
            }
        }
    }
}

void board::step_player(int beat,uint8_t movement){
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
    
    for(sax_t& sax:saxophones)
        if(player.x==sax.x&&player.y==sax.y)
            player.dead=true;
    
    for(drum_t& drummer:drums)
        if(player.x==drummer.x&&player.y==drummer.y)
            player.dead=true;
    
    for(demon_t& demon:demons)
        if(player.x==demon.x&&player.y==demon.y)
            player.dead=true;
    
    for(notes_t& note:notes)
        if(player.x==note.x&&player.y==note.y)
            player.dead=true;
    
    for(fireball_t& fireball:fireballs)
        if(player.x==fireball.x&&player.y==fireball.y)
            player.dead=true;
    
    if(player.dead) return;
    
    player.x = new_x;
    player.y = new_y;
    
    if(data[player.x][player.y].type==tile_t::exit) player.won = true;
    
}

void board::step_saxophone(int beat){
    
    if(data.size()==0) return;
    if(data[0].size()==0) return;
    
    for(sax_t& sax:saxophones){
        
        if(sax.shot_cooldown>0) sax.shot_cooldown--;
        
        int distance = -1;
        
        for(uint32_t y = sax.y; y < data.size()&&is_visible(sax.x,y);y--){
            if( player.x == sax.x && player.y == y && sax.shot_cooldown==0 ){
                sax.movement = 1;
                distance = sax.y-y;
                break;
            }
        }
        
        for(uint32_t x = sax.x; x < data.size()&&is_visible(x,sax.y);x--){
            if( player.x == x && player.y == sax.y && sax.shot_cooldown==0 ){
                sax.movement = 2;
                distance = sax.x-x;
                break;
            }
        }
        
        for(uint32_t y = sax.y; y < data.size()&&is_visible(sax.x,y);y++){
            if( player.x == sax.x && player.y == y && sax.shot_cooldown==0 ){
                sax.movement = 4;
                distance = y-sax.y;
                break;
            }
        }
        
        for(uint32_t x = sax.x; x < data.size()&&is_visible(x,sax.y);x++){
            if( player.x == x && player.y == sax.y && sax.shot_cooldown==0 ){
                sax.movement = 8;
                distance = x-sax.x;
                break;
            }
        }
        
        bool shoot = (distance>-1&&distance<=sax_range&&sax.shot_cooldown==0);
        
        for(const auto& note:notes){
            if(sax.x==note.x&&sax.y==note.y)
                shoot = true;
        }
        
        uint8_t movement = sax.movement;
        uint32_t new_x = sax.x, new_y = sax.y;
        bool move_up    =  (movement&1)&&!(movement&2)&&!(movement&4)&&!(movement&8);
        bool move_left  = !(movement&1)&& (movement&2)&&!(movement&4)&&!(movement&8);
        bool move_down  = !(movement&1)&&!(movement&2)&& (movement&4)&&!(movement&8);
        bool move_right = !(movement&1)&&!(movement&2)&&!(movement&4)&& (movement&8);
        
        if(move_up){
            new_y -= 1;
        }
        
        if(move_left){
            new_x -= 1;
        }
        
        if(move_down){
            new_y += 1;
        }
        
        if(move_right){
            new_x += 1;
        }
        
        if(shoot){
            // making the assumption that new pos is valid for notes to spawn in on.
            notes_t new_notes;
            new_notes.x = new_x;
            new_notes.y = new_y;
            new_notes.movement = sax.movement;
            // these 2 lines switch it's direction. Don't worry about it too much.
            sax.movement |= (sax.movement<<4);
            sax.movement >>= 2;
            sax.shot_cooldown = sax_cooldown;
            if(data[new_notes.x][new_notes.y].type==tile_t::firepit_on){
                data[new_notes.x][new_notes.y].type = tile_t::firepit_off;
                continue;
            }
            notes.push_back(new_notes);
        }
        
        if(!is_walkable(new_x,new_y)){
            // these 2 lines switch it's direction. Don't worry about it too much.
            sax.movement |= (sax.movement<<4);
            sax.movement >>= 2;
            continue;
        }
        
        for(uint32_t y = sax.y; y < data.size()&&is_visible(sax.x,y);y--)
            if( player.x == sax.x && player.y == y && sax.shot_cooldown==0 )
                sax.movement = 1;
        
        for(uint32_t x = sax.x; x < data.size()&&is_visible(x,sax.y);x--)
            if( player.x == x && player.y == sax.y && sax.shot_cooldown==0 )
                sax.movement = 2;
        
        for(uint32_t y = sax.y; y < data.size()&&is_visible(sax.x,y);y++)
            if( player.x == sax.x && player.y == y && sax.shot_cooldown==0 )
                sax.movement = 4;
        
        for(uint32_t x = sax.x; x < data.size()&&is_visible(x,sax.y);x++)
            if( player.x == x && player.y == sax.y && sax.shot_cooldown==0 )
                sax.movement = 8;
        
        sax.x = new_x;
        sax.y = new_y;
    }
}

void board::step_demons(int beat){
    
    if(data.size()==0) return;
    if(data[0].size()==0) return;
    
    for(demon_t& demon:demons){
        
        if(demon.shot_cooldown>0) demon.shot_cooldown--;
        
        int distance = -1;
        
        for(uint32_t y = demon.y; y < data.size()&&is_visible(demon.x,y);y--){
            if( player.x == demon.x && player.y == y && demon.shot_cooldown==0 ){
                demon.movement = 1;
                distance = demon.y-y;
                break;
            }
        }
        
        for(uint32_t x = demon.x; x < data.size()&&is_visible(x,demon.y);x--){
            if( player.x == x && player.y == demon.y && demon.shot_cooldown==0 ){
                demon.movement = 2;
                distance = demon.x-x;
                break;
            }
        }
        
        for(uint32_t y = demon.y; y < data.size()&&is_visible(demon.x,y);y++){
            if( player.x == demon.x && player.y == y && demon.shot_cooldown==0 ){
                demon.movement = 4;
                distance = y-demon.y;
                break;
            }
        }
        
        for(uint32_t x = demon.x; x < data.size()&&is_visible(x,demon.y);x++){
            if( player.x == x && player.y == demon.y && demon.shot_cooldown==0 ){
                demon.movement = 8;
                distance = x-demon.x;
                break;
            }
        }
        
        bool shoot = (distance>-1&&distance<=demon_range&&demon.shot_cooldown==0);
        
        for(const auto& note:notes){
            if(demon.x==note.x&&demon.y==note.y)
                shoot = true;
        }
        
        uint8_t movement = demon.movement;
        uint32_t new_x = demon.x, new_y = demon.y;
        bool move_up    =  (movement&1)&&!(movement&2)&&!(movement&4)&&!(movement&8);
        bool move_left  = !(movement&1)&& (movement&2)&&!(movement&4)&&!(movement&8);
        bool move_down  = !(movement&1)&&!(movement&2)&& (movement&4)&&!(movement&8);
        bool move_right = !(movement&1)&&!(movement&2)&&!(movement&4)&& (movement&8);
        
        if(move_up){
            new_y -= 1;
        }
        
        if(move_left){
            new_x -= 1;
        }
        
        if(move_down){
            new_y += 1;
        }
        
        if(move_right){
            new_x += 1;
        }
        
        if(shoot){
            // making the assumption that new pos is valid for notes to spawn in on.
            fireball_t new_fireball;
            new_fireball.x = new_x;
            new_fireball.y = new_y;
            new_fireball.movement = demon.movement;
            // these 2 lines switch it's direction. Don't worry about it too much.
            //demon.movement |= (demon.movement<<4);
            //demon.movement >>= 2;
            
            new_x = demon.x;
            new_y = demon.y;
            
            demon.shot_cooldown = demon_cooldown;
            if(data[new_fireball.x][new_fireball.y].type==tile_t::firepit_off){
                data[new_fireball.x][new_fireball.y].type = tile_t::firepit_on;
                continue;
            }
            fireballs.push_back(new_fireball);
        }
        
        if(!is_walkable(new_x,new_y)){
            // these 2 lines switch it's direction. Don't worry about it too much.
            demon.movement |= (demon.movement<<4);
            demon.movement >>= 2;
            continue;
        }
        
        for(uint32_t y = demon.y; y < data.size()&&is_visible(demon.x,y);y--)
            if( player.x == demon.x && player.y == y && demon.shot_cooldown==0 )
                demon.movement = 1;
        
        for(uint32_t x = demon.x; x < data.size()&&is_visible(x,demon.y);x--)
            if( player.x == x && player.y == demon.y && demon.shot_cooldown==0 )
                demon.movement = 2;
        
        for(uint32_t y = demon.y; y < data.size()&&is_visible(demon.x,y);y++)
            if( player.x == demon.x && player.y == y && demon.shot_cooldown==0 )
                demon.movement = 4;
        
        for(uint32_t x = demon.x; x < data.size()&&is_visible(x,demon.y);x++)
            if( player.x == x && player.y == demon.y && demon.shot_cooldown==0 )
                demon.movement = 8;
        
        demon.x = new_x;
        demon.y = new_y;
    }
}

void board::step_notes(int beat){
    
    if(data.size()==0) return;
    if(data[0].size()==0) return;
    
    for(notes_t& note:notes){
        uint8_t movement = note.movement;
        uint32_t new_x = note.x, new_y = note.y;
        bool move_up    =  (movement&1)&&!(movement&2)&&!(movement&4)&&!(movement&8);
        bool move_left  = !(movement&1)&& (movement&2)&&!(movement&4)&&!(movement&8);
        bool move_down  = !(movement&1)&&!(movement&2)&& (movement&4)&&!(movement&8);
        bool move_right = !(movement&1)&&!(movement&2)&&!(movement&4)&& (movement&8);
        
        if(move_up){
            new_y -= 1;
        }
        
        if(move_left){
            new_x -= 1;
        }
        
        if(move_down){
            new_y += 1;
        }
        
        if(move_right){
            new_x += 1;
        }
        
        if(!is_visible(new_x,new_y)){
            note.movement = 0;// non moving notes are marked for death & removed.
            continue;
        }
        
        note.x = new_x;
        note.y = new_y;
        
    }
    
    for(int i = notes.size()-1; i >= 0; i--){
        notes_t& note = notes[i];
        
        if(data[note.x][note.y].type==tile_t::firepit_on){
            note.movement = 0;// broken into 2 loops to catch all notes that move on a flame in the same turn.
        }
    }
    
    for(int i = notes.size()-1; i >= 0; i--){
        notes_t& note = notes[i];
        
        if(data[note.x][note.y].type==tile_t::firepit_on){
            data[note.x][note.y].type = tile_t::firepit_off;
        }
        
        if(note.movement==0)
            notes.erase(notes.begin()+i);
        
    }
}

void board::step_fireball(int beat){
    
    if(data.size()==0) return;
    if(data[0].size()==0) return;
    
    for(fireball_t& fireball:fireballs){
        uint8_t movement = fireball.movement;
        uint32_t new_x = fireball.x, new_y = fireball.y;
        bool move_up    =  (movement&1)&&!(movement&2)&&!(movement&4)&&!(movement&8);
        bool move_left  = !(movement&1)&& (movement&2)&&!(movement&4)&&!(movement&8);
        bool move_down  = !(movement&1)&&!(movement&2)&& (movement&4)&&!(movement&8);
        bool move_right = !(movement&1)&&!(movement&2)&&!(movement&4)&& (movement&8);
        
        if(move_up){
            new_y -= 1;
        }
        
        if(move_left){
            new_x -= 1;
        }
        
        if(move_down){
            new_y += 1;
        }
        
        if(move_right){
            new_x += 1;
        }
        
        if(!is_visible(new_x,new_y)){
            fireball.movement = 0;// non moving notes are marked for death & removed.
            continue;
        }
        
        fireball.x = new_x;
        fireball.y = new_y;
        
    }
    
    for(int i = fireballs.size()-1; i >= 0; i--){
        fireball_t& fireball = fireballs[i];
        
        if(data[fireball.x][fireball.y].type==tile_t::firepit_off){
            fireball.movement = 0;// broken into 2 loops to catch all notes that move on a flame in the same turn.
        }
    }
    
    for(int i = fireballs.size()-1; i >= 0; i--){
        fireball_t& fireball = fireballs[i];
        
        if(data[fireball.x][fireball.y].type==tile_t::firepit_off){
            data[fireball.x][fireball.y].type = tile_t::firepit_on;
        }
        
        if(fireball.movement==0)
            fireballs.erase(fireballs.begin()+i);
        
    }
}

void board::update_wall_borders(){
    for(int x = 0; x < data.size(); x++){
        for(int y = 0; y < data[x].size(); y++){
            
            if(data[x][y].type!=tile_t::wall) continue;
            
            uint8_t surroundings = 0;
            
            if(                 y  >0             &&data[x  ][y-1].type==tile_t::wall) surroundings |= 0b0000'0001;
            if(x+1<data.size()&&y  >0             &&data[x+1][y-1].type==tile_t::wall) surroundings |= 0b0000'0010;
            if(x+1<data.size()&&                    data[x+1][y  ].type==tile_t::wall) surroundings |= 0b0000'0100;
            if(x+1<data.size()&&y+1<data[x].size()&&data[x+1][y+1].type==tile_t::wall) surroundings |= 0b0000'1000;
            if(                 y+1<data[x].size()&&data[x  ][y+1].type==tile_t::wall) surroundings |= 0b0001'0000;
            if(x  >0          &&y+1<data[x].size()&&data[x-1][y+1].type==tile_t::wall) surroundings |= 0b0010'0000;
            if(x  >0          &&                    data[x-1][y  ].type==tile_t::wall) surroundings |= 0b0100'0000;
            if(x  >0          &&y  >0             &&data[x-1][y-1].type==tile_t::wall) surroundings |= 0b1000'0000;
            
            (*(wall_t*)data[x][y].cell_data).surroundings = surroundings;
        }
    }
}



std::array<glm::vec2,4> get_tex_coords(init_data_t::rect R, float w, float h){
    std::array<glm::vec2,4> retMe;
    
    retMe[0] = {float(R.x    )/w,float(R.y    )/h};
    retMe[1] = {float(R.x+R.w)/w,float(R.y    )/h};
    retMe[2] = {float(R.x+R.w)/w,float(R.y+R.h)/h};
    retMe[3] = {float(R.x    )/w,float(R.y+R.h)/h};
    
    return retMe;
}

std::vector<graphics::vulkan::Vertex> draw_board(board b, float aspect_ratio){
    
    const init_data_t::atlas_t& atlas = init_data.atlas;
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
    
    float depth = b.depth;
    
    for( int x = 0; x < b.data.size(); x++ ){
        for( int y = 0; y < b.data[x].size(); y++ ){
            
            bool skip = false;
            switch(b.data[x][y].type){
                case tile_t::wall       : break;
                case tile_t::floor      : texel = get_tex_coords(atlas.floor[(*(floor_t*)b.data[x][y].cell_data).variation],atlas.w,atlas.h);       break;
                case tile_t::plate      : texel = get_tex_coords(atlas.plate,atlas.w,atlas.h);       break;
                case tile_t::firepit_on : texel = get_tex_coords(atlas.fire,atlas.w,atlas.h);        break;
                case tile_t::firepit_off: texel = get_tex_coords(atlas.fire_out,atlas.w,atlas.h);    break;
                case tile_t::door_open  : texel = get_tex_coords(atlas.door_open,atlas.w,atlas.h);   break;
                case tile_t::door_closed: texel = get_tex_coords(atlas.door_closed,atlas.w,atlas.h); break;
                case tile_t::exit       : texel = get_tex_coords(atlas.exit,atlas.w,atlas.h);        break;
                case tile_t::pit        : goto draw_char;
                default: skip = true;
            }
            
            if(skip)continue;
            if(b.data[x][y].type==tile_t::wall){
                uint8_t tl_mask = 0b1100'0001;
                switch( (*(wall_t*)b.data[x][y].cell_data).surroundings&tl_mask ){
                    case 0b0000'0000: texel = get_tex_coords(atlas.wall_1_tl_000,atlas.w,atlas.h);break;
                    case 0b0000'0001: texel = get_tex_coords(atlas.wall_1_tl_001,atlas.w,atlas.h);break;
                    case 0b0100'0000: texel = get_tex_coords(atlas.wall_1_tl_010,atlas.w,atlas.h);break;
                    case 0b0100'0001: texel = get_tex_coords(atlas.wall_1_tl_011,atlas.w,atlas.h);break;
                    case 0b1000'0000: texel = get_tex_coords(atlas.wall_1_tl_100,atlas.w,atlas.h);break;
                    case 0b1000'0001: texel = get_tex_coords(atlas.wall_1_tl_101,atlas.w,atlas.h);break;
                    case 0b1100'0000: texel = get_tex_coords(atlas.wall_1_tl_110,atlas.w,atlas.h);break;
                    case 0b1100'0001: texel = get_tex_coords(atlas.wall_1_tl_111,atlas.w,atlas.h);break;
                    default:assert(false);
                }
                retMe.push_back({{x0+x+dX, y0+y+dY, depth}, color, texel[0]});
                retMe.push_back({{x1+x+dX-.5, y0+y+dY, depth}, color, texel[1]});
                retMe.push_back({{x1+x+dX-.5, y1+y+dY-.5, depth}, color, texel[2]});
                retMe.push_back({{x0+x+dX, y1+y+dY-.5, depth}, color, texel[3]});//*/
                
                uint8_t tr_mask = 0b0000'0111;
                switch( (*(wall_t*)b.data[x][y].cell_data).surroundings&tr_mask ){
                    case 0b0000'0000: texel = get_tex_coords(atlas.wall_1_tr_000,atlas.w,atlas.h);break;
                    case 0b0000'0001: texel = get_tex_coords(atlas.wall_1_tr_001,atlas.w,atlas.h);break;
                    case 0b0000'0010: texel = get_tex_coords(atlas.wall_1_tr_010,atlas.w,atlas.h);break;
                    case 0b0000'0011: texel = get_tex_coords(atlas.wall_1_tr_011,atlas.w,atlas.h);break;
                    case 0b0000'0100: texel = get_tex_coords(atlas.wall_1_tr_100,atlas.w,atlas.h);break;
                    case 0b0000'0101: texel = get_tex_coords(atlas.wall_1_tr_101,atlas.w,atlas.h);break;
                    case 0b0000'0110: texel = get_tex_coords(atlas.wall_1_tr_110,atlas.w,atlas.h);break;
                    case 0b0000'0111: texel = get_tex_coords(atlas.wall_1_tr_111,atlas.w,atlas.h);break;
                    default:assert(false);
                }
                retMe.push_back({{x0+x+dX+.5, y0+y+dY, depth}, color, texel[0]});
                retMe.push_back({{x1+x+dX, y0+y+dY, depth}, color, texel[1]});
                retMe.push_back({{x1+x+dX, y1+y+dY-.5, depth}, color, texel[2]});
                retMe.push_back({{x0+x+dX+.5, y1+y+dY-.5, depth}, color, texel[3]});
                
                uint8_t br_mask = 0b0001'1100;
                switch( (*(wall_t*)b.data[x][y].cell_data).surroundings&br_mask ){
                    case 0b0000'0000: texel = get_tex_coords(atlas.wall_1_br_000,atlas.w,atlas.h);break;
                    case 0b0000'0100: texel = get_tex_coords(atlas.wall_1_br_001,atlas.w,atlas.h);break;
                    case 0b0000'1000: texel = get_tex_coords(atlas.wall_1_br_010,atlas.w,atlas.h);break;
                    case 0b0000'1100: texel = get_tex_coords(atlas.wall_1_br_011,atlas.w,atlas.h);break;
                    case 0b0001'0000: texel = get_tex_coords(atlas.wall_1_br_100,atlas.w,atlas.h);break;
                    case 0b0001'0100: texel = get_tex_coords(atlas.wall_1_br_101,atlas.w,atlas.h);break;
                    case 0b0001'1000: texel = get_tex_coords(atlas.wall_1_br_110,atlas.w,atlas.h);break;
                    case 0b0001'1100: texel = get_tex_coords(atlas.wall_1_br_111,atlas.w,atlas.h);break;
                    default:assert(false);
                }
                retMe.push_back({{x0+x+dX+.5, y0+y+dY+.5, depth}, color, texel[0]});
                retMe.push_back({{x1+x+dX, y0+y+dY+.5, depth}, color, texel[1]});
                retMe.push_back({{x1+x+dX, y1+y+dY, depth}, color, texel[2]});
                retMe.push_back({{x0+x+dX+.5, y1+y+dY, depth}, color, texel[3]});
                
                uint8_t bl_mask = 0b0111'0000;
                switch( (*(wall_t*)b.data[x][y].cell_data).surroundings&bl_mask ){
                    case 0b0000'0000: texel = get_tex_coords(atlas.wall_1_bl_000,atlas.w,atlas.h);break;
                    case 0b0001'0000: texel = get_tex_coords(atlas.wall_1_bl_001,atlas.w,atlas.h);break;
                    case 0b0010'0000: texel = get_tex_coords(atlas.wall_1_bl_010,atlas.w,atlas.h);break;
                    case 0b0011'0000: texel = get_tex_coords(atlas.wall_1_bl_011,atlas.w,atlas.h);break;
                    case 0b0100'0000: texel = get_tex_coords(atlas.wall_1_bl_100,atlas.w,atlas.h);break;
                    case 0b0101'0000: texel = get_tex_coords(atlas.wall_1_bl_101,atlas.w,atlas.h);break;
                    case 0b0110'0000: texel = get_tex_coords(atlas.wall_1_bl_110,atlas.w,atlas.h);break;
                    case 0b0111'0000: texel = get_tex_coords(atlas.wall_1_bl_111,atlas.w,atlas.h);break;
                    default:std::cout << (int)((*(wall_t*)b.data[x][y].cell_data).surroundings&bl_mask) << "\n";assert(false);
                }
                retMe.push_back({{x0+x+dX, y0+y+dY+.5, depth}, color, texel[0]});
                retMe.push_back({{x1+x+dX-.5, y0+y+dY+.5, depth}, color, texel[1]});
                retMe.push_back({{x1+x+dX-.5, y1+y+dY, depth}, color, texel[2]});
                retMe.push_back({{x0+x+dX, y1+y+dY, depth}, color, texel[3]});//*/
            }else{
                bool draw_floor = false;
                uint8_t floor_variation = 0;
                switch(b.data[x][y].type){
                    case tile_t::firepit_on  : floor_variation = (*(firepit_t*)b.data[x][y].cell_data).variation; draw_floor = true; break;
                    case tile_t::firepit_off : floor_variation = (*(firepit_t*)b.data[x][y].cell_data).variation; draw_floor = true; break;
                    case tile_t::door_open   : floor_variation = (*(door_t   *)b.data[x][y].cell_data).variation; draw_floor = true; break;
                    case tile_t::door_closed : floor_variation = (*(door_t   *)b.data[x][y].cell_data).variation; draw_floor = true; break;
                    case tile_t::exit        : floor_variation = (*(exit_t   *)b.data[x][y].cell_data).variation; draw_floor = true; break;
                    default: break;
                }
                
                if(draw_floor){
                    std::array<glm::vec2,4> floor_texel = get_tex_coords(atlas.floor[floor_variation&3],atlas.w,atlas.h);
                    retMe.push_back({{x0+x+dX, y0+y+dY, depth}, color, floor_texel[0]});
                    retMe.push_back({{x1+x+dX, y0+y+dY, depth}, color, floor_texel[1]});
                    retMe.push_back({{x1+x+dX, y1+y+dY, depth}, color, floor_texel[2]});
                    retMe.push_back({{x0+x+dX, y1+y+dY, depth}, color, floor_texel[3]});
                }
                
                
                retMe.push_back({{x0+x+dX, y0+y+dY, depth}, color, texel[0]});
                retMe.push_back({{x1+x+dX, y0+y+dY, depth}, color, texel[1]});
                retMe.push_back({{x1+x+dX, y1+y+dY, depth}, color, texel[2]});
                retMe.push_back({{x0+x+dX, y1+y+dY, depth}, color, texel[3]});
            }
            
            draw_char:;
            bool draw_heiroglyphics = false;
            uint8_t heiroglyphic = 0;
            switch(b.data[x][y].type){
                case tile_t::pit         : heiroglyphic = (*(pit_t    *)b.data[x][y].cell_data).character; draw_heiroglyphics = true; break;
                case tile_t::wall        : heiroglyphic = (*(wall_t   *)b.data[x][y].cell_data).character; draw_heiroglyphics = true; break;
                case tile_t::door_open   : heiroglyphic = (*(door_t   *)b.data[x][y].cell_data).character; draw_heiroglyphics = true; break;
                case tile_t::door_closed : heiroglyphic = (*(door_t   *)b.data[x][y].cell_data).character; draw_heiroglyphics = true; break;
                case tile_t::plate       : heiroglyphic = (*(plate_t  *)b.data[x][y].cell_data).character; draw_heiroglyphics = true; break;
                default: break;
            }
            
            if(draw_heiroglyphics&&heiroglyphic){
                std::array<glm::vec2,4> heiroglyphic_texel = get_tex_coords(atlas.heiroglyphics[heiroglyphic-1],atlas.w,atlas.h);
                retMe.push_back({{x0+x+dX, y0+y+dY, depth}, heiroglyphic_colors[0], heiroglyphic_texel[0]});
                retMe.push_back({{x1+x+dX, y0+y+dY, depth}, heiroglyphic_colors[0], heiroglyphic_texel[1]});
                retMe.push_back({{x1+x+dX, y1+y+dY, depth}, heiroglyphic_colors[0], heiroglyphic_texel[2]});
                retMe.push_back({{x0+x+dX, y1+y+dY, depth}, heiroglyphic_colors[0], heiroglyphic_texel[3]});
            }
            
            if(b.data[x][y].type==tile_t::plate){
                uint8_t ticks = ((*(plate_t*) b.data[x][y].cell_data).ticks_alive+3)/4;
                if(ticks>0){
                    uint8_t tick = ticks>4?4:ticks%4;
                    std::array<glm::vec2,4> timer_texel = get_tex_coords(atlas.plate_tick[tick%4],atlas.w,atlas.h);
                    retMe.push_back({{x0+x+dX, y0+y+dY, depth}, plate_colors[0], timer_texel[0]});
                    retMe.push_back({{x1+x+dX, y0+y+dY, depth}, plate_colors[0], timer_texel[1]});
                    retMe.push_back({{x1+x+dX, y1+y+dY, depth}, plate_colors[0], timer_texel[2]});
                    retMe.push_back({{x0+x+dX, y1+y+dY, depth}, plate_colors[0], timer_texel[3]});
                }
                
                if(ticks>4){
                    uint8_t tick = ticks>8?8:ticks%4;
                    std::array<glm::vec2,4> timer_texel = get_tex_coords(atlas.plate_tick[tick%4],atlas.w,atlas.h);
                    retMe.push_back({{x0+x+dX, y0+y+dY, depth}, plate_colors[1], timer_texel[0]});
                    retMe.push_back({{x1+x+dX, y0+y+dY, depth}, plate_colors[1], timer_texel[1]});
                    retMe.push_back({{x1+x+dX, y1+y+dY, depth}, plate_colors[1], timer_texel[2]});
                    retMe.push_back({{x0+x+dX, y1+y+dY, depth}, plate_colors[1], timer_texel[3]});
                }
                
                if(ticks>8){
                    uint8_t tick = ticks>12?12:ticks%4;
                    std::array<glm::vec2,4> timer_texel = get_tex_coords(atlas.plate_tick[tick%4],atlas.w,atlas.h);
                    retMe.push_back({{x0+x+dX, y0+y+dY, depth}, plate_colors[2], timer_texel[0]});
                    retMe.push_back({{x1+x+dX, y0+y+dY, depth}, plate_colors[2], timer_texel[1]});
                    retMe.push_back({{x1+x+dX, y1+y+dY, depth}, plate_colors[2], timer_texel[2]});
                    retMe.push_back({{x0+x+dX, y1+y+dY, depth}, plate_colors[2], timer_texel[3]});
                }
            }
            
            
            
        }
        
    }
    
    
    texel = get_tex_coords(atlas.player,atlas.w,atlas.h);
    retMe.push_back({{x0+b.player.x+dX, y0+b.player.y+dY, depth}, color, texel[0]});
    retMe.push_back({{x1+b.player.x+dX, y0+b.player.y+dY, depth}, color, texel[1]});
    retMe.push_back({{x1+b.player.x+dX, y1+b.player.y+dY, depth}, color, texel[2]});
    retMe.push_back({{x0+b.player.x+dX, y1+b.player.y+dY, depth}, color, texel[3]});
    
    texel = get_tex_coords(atlas.sax,atlas.w,atlas.h);
    for(const auto& sax:b.saxophones){
        retMe.push_back({{x0+sax.x+dX, y0+sax.y+dY, depth}, color, texel[0]});
        retMe.push_back({{x1+sax.x+dX, y0+sax.y+dY, depth}, color, texel[1]});
        retMe.push_back({{x1+sax.x+dX, y1+sax.y+dY, depth}, color, texel[2]});
        retMe.push_back({{x0+sax.x+dX, y1+sax.y+dY, depth}, color, texel[3]});
    }
    
    texel = get_tex_coords(atlas.drummer,atlas.w,atlas.h);
    for(const auto& drummer:b.drums){
        retMe.push_back({{x0+drummer.x+dX, y0+drummer.y+dY, depth}, color, texel[0]});
        retMe.push_back({{x1+drummer.x+dX, y0+drummer.y+dY, depth}, color, texel[1]});
        retMe.push_back({{x1+drummer.x+dX, y1+drummer.y+dY, depth}, color, texel[2]});
        retMe.push_back({{x0+drummer.x+dX, y1+drummer.y+dY, depth}, color, texel[3]});
    }
    
    texel = get_tex_coords(atlas.notes,atlas.w,atlas.h);
    for(const auto& notes:b.notes){
        retMe.push_back({{x0+notes.x+dX, y0+notes.y+dY, depth}, color, texel[0]});
        retMe.push_back({{x1+notes.x+dX, y0+notes.y+dY, depth}, color, texel[1]});
        retMe.push_back({{x1+notes.x+dX, y1+notes.y+dY, depth}, color, texel[2]});
        retMe.push_back({{x0+notes.x+dX, y1+notes.y+dY, depth}, color, texel[3]});
    }
    
    texel = get_tex_coords(atlas.demon,atlas.w,atlas.h);
    for(const auto& demon:b.demons){
        retMe.push_back({{x0+demon.x+dX, y0+demon.y+dY, depth}, color, texel[0]});
        retMe.push_back({{x1+demon.x+dX, y0+demon.y+dY, depth}, color, texel[1]});
        retMe.push_back({{x1+demon.x+dX, y1+demon.y+dY, depth}, color, texel[2]});
        retMe.push_back({{x0+demon.x+dX, y1+demon.y+dY, depth}, color, texel[3]});
    }
    
    texel = get_tex_coords(atlas.fireball,atlas.w,atlas.h);
    for(const auto& fireball:b.fireballs){
        retMe.push_back({{x0+fireball.x+dX, y0+fireball.y+dY, depth}, color, texel[0]});
        retMe.push_back({{x1+fireball.x+dX, y0+fireball.y+dY, depth}, color, texel[1]});
        retMe.push_back({{x1+fireball.x+dX, y1+fireball.y+dY, depth}, color, texel[2]});
        retMe.push_back({{x0+fireball.x+dX, y1+fireball.y+dY, depth}, color, texel[3]});
    }
    
    return retMe;
}

