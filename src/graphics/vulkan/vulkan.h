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
            glm::vec3 pos;
            glm::vec3 color;
            glm::vec2 uv;
        };
        
        struct UniformBufferObject {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };
        
        VkVertexInputBindingDescription getBindingDescription();
        std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
        
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
        bool load_textures();
        bool create_texture_views();
        bool create_texture_sampler();
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
        bool createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        bool transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        VkImageView createImageView(VkImage image, VkFormat format);
        
        void upload_tris(std::vector<graphics::vulkan::Vertex> tris, std::vector<uint16_t> indices);
        
    }
}

#endif//VULKAN_H
