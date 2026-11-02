// Vulkan 1.3 port of black_hole.cpp: same simulation (sim.h), same compute ray tracer, same grid overlay.
// Uses dynamic rendering + synchronization2, so there are no render passes or framebuffers.
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan clip-space depth is 0..1
#include "sim.h"

#define VK(x) do { VkResult r_ = (x); if (r_ != VK_SUCCESS) { \
    cerr << #x << " failed: VkResult " << r_ << " (" << __FILE__ << ":" << __LINE__ << ")\n"; exit(EXIT_FAILURE); } } while (0)

static bool g_resized = false;

static vector<char> readFile(const char* path) {
    ifstream in(path, ios::binary | ios::ate);
    if (!in.is_open()) { cerr << "Failed to open " << path << " (run from the folder with the .spv files)\n"; exit(EXIT_FAILURE); }
    vector<char> data(size_t(in.tellg()));
    in.seekg(0);
    in.read(data.data(), data.size());
    return data;
}

struct Buf { VkBuffer buf = VK_NULL_HANDLE; VkDeviceMemory mem = VK_NULL_HANDLE; void* map = nullptr; };

struct App {
    GLFWwindow* window;
    const int WIDTH = 800, HEIGHT = 600;
    const uint32_t COMPUTE_WIDTH = 200, COMPUTE_HEIGHT = 150;

    VkInstance inst; VkSurfaceKHR surf; VkPhysicalDevice phys; VkDevice dev; uint32_t qfam; VkQueue q;
    VkSwapchainKHR sc = VK_NULL_HANDLE; VkFormat scFormat; VkExtent2D scExtent;
    vector<VkImage> scImages; vector<VkImageView> scViews; vector<VkSemaphore> renderDone;
    VkImage img; VkDeviceMemory imgMem; VkImageView imgView; VkSampler sampler;
    Buf camU, diskU, objU, gridV, gridI;
    uint32_t gridIndexCount = 0;
    VkDescriptorSetLayout dsl; VkPipelineLayout pl; VkDescriptorPool pool; VkDescriptorSet ds;
    VkPipeline computePipe, gridPipe, quadPipe;
    VkCommandPool cpool; VkCommandBuffer cmd; VkSemaphore imageAvail; VkFence fence;

    // ---------- helpers ----------
    uint32_t findMemType(uint32_t bits, VkMemoryPropertyFlags want) {
        VkPhysicalDeviceMemoryProperties mp; vkGetPhysicalDeviceMemoryProperties(phys, &mp);
        for (uint32_t i = 0; i < mp.memoryTypeCount; ++i)
            if ((bits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & want) == want) return i;
        cerr << "No suitable memory type\n"; exit(EXIT_FAILURE);
    }
    // shortcut: every buffer is host-visible + persistently mapped (tiny, rewritten each frame); no VMA, one allocation per buffer. Upgrade when buffers get big.
    Buf makeBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
        Buf b;
        VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bi.size = size; bi.usage = usage; bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK(vkCreateBuffer(dev, &bi, nullptr, &b.buf));
        VkMemoryRequirements mr; vkGetBufferMemoryRequirements(dev, b.buf, &mr);
        VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        ai.allocationSize = mr.size;
        ai.memoryTypeIndex = findMemType(mr.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK(vkAllocateMemory(dev, &ai, nullptr, &b.mem));
        VK(vkBindBufferMemory(dev, b.buf, b.mem, 0));
        VK(vkMapMemory(dev, b.mem, 0, VK_WHOLE_SIZE, 0, &b.map));
        return b;
    }
    VkShaderModule loadShader(const char* path) {
        auto code = readFile(path);
        VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        ci.codeSize = code.size(); ci.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule m; VK(vkCreateShaderModule(dev, &ci, nullptr, &m));
        return m;
    }
    void barrier(VkCommandBuffer c, VkImage image, VkImageLayout from, VkImageLayout to,
                 VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                 VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess) {
        VkImageMemoryBarrier2 b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        b.srcStageMask = srcStage; b.srcAccessMask = srcAccess; b.dstStageMask = dstStage; b.dstAccessMask = dstAccess;
        b.oldLayout = from; b.newLayout = to;
        b.srcQueueFamilyIndex = b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.image = image; b.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        VkDependencyInfo d{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        d.imageMemoryBarrierCount = 1; d.pImageMemoryBarriers = &b;
        vkCmdPipelineBarrier2(c, &d);
    }

    // ---------- init ----------
    App() {
        if (!glfwInit()) { cerr << "GLFW init failed\n"; exit(EXIT_FAILURE); }
        if (!glfwVulkanSupported()) { cerr << "GLFW: Vulkan loader/driver not found\n"; exit(EXIT_FAILURE); }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Black Hole (Vulkan)", nullptr, nullptr);
        if (!window) { cerr << "Failed to create GLFW window\n"; glfwTerminate(); exit(EXIT_FAILURE); }
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int, int) { g_resized = true; });

        createInstance();
        VK(glfwCreateWindowSurface(inst, window, nullptr, &surf));
        pickDevice();
        createSwapchain();
        createResources();
        createPipelines();

        VkCommandPoolCreateInfo pi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; pi.queueFamilyIndex = qfam;
        VK(vkCreateCommandPool(dev, &pi, nullptr, &cpool));
        VkCommandBufferAllocateInfo ci{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        ci.commandPool = cpool; ci.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ci.commandBufferCount = 1;
        VK(vkAllocateCommandBuffers(dev, &ci, &cmd));
        VkSemaphoreCreateInfo si{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VK(vkCreateSemaphore(dev, &si, nullptr, &imageAvail));
        VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK(vkCreateFence(dev, &fi, nullptr, &fence));
    }

    void createInstance() {
        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app.pApplicationName = "Black Hole"; app.apiVersion = VK_API_VERSION_1_3;
        uint32_t extCount; const char** exts = glfwGetRequiredInstanceExtensions(&extCount);
        VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        ci.pApplicationInfo = &app; ci.enabledExtensionCount = extCount; ci.ppEnabledExtensionNames = exts;
        const char* validation = "VK_LAYER_KHRONOS_validation";
#ifndef NDEBUG
        uint32_t n; vkEnumerateInstanceLayerProperties(&n, nullptr);
        vector<VkLayerProperties> layers(n); vkEnumerateInstanceLayerProperties(&n, layers.data());
        for (auto& l : layers) if (!strcmp(l.layerName, validation)) {
            ci.enabledLayerCount = 1; ci.ppEnabledLayerNames = &validation;
            cout << "[INFO] Vulkan validation layer ON\n";
        }
#endif
        VK(vkCreateInstance(&ci, nullptr, &inst));
    }

    void pickDevice() {
        uint32_t n = 0; vkEnumeratePhysicalDevices(inst, &n, nullptr);
        vector<VkPhysicalDevice> devs(n); vkEnumeratePhysicalDevices(inst, &n, devs.data());
        int bestScore = -1;
        for (auto d : devs) {
            VkPhysicalDeviceProperties p; vkGetPhysicalDeviceProperties(d, &p);
            if (p.apiVersion < VK_API_VERSION_1_3) continue;
            uint32_t qn; vkGetPhysicalDeviceQueueFamilyProperties(d, &qn, nullptr);
            vector<VkQueueFamilyProperties> qf(qn); vkGetPhysicalDeviceQueueFamilyProperties(d, &qn, qf.data());
            for (uint32_t i = 0; i < qn; ++i) {
                VkBool32 present = VK_FALSE; vkGetPhysicalDeviceSurfaceSupportKHR(d, i, surf, &present);
                if (!present || !(qf[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) || !(qf[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) continue;
                int score = p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 2 : p.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 1 : 0;
                if (score > bestScore) { bestScore = score; phys = d; qfam = i; }
                break;
            }
        }
        if (bestScore < 0) { cerr << "No Vulkan 1.3 GPU with graphics+compute+present queue\n"; exit(EXIT_FAILURE); }
        VkPhysicalDeviceProperties p; vkGetPhysicalDeviceProperties(phys, &p);
        cout << "Vulkan device: " << p.deviceName << "\n";

        float prio = 1.0f;
        VkDeviceQueueCreateInfo qi{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        qi.queueFamilyIndex = qfam; qi.queueCount = 1; qi.pQueuePriorities = &prio;
        VkPhysicalDeviceVulkan13Features f13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
        f13.dynamicRendering = VK_TRUE; f13.synchronization2 = VK_TRUE;
        const char* devExt = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        VkDeviceCreateInfo di{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        di.pNext = &f13; di.queueCreateInfoCount = 1; di.pQueueCreateInfos = &qi;
        di.enabledExtensionCount = 1; di.ppEnabledExtensionNames = &devExt;
        VK(vkCreateDevice(phys, &di, nullptr, &dev));
        vkGetDeviceQueue(dev, qfam, 0, &q);
    }

    void createSwapchain() {
        VkSurfaceCapabilitiesKHR caps; VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys, surf, &caps));
        uint32_t n; vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surf, &n, nullptr);
        vector<VkSurfaceFormatKHR> fmts(n); vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surf, &n, fmts.data());
        // Only SRGB_NONLINEAR colorspaces: others need VK_EXT_swapchain_colorspace. UNORM format (not *_SRGB): the GL default framebuffer also wrote raw values.
        VkSurfaceFormatKHR chosen = fmts[0];
        for (auto& f : fmts) if (f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { chosen = f; if (f.format == VK_FORMAT_B8G8R8A8_UNORM) break; }
        scFormat = chosen.format;

        if (caps.currentExtent.width != UINT32_MAX) scExtent = caps.currentExtent;
        else {
            int w, h; glfwGetFramebufferSize(window, &w, &h);
            scExtent = { glm::clamp(uint32_t(w), caps.minImageExtent.width, caps.maxImageExtent.width),
                         glm::clamp(uint32_t(h), caps.minImageExtent.height, caps.maxImageExtent.height) };
        }
        uint32_t count = caps.minImageCount + 1;
        if (caps.maxImageCount && count > caps.maxImageCount) count = caps.maxImageCount;

        VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        ci.surface = surf; ci.minImageCount = count; ci.imageFormat = scFormat; ci.imageColorSpace = chosen.colorSpace;
        ci.imageExtent = scExtent; ci.imageArrayLayers = 1; ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; ci.preTransform = caps.currentTransform;
        ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        ci.presentMode = VK_PRESENT_MODE_FIFO_KHR; // always available, vsync
        ci.clipped = VK_TRUE;
        VK(vkCreateSwapchainKHR(dev, &ci, nullptr, &sc));

        vkGetSwapchainImagesKHR(dev, sc, &n, nullptr);
        scImages.resize(n); vkGetSwapchainImagesKHR(dev, sc, &n, scImages.data());
        scViews.resize(n); renderDone.resize(n);
        for (uint32_t i = 0; i < n; ++i) {
            VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            vi.image = scImages[i]; vi.viewType = VK_IMAGE_VIEW_TYPE_2D; vi.format = scFormat;
            vi.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            VK(vkCreateImageView(dev, &vi, nullptr, &scViews[i]));
            VkSemaphoreCreateInfo si{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VK(vkCreateSemaphore(dev, &si, nullptr, &renderDone[i])); // one per swapchain image: present may still hold it
        }
    }
    void destroySwapchain() {
        for (auto s : renderDone) vkDestroySemaphore(dev, s, nullptr);
        for (auto v : scViews) vkDestroyImageView(dev, v, nullptr);
        vkDestroySwapchainKHR(dev, sc, nullptr);
    }
    void recreateSwapchain() {
        int w = 0, h = 0; glfwGetFramebufferSize(window, &w, &h);
        while (w == 0 || h == 0) { glfwWaitEvents(); glfwGetFramebufferSize(window, &w, &h); } // minimized
        vkDeviceWaitIdle(dev);
        destroySwapchain();
        createSwapchain();
        g_resized = false;
    }

    void createResources() {
        // ray tracer output: written by compute, sampled by the quad. Fixed 200x150 (both branches of cam.moving used the same size).
        VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        ii.imageType = VK_IMAGE_TYPE_2D; ii.format = VK_FORMAT_R8G8B8A8_UNORM; ii.extent = {COMPUTE_WIDTH, COMPUTE_HEIGHT, 1};
        ii.mipLevels = 1; ii.arrayLayers = 1; ii.samples = VK_SAMPLE_COUNT_1_BIT; ii.tiling = VK_IMAGE_TILING_OPTIMAL;
        ii.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VK(vkCreateImage(dev, &ii, nullptr, &img));
        VkMemoryRequirements mr; vkGetImageMemoryRequirements(dev, img, &mr);
        VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        ai.allocationSize = mr.size; ai.memoryTypeIndex = findMemType(mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK(vkAllocateMemory(dev, &ai, nullptr, &imgMem));
        VK(vkBindImageMemory(dev, img, imgMem, 0));
        VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        vi.image = img; vi.viewType = VK_IMAGE_VIEW_TYPE_2D; vi.format = ii.format;
        vi.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        VK(vkCreateImageView(dev, &vi, nullptr, &imgView));
        VkSamplerCreateInfo sp{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        sp.magFilter = sp.minFilter = VK_FILTER_LINEAR;
        sp.addressModeU = sp.addressModeV = sp.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        VK(vkCreateSampler(dev, &sp, nullptr, &sampler));

        camU  = makeBuffer(128, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        diskU = makeBuffer(16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        objU  = makeBuffer(16 + 16 * 16 * 2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        const size_t nVerts = size_t(GRID_SIZE + 1) * (GRID_SIZE + 1);
        gridV = makeBuffer(nVerts * sizeof(vec3), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        vector<vec3> v; vector<uint32_t> idx;
        buildGrid(objects, v, idx);
        gridIndexCount = uint32_t(idx.size());
        gridI = makeBuffer(idx.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        memcpy(gridI.map, idx.data(), idx.size() * sizeof(uint32_t)); // indices never change

        // one set / one layout shared by the compute and graphics pipelines
        VkDescriptorSetLayoutBinding b[5] = {};
        b[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
        for (uint32_t i = 1; i <= 3; ++i) b[i] = {i, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
        b[4] = {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        li.bindingCount = 5; li.pBindings = b;
        VK(vkCreateDescriptorSetLayout(dev, &li, nullptr, &dsl));
        VkPushConstantRange pc{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4)};
        VkPipelineLayoutCreateInfo pli{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pli.setLayoutCount = 1; pli.pSetLayouts = &dsl; pli.pushConstantRangeCount = 1; pli.pPushConstantRanges = &pc;
        VK(vkCreatePipelineLayout(dev, &pli, nullptr, &pl));

        VkDescriptorPoolSize ps[3] = {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
        VkDescriptorPoolCreateInfo dpi{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        dpi.maxSets = 1; dpi.poolSizeCount = 3; dpi.pPoolSizes = ps;
        VK(vkCreateDescriptorPool(dev, &dpi, nullptr, &pool));
        VkDescriptorSetAllocateInfo dai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        dai.descriptorPool = pool; dai.descriptorSetCount = 1; dai.pSetLayouts = &dsl;
        VK(vkAllocateDescriptorSets(dev, &dai, &ds));

        // The image stays in GENERAL for both the storage write and the sampled read.
        VkDescriptorImageInfo storage{VK_NULL_HANDLE, imgView, VK_IMAGE_LAYOUT_GENERAL};
        VkDescriptorImageInfo sampled{sampler, imgView, VK_IMAGE_LAYOUT_GENERAL};
        VkDescriptorBufferInfo bi[3] = {{camU.buf, 0, 128}, {diskU.buf, 0, 16}, {objU.buf, 0, 16 + 16 * 16 * 2}};
        VkWriteDescriptorSet w[5] = {};
        for (auto& x : w) { x.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; x.dstSet = ds; x.descriptorCount = 1; }
        w[0].dstBinding = 0; w[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; w[0].pImageInfo = &storage;
        for (uint32_t i = 0; i < 3; ++i) { w[i + 1].dstBinding = i + 1; w[i + 1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; w[i + 1].pBufferInfo = &bi[i]; }
        w[4].dstBinding = 4; w[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; w[4].pImageInfo = &sampled;
        vkUpdateDescriptorSets(dev, 5, w, 0, nullptr);
    }

    VkPipeline makeGraphicsPipeline(const char* vert, const char* frag, VkPrimitiveTopology topo, bool vertexInput) {
        VkShaderModule vs = loadShader(vert), fs = loadShader(frag);
        VkPipelineShaderStageCreateInfo st[2] = {{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}, {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}};
        st[0].stage = VK_SHADER_STAGE_VERTEX_BIT; st[0].module = vs; st[0].pName = "main";
        st[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; st[1].module = fs; st[1].pName = "main";
        VkVertexInputBindingDescription vb{0, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX};
        VkVertexInputAttributeDescription va{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
        VkPipelineVertexInputStateCreateInfo vin{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        if (vertexInput) { vin.vertexBindingDescriptionCount = 1; vin.pVertexBindingDescriptions = &vb; vin.vertexAttributeDescriptionCount = 1; vin.pVertexAttributeDescriptions = &va; }
        VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        ia.topology = topo;
        VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        vp.viewportCount = 1; vp.scissorCount = 1; // set dynamically
        VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rs.polygonMode = VK_POLYGON_MODE_FILL; rs.cullMode = VK_CULL_MODE_NONE; rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; rs.lineWidth = 1.0f;
        VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        // GL left GL_BLEND on from drawGrid, so the quad (alpha 0 where rays escape) showed the grid behind it. Blend is explicit here.
        VkPipelineColorBlendAttachmentState ba{};
        ba.blendEnable = VK_TRUE;
        ba.srcColorBlendFactor = ba.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        ba.dstColorBlendFactor = ba.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        ba.colorBlendOp = ba.alphaBlendOp = VK_BLEND_OP_ADD;
        ba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        cb.attachmentCount = 1; cb.pAttachments = &ba;
        VkDynamicState dyn[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo ds2{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        ds2.dynamicStateCount = 2; ds2.pDynamicStates = dyn;
        VkPipelineRenderingCreateInfo rendering{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
        rendering.colorAttachmentCount = 1; rendering.pColorAttachmentFormats = &scFormat;

        VkGraphicsPipelineCreateInfo gi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        gi.pNext = &rendering; gi.stageCount = 2; gi.pStages = st; gi.pVertexInputState = &vin; gi.pInputAssemblyState = &ia;
        gi.pViewportState = &vp; gi.pRasterizationState = &rs; gi.pMultisampleState = &ms; gi.pColorBlendState = &cb;
        gi.pDynamicState = &ds2; gi.layout = pl;
        VkPipeline p; VK(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &gi, nullptr, &p));
        vkDestroyShaderModule(dev, vs, nullptr); vkDestroyShaderModule(dev, fs, nullptr);
        return p;
    }
    void createPipelines() {
        VkShaderModule cs = loadShader("geodesic_vk.comp.spv");
        VkComputePipelineCreateInfo ci{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        ci.stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_COMPUTE_BIT, cs, "main", nullptr};
        ci.layout = pl;
        VK(vkCreateComputePipelines(dev, VK_NULL_HANDLE, 1, &ci, nullptr, &computePipe));
        vkDestroyShaderModule(dev, cs, nullptr);
        gridPipe = makeGraphicsPipeline("grid_vk.vert.spv", "grid_vk.frag.spv", VK_PRIMITIVE_TOPOLOGY_LINE_LIST, true);
        quadPipe = makeGraphicsPipeline("quad_vk.vert.spv", "quad_vk.frag.spv", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);
    }

    // ---------- per frame ----------
    void uploadUBOs(const Camera& cam) {
        struct { vec3 pos; float _p0; vec3 right; float _p1; vec3 up; float _p2; vec3 forward; float _p3;
                 float tanHalfFov; float aspect; int moving; int _p4; } c;
        vec3 fwd = normalize(cam.target - cam.position());
        vec3 up = vec3(0, 1, 0); // y axis is up, so disk is in x-z plane
        vec3 right = normalize(cross(fwd, up));
        up = cross(right, fwd);
        c.pos = cam.position(); c.right = right; c.up = up; c.forward = fwd;
        c.tanHalfFov = tan(radians(60.0f * 0.5f));
        c.aspect = float(WIDTH) / float(HEIGHT);
        c.moving = (cam.dragging || cam.panning) ? 1 : 0;
        memcpy(camU.map, &c, sizeof(c));

        float disk[4] = { float(SagA.r_s) * 2.2f, float(SagA.r_s) * 5.2f, 2.0f, 1e9f }; // r1, r2, num, thickness
        memcpy(diskU.map, disk, sizeof(disk));

        struct { int numObjects; float _p[3]; vec4 posRadius[16]; vec4 color[16]; } o{};
        size_t count = std::min(objects.size(), size_t(16));
        o.numObjects = int(count);
        for (size_t i = 0; i < count; ++i) { o.posRadius[i] = objects[i].posRadius; o.color[i] = objects[i].color; }
        memcpy(objU.map, &o, sizeof(o));

        vector<vec3> v; vector<uint32_t> idx;
        buildGrid(objects, v, idx);
        memcpy(gridV.map, v.data(), v.size() * sizeof(vec3));
    }

    void record(uint32_t i) {
        VK(vkResetCommandBuffer(cmd, 0));
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK(vkBeginCommandBuffer(cmd, &bi));

        // 1) ray tracer -> storage image (previous contents are fully overwritten, so UNDEFINED is fine)
        barrier(cmd, img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, 0, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, computePipe);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pl, 0, 1, &ds, 0, nullptr);
        vkCmdDispatch(cmd, (COMPUTE_WIDTH + 15) / 16, (COMPUTE_HEIGHT + 15) / 16, 1);
        barrier(cmd, img, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        // 2) grid, then the ray-traced quad blended on top
        barrier(cmd, scImages[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
        VkRenderingAttachmentInfo ca{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
        ca.imageView = scViews[i]; ca.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        ca.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; ca.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        ca.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        VkRenderingInfo ri{VK_STRUCTURE_TYPE_RENDERING_INFO};
        ri.renderArea = {{0, 0}, scExtent}; ri.layerCount = 1; ri.colorAttachmentCount = 1; ri.pColorAttachments = &ca;
        vkCmdBeginRendering(cmd, &ri);
        VkViewport vp{0, 0, float(scExtent.width), float(scExtent.height), 0.0f, 1.0f};
        VkRect2D sc2{{0, 0}, scExtent};
        vkCmdSetViewport(cmd, 0, 1, &vp);
        vkCmdSetScissor(cmd, 0, 1, &sc2);

        mat4 view = lookAt(camera.position(), camera.target, vec3(0, 1, 0));
        mat4 proj = perspective(radians(60.0f), float(COMPUTE_WIDTH) / COMPUTE_HEIGHT, 1e9f, 1e14f);
        proj[1][1] *= -1.0f; // Vulkan clip-space y points down
        mat4 viewProj = proj * view;
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gridPipe);
        vkCmdPushConstants(cmd, pl, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &viewProj);
        VkDeviceSize off = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &gridV.buf, &off);
        vkCmdBindIndexBuffer(cmd, gridI.buf, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, gridIndexCount, 1, 0, 0, 0);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, quadPipe);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pl, 0, 1, &ds, 0, nullptr);
        vkCmdDraw(cmd, 6, 1, 0, 0);
        vkCmdEndRendering(cmd);

        barrier(cmd, scImages[i], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_NONE, 0);
        VK(vkEndCommandBuffer(cmd));
    }

    // shortcut: 1 frame in flight (fence wait at the top). Per-frame UBO copies would let the CPU run ahead; not worth it here.
    void frame() {
        VK(vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX));
        uint32_t i;
        VkResult r = vkAcquireNextImageKHR(dev, sc, UINT64_MAX, imageAvail, VK_NULL_HANDLE, &i);
        if (r == VK_ERROR_OUT_OF_DATE_KHR) { recreateSwapchain(); return; }
        if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) VK(r);
        VK(vkResetFences(dev, 1, &fence));

        uploadUBOs(camera);
        record(i);

        VkPipelineStageFlags wait = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        si.waitSemaphoreCount = 1; si.pWaitSemaphores = &imageAvail; si.pWaitDstStageMask = &wait;
        si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
        si.signalSemaphoreCount = 1; si.pSignalSemaphores = &renderDone[i];
        VK(vkQueueSubmit(q, 1, &si, fence));

        VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        pi.waitSemaphoreCount = 1; pi.pWaitSemaphores = &renderDone[i];
        pi.swapchainCount = 1; pi.pSwapchains = &sc; pi.pImageIndices = &i;
        r = vkQueuePresentKHR(q, &pi);
        if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR || g_resized) recreateSwapchain();
        else VK(r);
    }
};

// -- MAIN -- //
int main() {
    App app;
    setupCameraCallbacks(app.window);
    while (!glfwWindowShouldClose(app.window)) {
        glfwPollEvents();
        stepGravity();
        app.frame();
    }
    vkDeviceWaitIdle(app.dev);
    // shortcut: no explicit vkDestroy* teardown, the process exit reclaims everything. Add it if App ever gets created more than once.
    glfwDestroyWindow(app.window);
    glfwTerminate();
    return 0;
}
