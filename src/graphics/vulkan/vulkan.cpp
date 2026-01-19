#include "vulkan.h"
#include "common.h"
#include "init.h"

#include "assert.h"

#include "graphics/imgui/imgui.h"
#include "graphics/imgui/imgui_impl_sdl3.h"
#include "graphics/imgui/imgui_impl_vulkan.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "util/stb/stb_image.h"

#include <set>

// validation layers
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

constexpr bool enableValidationLayers = false;

// required extensions
std::vector<const char*>        instance_extensions={
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    "VK_KHR_xlib_surface",
    "VK_KHR_surface",
    "VK_EXT_debug_utils"
};

std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// step 0, create instance
VkInstance                      instance = VK_NULL_HANDLE;
VkSurfaceKHR surface;// technically a later step in the tutorial, but happens right after instance creation, so eh
// step 1, initialize validation layers
VkDebugUtilsMessengerEXT debugMessenger;
// step 2, create the window surface created.)
SDL_Window* window;
// step 3, select physical device
VkPhysicalDevice                PhysicalDevice = VK_NULL_HANDLE;// active GPU
std::vector<VkPhysicalDevice>   gpus;// so users can pick a specific GPU if they wish.
// step 4, create logical device ( & queue )
VkDevice                        LogicalDevice = VK_NULL_HANDLE;
VkQueue                         GraphicsQueue = VK_NULL_HANDLE;
VkQueue                         PresentQueue  = VK_NULL_HANDLE;
uint32_t                        GraphicsQueueFamily = (uint32_t)-1;
uint32_t                        PresentQueueFamily = (uint32_t)-1;
// step 5, create the swap chain
VkSwapchainKHR swapChain;
std::vector<VkImage> swapChainImages;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;

uint32_t MinImageCount = 0;
uint32_t ImageCount = 0;
uint32_t MaxImageCount = 0;
// step 6, create image views
std::vector<VkImageView> swapChainImageViews;
// step 7, create graphics pipeline
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
VkRenderPass renderPass;
// step 8, create frame buffers
std::vector<VkFramebuffer> swapChainFramebuffers;
// step 9, create command buffers
VkCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;
// step 10, create threading primitives
std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
// step 11, multiple frames at once (changed semaphores and fences to be vectors, as well as the command buffer
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
// step 12, getting ready to use multiple vertices.
uint32_t num_idxs = 0;
// step 13, using a staging buffer.
VkBuffer stagingBuffer;
VkDeviceMemory stagingBufferMemory;
VkBuffer stagingBuffer2;
VkDeviceMemory stagingBufferMemory2;

VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;

VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;

// Uniform buffers n stuff
VkDescriptorSetLayout descriptorSetLayout;

VkDescriptorPool descriptorPool;
std::vector<VkDescriptorSet> descriptorSets;

std::vector<VkBuffer> uniformBuffers;
std::vector<VkDeviceMemory> uniformBuffersMemory;
std::vector<void*> uniformBuffersMapped;

uint32_t currentFrame = 0;
// step idfk, adding textures finally!!!
VkImage textureImage;
VkDeviceMemory textureImageMemory;
VkImageView textureImageView;
VkSampler textureSampler;

uint32_t MAX_VERTICES = 65536;

graphics::vulkan::UniformBufferObject ubo;

namespace graphics{
    namespace vulkan{
        
        VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            
            return bindingDescription;
        }
        
        std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
            
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);
            
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);
            
            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, uv);
            
            return attributeDescriptions;
        }//*/
        
        bool IsExtensionAvailable(const std::vector<VkExtensionProperties>& properties, const char* extension){
            for(const VkExtensionProperties& p : properties)
                if( std::string(p.extensionName) == std::string(extension) )// the temporary construction is obviously worse performing than strcmp, but I can read it better.
                    return true;
            return false;
        }
        
        bool initialize(){
            bool success = true;
            // Setup SDL
            // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
            {
                printf("Error: SDL_Init(): %s\n", SDL_GetError());
                return false;
            }
            
            // Create window with Vulkan graphics context
            float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
            SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
            window = SDL_CreateWindow(init_data.name.c_str(), (int)(1280 * main_scale), (int)(720 * main_scale), window_flags);
            if (window == nullptr)
            {
                printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
                return false;
            }
            
            std::vector<const char*> extensions;
            {
                uint32_t sdl_extensions_count = 0;
                const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
                for (uint32_t n = 0; n < sdl_extensions_count; n++)
                    extensions.push_back(sdl_extensions[n]);
            }
            
            success &= create_instance();
            success &= start_validation_layers();
            success &= create_vulkan_surface();
            success &= select_device();
            success &= create_logical_device();
            success &= create_swapchain();
            success &= create_image_views();
            success &= create_renderpass();
            success &= create_descriptor_set_layout();
            success &= create_pipeline();
            success &= create_framebuffers();
            success &= create_command_pool();
            success &= load_textures();
            success &= create_texture_views();
            success &= create_texture_sampler();
            success &= create_vertex_buffers();
            success &= create_index_buffers();
            success &= create_uniform_buffers();
            success &= create_descriptor_pool();
            success &= create_descriptor_sets();
            success &= create_command_buffers();
            success &= create_sync_objects();
            std::cout << "finished vulkan initialization.\n";
            
            //////////////////////////// Pure IMGUI set up after this point. Figure out which variables are needed and move them into a proper function.
            
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            
            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsLight();
            
            // Setup scaling
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
            style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
            
            // Setup Platform/Renderer backends
            ImGui_ImplSDL3_InitForVulkan(window);
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.ApiVersion = VK_API_VERSION_1_0;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
            init_info.Instance = instance;
            init_info.PhysicalDevice = PhysicalDevice;
            init_info.Device = LogicalDevice;
            init_info.QueueFamily = GraphicsQueueFamily;
            init_info.Queue = GraphicsQueue;
            
            {
            VkDescriptorPoolSize pool_sizes[] =
                {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
                };

                VkDescriptorPoolCreateInfo pool_info = {};
                pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
                pool_info.maxSets = 1000;
                pool_info.poolSizeCount = std::size(pool_sizes);
                pool_info.pPoolSizes = pool_sizes;

                VkDescriptorPool imguiPool;
                if(vkCreateDescriptorPool(LogicalDevice, &pool_info, nullptr, &imguiPool)){
                    std::cout << "Failing to create imgui descriptor pool\n";
                    return false;
                }
                init_info.DescriptorPool = imguiPool;
            }
            //init_info.PipelineCache = PipelineCache;
            init_info.RenderPass = renderPass;
            init_info.Subpass = 0;
            
            init_info.MinImageCount = MinImageCount;
            init_info.ImageCount = ImageCount;
            
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            
            init_info.Allocator = nullptr;
            //init_info.CheckVkResultFn = check_vk_result;
            ImGui_ImplVulkan_Init(&init_info);
            
            return success;
        }
        
        bool terminate(){
            VkResult err = vkDeviceWaitIdle(LogicalDevice);
            
            if(err!=VK_SUCCESS)
                return false;
            //*
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();//*/
            
            /// Vulkan backend clean up.
            
            vkDestroySampler(LogicalDevice, textureSampler, nullptr);
            vkDestroyImageView(LogicalDevice, textureImageView, nullptr);
            
            vkDestroyImage(LogicalDevice, textureImage, nullptr);
            vkFreeMemory(LogicalDevice, textureImageMemory, nullptr);
            
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroyBuffer(LogicalDevice, uniformBuffers[i], nullptr);
                vkFreeMemory(LogicalDevice, uniformBuffersMemory[i], nullptr);
            }
            
            vkDestroyDescriptorSetLayout(LogicalDevice, descriptorSetLayout, nullptr);
            
            for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
                vkDestroySemaphore(LogicalDevice, imageAvailableSemaphores[i], nullptr);
                vkDestroySemaphore(LogicalDevice, renderFinishedSemaphores[i], nullptr);
                vkDestroyFence(LogicalDevice, inFlightFences[i], nullptr);
            }
            
            vkDestroyCommandPool(LogicalDevice, commandPool, nullptr);
            
            terminate_swapchain();
            
            vkDestroyBuffer(LogicalDevice, vertexBuffer, nullptr);
            vkFreeMemory(LogicalDevice, vertexBufferMemory, nullptr);
            
            vkDestroyBuffer(LogicalDevice, stagingBuffer, nullptr);
            vkFreeMemory(LogicalDevice, stagingBufferMemory, nullptr);
            
            // SDL clean up.
            SDL_DestroyWindow(window);
            SDL_Quit();
            return true;
        }
        
        void start_frame(){
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
        }
        
        void end_frame(){
            
            ImGui::Render();
            
            vkWaitForFences(LogicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
            
            uint32_t imageIndex;
            auto err = vkAcquireNextImageKHR(LogicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
            
            if(err==VK_ERROR_OUT_OF_DATE_KHR||err==VK_SUBOPTIMAL_KHR){
                recreate_swapchain();
                return;// rendering is hopeless, abandon frame.
            }else if(err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR){
                std::cout << "Unhandled errors for acquiring an image.\n";
                return;
            }
            
            update_uniforms(currentFrame);
            
            vkResetFences(LogicalDevice, 1, &inFlightFences[currentFrame]);
            
            vkResetCommandBuffer(commandBuffers[currentFrame], 0);
            record_command_buffer(commandBuffers[currentFrame], imageIndex);
            
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            
            VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
            
            VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;
            
            if (vkQueueSubmit(GraphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
                assert(false&&"failed to submit draw command buffer!");
            }
            
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;
            
            VkSwapchainKHR swapChains[] = {swapChain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &imageIndex;
            presentInfo.pResults = nullptr; // Optional
            
            err = vkQueuePresentKHR(PresentQueue, &presentInfo);
            
            if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
                recreate_swapchain();
            } else if (err != VK_SUCCESS) {
                std::cout << "Rendering a frame failed!\n";
            }
            
            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
        
        bool create_instance(){
            bool success = true;
            
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = init_data.name.c_str();
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;
            appInfo.pNext = nullptr;
            
            
            // Enumerate available extensions
            uint32_t properties_count;
            std::vector<VkExtensionProperties> properties;
            vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
            properties.resize(properties_count);
            VkResult found_extensions = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data());
            if(found_extensions!=VK_SUCCESS){
                std::cout << "Required extension not enabled.\n";
                success = false;
            }
            
            // Enable required extensions
            for(auto ext : instance_extensions)
                if (IsExtensionAvailable(properties, ext))
                    instance_extensions.push_back(ext);
            
            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            
            createInfo.enabledExtensionCount = instance_extensions.size();
            createInfo.ppEnabledExtensionNames = instance_extensions.data();
            
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
                
                populateDebugMessengerCreateInfo(debugCreateInfo);
                
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
            } else {
                createInfo.enabledLayerCount = 0;

                createInfo.pNext = nullptr;
            }
            
            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
                success = false;
            if(!success) std::cout << "Why is vulkan instance creation failing?\n";
            return success;
        }
        
        bool start_validation_layers(){
            if (!enableValidationLayers) return true;

            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);

            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
                std::cout << "failed to set up debug messenger!\n";
                return false;
            }
            return true;
        }
        
        bool create_vulkan_surface(){
            if (SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface) == 0)
            {
                printf("Failed to create Vulkan surface.\n");
                std::cout << SDL_GetError() << "\n";
                return false;
            }
            
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            
            SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            SDL_ShowWindow(window);
            return true;
        }
        
        bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
            
            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
            
            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }
            
            return requiredExtensions.empty();
        }
        
        bool is_usable_device(VkPhysicalDevice device){
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
            
            return checkDeviceExtensionSupport(device) && supportedFeatures.samplerAnisotropy;
        }
        
        bool select_device(){
            
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
            
            if (deviceCount == 0) {
                std::cout << "Detected no GPUs with Vulkan support!\n";
                return false;
            }
            
            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
            
            for(const auto& D:devices){
                if(is_usable_device(D)){
                    if(PhysicalDevice==VK_NULL_HANDLE)// use the first good GPU, but track all the usable ones. We can populate a menu later for user selection.
                        PhysicalDevice = D;
                    gpus.push_back(D);
                }
            }
            
            if(PhysicalDevice==VK_NULL_HANDLE){
                std::cout << "Failed to find a GPU.\n";
                return false;
            }
            return true;
        }
        
        bool create_logical_device(){
            
            
            {
            
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, nullptr);

                std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, queueFamilies.data());

                int i = 0;
                for (const auto& queueFamily : queueFamilies) {
                    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        GraphicsQueueFamily = i;
                    }

                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, surface, &presentSupport);

                    if (presentSupport) {
                        PresentQueueFamily = i;
                    }

                    i++;
                }
            
            }
            
            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = {
                GraphicsQueueFamily,
                PresentQueueFamily
            };
            
            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }
            
            VkPhysicalDeviceFeatures deviceFeatures{};
            deviceFeatures.samplerAnisotropy = VK_TRUE;
            
            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.queueCreateInfoCount = queueCreateInfos.size();
            
            createInfo.pEnabledFeatures = &deviceFeatures;
            
            
            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
            }
            
            if (vkCreateDevice(PhysicalDevice, &createInfo, nullptr, &LogicalDevice) != VK_SUCCESS) {
                std::cout << "failed to create logical device!\n";
            }

            vkGetDeviceQueue(LogicalDevice, GraphicsQueueFamily, 0, &GraphicsQueue);
            vkGetDeviceQueue(LogicalDevice, PresentQueueFamily, 0, &PresentQueue);
            return true;
        }
        
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
            SwapChainSupportDetails details;
            
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
            
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
            
            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }
            
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
            
            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
            }
            
            return details;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
            for (const auto& availableFormat : availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }
            
            return availableFormats[0];
        }

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
            for (const auto& availablePresentMode : availablePresentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
            }
            
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {
                int width, height;
                SDL_GetWindowSize(window, &width, &height);
                
                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };
                
                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
                
                return actualExtent;
            }
        }
        
        bool create_swapchain(){
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(PhysicalDevice);
            
            VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
            VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
            
            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }
            
            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surface;
            
            MinImageCount = swapChainSupport.capabilities.minImageCount;
            MaxImageCount = swapChainSupport.capabilities.maxImageCount;
            ImageCount    = imageCount;
            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            
            uint32_t queueFamilyIndices[] = {GraphicsQueueFamily, PresentQueueFamily};
            
            if (GraphicsQueueFamily != PresentQueueFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            } else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }
            
            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;
            
            if (vkCreateSwapchainKHR(LogicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
                std::cout <<"failed to create swap chain!\n";
                return false;
            }
            
            vkGetSwapchainImagesKHR(LogicalDevice, swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(LogicalDevice, swapChain, &imageCount, swapChainImages.data());
            
            swapChainImageFormat = surfaceFormat.format;
            swapChainExtent = extent;
            
            return true;
        }

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VkImageView imageView;
            if (vkCreateImageView(LogicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
                std::cout << "failed to create image view!\n";
                assert(false&&"Need a refactor to handle this error properly.");
            }

            return imageView;
        }
        
        bool create_image_views(){
            swapChainImageViews.resize(swapChainImages.size());
            
            for (uint32_t i = 0; i < swapChainImages.size(); i++) {
                swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
            }
            return true;
        }
        
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
            for (VkFormat format : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(PhysicalDevice, format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                    return format;
                } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }

            assert(false&&"failed to find supported format!");
        }

        VkFormat findDepthFormat() {
            return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            );
        }
        
        bool create_renderpass(){
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = swapChainImageFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            
            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            
            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            
            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &colorAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;
            
            if (vkCreateRenderPass(LogicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                std::cout << "failed to create render pass!\n";
                return false;
            }
            return true;
        }
        
        bool create_descriptor_set_layout(){
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            
            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 1;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            
            std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();
            
            if (vkCreateDescriptorSetLayout(LogicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
                std::cout << "failed to create descriptor set layout!\n";
                return false;
            }
            
            return true;
        }
        
        VkShaderModule createShaderModule(const std::vector<uint8_t>& code) {
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
            
            VkShaderModule shaderModule;
            if (vkCreateShaderModule(LogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                assert(false&&"Failed to create shader module!");
            }
            
            return shaderModule;
        }
        
        bool create_pipeline(){
            auto vertShaderCode = load_file("res/shaders/vert.spv");
            auto fragShaderCode = load_file("res/shaders/frag.spv");

            VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
            VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            auto bindingDescription = getBindingDescription();
            auto attributeDescriptions = getAttributeDescriptions();
            
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;

            std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
            //pipelineLayoutInfo.pushConstantRangeCount = 0;

            if (vkCreatePipelineLayout(LogicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                std::cout << "failed to create pipeline layout!\n";
                return false;
            }

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.layout = pipelineLayout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
                std::cout << "failed to create graphics pipeline!\n";
                return false;
            }

            vkDestroyShaderModule(LogicalDevice, fragShaderModule, nullptr);
            vkDestroyShaderModule(LogicalDevice, vertShaderModule, nullptr);
            
            return true;
        }
        
        bool create_framebuffers(){
            swapChainFramebuffers.resize(swapChainImageViews.size());
            
            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                VkImageView attachments[] = {
                    swapChainImageViews[i]
                };
                
                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;
                
                if (vkCreateFramebuffer(LogicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                    std::cout << "failed to create framebuffer!\n";
                    return false;
                }
            }
            return true;
        }
        
        bool create_command_pool(){
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = GraphicsQueueFamily;
            
            if (vkCreateCommandPool(LogicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                std::cout << "failed to create command pool!\n";
                return false;
            }
            
            return true;
        }
        
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memProperties);
            
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            
            assert(false&&"Failed to find a memory type for vertex buffers.\n");
        }
        
        bool create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory){
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            
            if (vkCreateBuffer(LogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                std::cout << "failed to create vertex buffer!\n";
                return false;
            }
            
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(LogicalDevice, buffer, &memRequirements);
            
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
            
            if (vkAllocateMemory(LogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
                std::cout << "failed to allocate vertex buffer memory!\n";
                return false;
            }
            
            vkBindBufferMemory(LogicalDevice, buffer, bufferMemory, 0);
            
            return true;
        }
        
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size){
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;
            
            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(LogicalDevice, &allocInfo, &commandBuffer);
            
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            
            vkBeginCommandBuffer(commandBuffer, &beginInfo);
                
                VkBufferCopy copyRegion{};
                copyRegion.size = size;
                vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
                
            vkEndCommandBuffer(commandBuffer);
            
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            
            vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(GraphicsQueue);
            
            vkFreeCommandBuffers(LogicalDevice, commandPool, 1, &commandBuffer);
        }
        
        bool load_textures(){
            
            int texWidth, texHeight, texChannels;
            stbi_uc* pixels = stbi_load("res/atlas.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            VkDeviceSize imageSize = texWidth * texHeight * 4;
            
            if (!pixels) {
                std::cout << "failed to load texture image!\n";
                return false;
            }
            
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
            
            void* data;
            vkMapMemory(LogicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
                memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(LogicalDevice, stagingBufferMemory);
            
            stbi_image_free(pixels);
            
            createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
            
            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            
            vkDestroyBuffer(LogicalDevice, stagingBuffer, nullptr);
            vkFreeMemory(LogicalDevice, stagingBufferMemory, nullptr);
            
            return true;
        }
        
        bool create_texture_views(){
            textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
            return true;
        }
        
        bool create_texture_sampler(){
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(PhysicalDevice, &properties);
            
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            
            if (vkCreateSampler(LogicalDevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
                std::cout << "failed to create texture sampler!\n";
                return false;
            }
            
            return true;
        }
        
        bool create_vertex_buffers(){
            
            VkDeviceSize bufferSize = sizeof(graphics::vulkan::Vertex) * MAX_VERTICES;
            if(!create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory)){
                std::cout << "Failed to create staging buffer.\n";
                return false;
            }
            
            if(!create_buffer(
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    vertexBuffer,
                    vertexBufferMemory
            )){
                std::cout << "Failed to create vertex buffer.\n";
                return false;
            }
            
            /*
            vkDestroyBuffer(LogicalDevice, stagingBuffer, nullptr);
            vkFreeMemory(LogicalDevice, stagingBufferMemory, nullptr);//*/
            
            return true;
        }
        
        bool create_index_buffers(){
            
            VkDeviceSize bufferSize = 16 * (MAX_VERTICES);
            if(!create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer2, stagingBufferMemory2)){
                std::cout << "Failed to create staging buffer.\n";
                return false;
            }
            
            if(!create_buffer(
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    indexBuffer,
                    indexBufferMemory
            )){
                std::cout << "Failed to create index buffer.\n";
                return false;
            }
            
            return true;
        }
        
        bool create_uniform_buffers(){
            VkDeviceSize bufferSize = sizeof(UniformBufferObject);
            
            uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
            uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
            
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
                
                vkMapMemory(LogicalDevice, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
            }
            return true;
        }
        
        bool create_descriptor_pool(){
            std::array<VkDescriptorPoolSize, 2> poolSizes{};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            
            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            
            if (vkCreateDescriptorPool(LogicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
                std::cout << "failed to create descriptor pool!\n";
                return false;
            }
            return true;
        }
        
        bool create_descriptor_sets(){
            std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            allocInfo.pSetLayouts = layouts.data();

            descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
            if (vkAllocateDescriptorSets(LogicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
                std::cout << "failed to allocate descriptor sets!\n";
                return false;
            }

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = uniformBuffers[i];
                bufferInfo.offset = 0;
                bufferInfo.range = sizeof(UniformBufferObject);
                
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = textureImageView;
                imageInfo.sampler = textureSampler;
                
                std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
                
                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = descriptorSets[i];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &bufferInfo;
                
                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = descriptorSets[i];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pImageInfo = &imageInfo;
                
                vkUpdateDescriptorSets(LogicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
            return true;
        }
        
        bool create_command_buffers(){
            
            commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
            
            if (vkAllocateCommandBuffers(LogicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                std::cout << "failed to allocate command buffers!\n";
                return false;
            }
            return true;
        }
        
        bool create_sync_objects(){
            
            imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
            
            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            
            for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
                if (vkCreateSemaphore(LogicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                    vkCreateSemaphore(LogicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                    vkCreateFence(LogicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                    std::cout << "failed to create driver synchronization objects!\n";
                    return false;
                }
            }
            
            return true;
        }
        
        void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex){
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            
            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                std::cout << "failed to begin recording command buffer!\n";
                std::exit(-1);
            }
            
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChainExtent;
            
            VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;
            
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
            
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) swapChainExtent.width;
            viewport.height = (float) swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            
            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
            
            //copyBuffer(stagingBuffer,vertexBuffer,vertices.size()*sizeof(Vertex) );
            
            VkBuffer vertexBuffers[] = {vertexBuffer};
            VkDeviceSize offsets[] = {0};
            
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
            
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
            //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(num_idxs), 1, 0, 0, 0);
            
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            
            vkCmdEndRenderPass(commandBuffer);
            
            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                std::cout << "failed to record command buffer!\n";
                std::exit(-1);
            }
        }
        
        void recreate_swapchain(){
            vkDeviceWaitIdle(LogicalDevice);
            
            terminate_swapchain();
            
            create_swapchain();
            create_image_views();
            create_framebuffers();
        }
        
        void terminate_swapchain(){
            for (auto framebuffer : swapChainFramebuffers) {
                vkDestroyFramebuffer(LogicalDevice, framebuffer, nullptr);
            }

            for (auto imageView : swapChainImageViews) {
                vkDestroyImageView(LogicalDevice, imageView, nullptr);
            }

            vkDestroySwapchainKHR(LogicalDevice, swapChain, nullptr);
        }
        
        void update_uniforms(uint32_t currentImage){
            memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
        }
        
        bool createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(LogicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
                std::cout << "failed to create image!\n";
                return false;
            }
            
            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(LogicalDevice, image, &memRequirements);
            
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
            
            if (vkAllocateMemory(LogicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
                std::cout << "failed to allocate image memory!\n";
                return false;
            }

            vkBindImageMemory(LogicalDevice, image, imageMemory, 0);
            
            return true;
        }
        
        bool transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();
            
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            
            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;
            
            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else {
                std::cout << "unsupported layout transition!\n";
                return false;
            }
            
            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
            
            endSingleTimeCommands(commandBuffer);
            return false;
        }
        
        VkCommandBuffer beginSingleTimeCommands() {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;
            
            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(LogicalDevice, &allocInfo, &commandBuffer);
            
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            
            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            
            return commandBuffer;
        }
        
        void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
            vkEndCommandBuffer(commandBuffer);
            
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            
            vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(GraphicsQueue);
            
            vkFreeCommandBuffers(LogicalDevice, commandPool, 1, &commandBuffer);
        }
        
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();
            
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {
                width,
                height,
                1
            };
            
            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            
            endSingleTimeCommands(commandBuffer);
        }
        
        VkImageView createImageView(VkImage image, VkFormat format) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            
            VkImageView imageView;
            if (vkCreateImageView(LogicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
                std::cout << "failed to create image view!\n";
                assert(false&&"ah ah ah, I've counted your last day.\nNow die program.\n");
            }
            
            return imageView;
        }
        
        void upload_tris(std::vector<graphics::vulkan::Vertex> tris, std::vector<uint16_t> indices){
            assert(tris.size()<MAX_VERTICES);
            
            num_idxs = indices.size();
            if(num_idxs==0) return;
            
            void* vert_data;
            vkMapMemory(LogicalDevice, stagingBufferMemory, 0, tris.size()*sizeof(graphics::vulkan::Vertex), 0, &vert_data);
                memcpy(vert_data, tris.data(), (size_t) tris.size()*sizeof(graphics::vulkan::Vertex));
            vkUnmapMemory(LogicalDevice, stagingBufferMemory);
            
            copyBuffer(stagingBuffer,vertexBuffer,(uint32_t)tris.size()*sizeof(graphics::vulkan::Vertex));
            
            void* idx_data;
            vkMapMemory(LogicalDevice, stagingBufferMemory2, 0, 2*indices.size(), 0, &idx_data);
                memcpy(idx_data, indices.data(), (size_t) 2*indices.size());
            vkUnmapMemory(LogicalDevice, stagingBufferMemory2);
            
            copyBuffer(stagingBuffer2,indexBuffer,2*indices.size());
            
        }
        
    }
}



















