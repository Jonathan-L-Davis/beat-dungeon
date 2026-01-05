#include "main.h"

#include "graphics/graphics.h"
#include "graphics/imgui/imgui.h"
#include "graphics/imgui/imgui_impl_sdl3.h"

#include <algorithm>
#include <vector>
#include <cmath>

bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void draw(){
    graphics::start_frame();
    
    ;
    
    graphics::end_frame();
}
