#ifndef VULKAN_H
#define VULKAN_H

#include <set>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "graphics/imgui/imgui.h"
#include "graphics/imgui/imgui_impl_sdl3.h"
#include "graphics/imgui/imgui_impl_vulkan.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace graphics{
    namespace vulkan{
        bool initialize();
        bool terminate();
        void start_frame();
        void end_frame();
        
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };
        
        struct Vertex {
            glm::vec2 pos;
            glm::vec3 color;
        };
        
        struct UniformBufferObject {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };
        
        VkVertexInputBindingDescription getBindingDescription();
        std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
        
        bool create_instance();
        bool start_validation_layers();
        bool create_vulkan_surface();
        bool select_device();
        bool create_logical_device();
        bool create_swapchain();
        bool create_image_views();
        bool create_renderpass();
        bool create_descriptor_set_layout();
        bool create_pipeline();
        bool create_framebuffers();
        bool create_command_pool();
        bool create_vertex_buffers();
        bool create_index_buffers();
        bool create_uniform_buffers();
        bool create_descriptor_pool();
        bool create_descriptor_sets();
        bool create_command_buffers();
        bool create_sync_objects();
        
        bool create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        
        void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        
        void recreate_swapchain();
        void terminate_swapchain();
        
        void update_uniforms(uint32_t currentImage);
    }
}

#endif//VULKAN_H
