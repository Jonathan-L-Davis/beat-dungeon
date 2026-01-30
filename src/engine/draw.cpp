#include "main.h"
#include "io/keyboard.h"
#include "io/mouse.h"

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

void draw(){
    graphics::start_frame();
    SDL_FRect src,dst;
    SDL_RenderTexture(nullptr,nullptr,&src,&dst);
    
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
