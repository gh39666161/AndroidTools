#include <jni.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdint>

#include <android/log.h>
#include <vulkan/vulkan.h>

#include "spirv_reflect/spirv_reflect.h"

#define LOG_TAG "VulkanContext"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {

// Minimal Vulkan state kept alive between JNI calls. A single global instance is
// used because this sample drives everything from one worker thread.
struct VulkanState {
    VkInstance               instance       = VK_NULL_HANDLE;
    VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;
    VkDevice                 device         = VK_NULL_HANDLE;
    uint32_t                 queueFamily    = 0;
    VkQueue                  queue          = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    bool                     initialized    = false;
};

VulkanState g_vk;

// When non-null, the debug messenger callback also appends messages here so they
// can be surfaced in the string returned to Java (and shown in the on-screen log).
std::ostringstream *g_vkMessageSink = nullptr;

const char *severityName(VkDebugUtilsMessageSeverityFlagBitsEXT s) {
    if (s & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)   return "ERROR";
    if (s & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return "WARN";
    if (s & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)    return "INFO";
    return "VERBOSE";
}

// Debug messenger callback: the validation layers / driver call this with the
// exact human-readable reason behind failures (including VK_ERROR_INITIALIZATION_FAILED).
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT /* types */,
        const VkDebugUtilsMessengerCallbackDataEXT *data,
        void * /* userData */) {
    const char *msg = (data && data->pMessage) ? data->pMessage : "(null)";
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        LOGE("[VK] %s", msg);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "[VK] %s", msg);
    } else {
        LOGI("[VK] %s", msg);
    }
    // Mirror into the active sink so the message reaches the UI log too.
    if (g_vkMessageSink != nullptr) {
        *g_vkMessageSink << "  [VK " << severityName(severity) << "] " << msg << "\n";
    }
    return VK_FALSE;
}

void fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &info) {
    info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debugCallback;
}

// Destroys the debug messenger (if any) and the instance, resetting handles.
void cleanupInstance() {
    if (g_vk.debugMessenger != VK_NULL_HANDLE && g_vk.instance != VK_NULL_HANDLE) {
        auto pfnDestroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(g_vk.instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (pfnDestroy != nullptr) {
            pfnDestroy(g_vk.instance, g_vk.debugMessenger, nullptr);
        }
        g_vk.debugMessenger = VK_NULL_HANDLE;
    }
    if (g_vk.instance != VK_NULL_HANDLE) {
        vkDestroyInstance(g_vk.instance, nullptr);
        g_vk.instance = VK_NULL_HANDLE;
    }
}

const char *vkResultName(VkResult r) {
    switch (r) {
        case VK_SUCCESS:                        return "VK_SUCCESS";
        case VK_NOT_READY:                      return "VK_NOT_READY";
        case VK_TIMEOUT:                        return "VK_TIMEOUT";
        case VK_INCOMPLETE:                     return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:    return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:              return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_INVALID_SHADER_NV:        return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_EXTENSION_NOT_PRESENT:    return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:      return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:      return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_LAYER_NOT_PRESENT:        return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_TOO_MANY_OBJECTS:         return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:     return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        default:                                return "VK_<other>";
    }
}

// Copies a Java byte[] of SPIR-V into a word-aligned vector. Returns false and
// appends a reason to `log` on failure.
bool readSpirvWords(JNIEnv *env, jbyteArray spirvBytes, const char *stageName,
                    std::vector<uint32_t> &outCode, std::ostringstream &log) {
    if (spirvBytes == nullptr) {
        log << "FAIL " << stageName << ": spirv byte array is null\n";
        return false;
    }
    const jsize byteLen = env->GetArrayLength(spirvBytes);
    if (byteLen == 0) {
        log << "FAIL " << stageName << ": spirv byte array is empty\n";
        return false;
    }
    if ((byteLen % 4) != 0) {
        log << "FAIL " << stageName << ": spirv length (" << byteLen
            << ") is not a multiple of 4\n";
        return false;
    }
    outCode.resize(static_cast<size_t>(byteLen) / 4);
    env->GetByteArrayRegion(spirvBytes, 0, byteLen,
                            reinterpret_cast<jbyte *>(outCode.data()));
    if (outCode[0] != 0x07230203u) {
        log << "WARN " << stageName << ": unexpected SPIR-V magic 0x"
            << std::hex << outCode[0] << std::dec << " (expected 0x07230203)\n";
    }
    return true;
}

// Builds a VkShaderModule from SPIR-V words. On failure returns VK_NULL_HANDLE
// and appends a reason to `log`.
VkShaderModule createShaderModuleFromWords(const std::vector<uint32_t> &code,
                                           const char *stageName,
                                           std::ostringstream &log) {
    VkShaderModuleCreateInfo smInfo{};
    smInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smInfo.codeSize = code.size() * sizeof(uint32_t);
    smInfo.pCode    = code.data();

    VkShaderModule module = VK_NULL_HANDLE;
    VkResult res = vkCreateShaderModule(g_vk.device, &smInfo, nullptr, &module);
    if (res != VK_SUCCESS) {
        log << "FAIL " << stageName << ": vkCreateShaderModule "
            << vkResultName(res) << " (" << res << ")\n";
        return VK_NULL_HANDLE;
    }
    log << "OK " << stageName << ": vkCreateShaderModule ("
        << code.size() << " words, " << (code.size() * 4) << " bytes)\n";
    return module;
}

// Size in bytes of the vertex-attribute VkFormats that SPIRV-Reflect emits.
// Only the formats produced for vertex inputs are covered; defaults to 16.
uint32_t formatSizeBytes(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:          return 4;
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:       return 8;
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:    return 12;
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
        default:                            return 16;
    }
}

// Reflected interface of one or more shader stages, used to build a pipeline
// layout and render pass that are compatible with what the SPIR-V declares.
struct ReflectedInterface {
    // set -> (binding -> binding info). Merged across stages; stageFlags OR'd.
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> sets;
    std::vector<VkPushConstantRange> pushConstants;
    uint32_t maxColorOutputLocation = 0;   // highest fragment output location seen
    bool     hasFragmentOutputs     = false;
    // Vertex shader input attributes (location + format), needed so the pipeline's
    // vertex input state matches what the vertex shader consumes.
    std::vector<VkVertexInputAttributeDescription> vertexAttributes;
    // Actual SPIR-V entry point names per stage. UE mangles "main" into names like
    // "main_00003704_3d846d82", so the pipeline must use the reflected name.
    std::string vertexEntryPoint;
    std::string fragmentEntryPoint;
};

const char *descriptorTypeName(VkDescriptorType t) {
    switch (t) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:                return "SAMPLER";
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return "COMBINED_IMAGE_SAMPLER";
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:          return "SAMPLED_IMAGE";
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:          return "STORAGE_IMAGE";
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:   return "UNIFORM_TEXEL_BUFFER";
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:   return "STORAGE_TEXEL_BUFFER";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:         return "UNIFORM_BUFFER";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:         return "STORAGE_BUFFER";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return "UNIFORM_BUFFER_DYNAMIC";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return "STORAGE_BUFFER_DYNAMIC";
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:       return "INPUT_ATTACHMENT";
        default:                                        return "OTHER";
    }
}

const char *formatName(VkFormat f) {
    switch (f) {
        case VK_FORMAT_R32_SFLOAT:          return "R32_SFLOAT";
        case VK_FORMAT_R32G32_SFLOAT:       return "R32G32_SFLOAT";
        case VK_FORMAT_R32G32B32_SFLOAT:    return "R32G32B32_SFLOAT";
        case VK_FORMAT_R32G32B32A32_SFLOAT: return "R32G32B32A32_SFLOAT";
        case VK_FORMAT_R32_UINT:            return "R32_UINT";
        case VK_FORMAT_R32G32_UINT:         return "R32G32_UINT";
        case VK_FORMAT_R32G32B32_UINT:      return "R32G32B32_UINT";
        case VK_FORMAT_R32G32B32A32_UINT:   return "R32G32B32A32_UINT";
        case VK_FORMAT_R32_SINT:            return "R32_SINT";
        case VK_FORMAT_R32G32_SINT:         return "R32G32_SINT";
        case VK_FORMAT_R32G32B32_SINT:      return "R32G32B32_SINT";
        case VK_FORMAT_R32G32B32A32_SINT:   return "R32G32B32A32_SINT";
        default:                            return "OTHER";
    }
}

// Reflects one SPIR-V stage and merges its descriptor bindings / push constants
// / fragment output locations into `out`. Returns false on reflection failure.
bool reflectStage(const std::vector<uint32_t> &code, VkShaderStageFlagBits stage,
                  const char *stageName, ReflectedInterface &out,
                  std::ostringstream &log) {
    SpvReflectShaderModule rmod;
    SpvReflectResult r = spvReflectCreateShaderModule(
            code.size() * sizeof(uint32_t), code.data(), &rmod);
    if (r != SPV_REFLECT_RESULT_SUCCESS) {
        log << "FAIL " << stageName << ": spvReflectCreateShaderModule (" << r << ")\n";
        return false;
    }

    // Capture the real entry point name (UE mangles "main").
    const char *entry = rmod.entry_point_name ? rmod.entry_point_name : "main";
    if (stage == VK_SHADER_STAGE_VERTEX_BIT)   out.vertexEntryPoint   = entry;
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) out.fragmentEntryPoint = entry;
    log << "entry point " << stageName << ": \"" << entry << "\"\n";

    // Descriptor bindings.
    uint32_t bindingCount = 0;
    spvReflectEnumerateDescriptorBindings(&rmod, &bindingCount, nullptr);
    std::vector<SpvReflectDescriptorBinding *> bindings(bindingCount);
    if (bindingCount > 0) {
        spvReflectEnumerateDescriptorBindings(&rmod, &bindingCount, bindings.data());
    }
    for (auto *b : bindings) {
        VkDescriptorSetLayoutBinding &dst = out.sets[b->set][b->binding];
        dst.binding         = b->binding;
        dst.descriptorType  = static_cast<VkDescriptorType>(b->descriptor_type);
        // count = product of array dims (1 if not an array).
        uint32_t count = 1;
        for (uint32_t d = 0; d < b->array.dims_count; ++d) count *= b->array.dims[d];
        if (count == 0) count = 1; // runtime arrays reflect as 0
        dst.descriptorCount = count;
        dst.stageFlags     |= stage;
        log << "    binding set=" << b->set << " binding=" << b->binding
            << " type=" << descriptorTypeName(dst.descriptorType)
            << " count=" << count
            << " name=\"" << (b->name ? b->name : "") << "\"\n";
    }

    // Push constant ranges.
    uint32_t pcCount = 0;
    spvReflectEnumeratePushConstantBlocks(&rmod, &pcCount, nullptr);
    std::vector<SpvReflectBlockVariable *> pcs(pcCount);
    if (pcCount > 0) {
        spvReflectEnumeratePushConstantBlocks(&rmod, &pcCount, pcs.data());
    }
    for (auto *pc : pcs) {
        VkPushConstantRange range{};
        range.stageFlags = stage;
        range.offset     = pc->offset;
        range.size       = pc->size;
        out.pushConstants.push_back(range);
        log << "    pushConstant offset=" << pc->offset << " size=" << pc->size
            << " name=\"" << (pc->name ? pc->name : "") << "\"\n";
    }

    // Fragment output locations -> number of color attachments.
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        uint32_t outVarCount = 0;
        spvReflectEnumerateOutputVariables(&rmod, &outVarCount, nullptr);
        std::vector<SpvReflectInterfaceVariable *> outVars(outVarCount);
        if (outVarCount > 0) {
            spvReflectEnumerateOutputVariables(&rmod, &outVarCount, outVars.data());
        }
        for (auto *v : outVars) {
            // Skip built-ins (gl_FragDepth etc.), which have a built-in decoration.
            if (v->built_in != -1) continue;
            out.hasFragmentOutputs = true;
            if (v->location > out.maxColorOutputLocation) {
                out.maxColorOutputLocation = v->location;
            }
            log << "    fragOutput location=" << v->location
                << " format=" << formatName(static_cast<VkFormat>(v->format))
                << " name=\"" << (v->name ? v->name : "") << "\"\n";
        }
    }

    // Vertex input attributes -> vertex input state. The vertex shader consuming
    // input locations with no matching attribute descriptions is a common cause
    // of VK_ERROR_INITIALIZATION_FAILED at pipeline creation on Mali.
    if (stage == VK_SHADER_STAGE_VERTEX_BIT) {
        uint32_t inVarCount = 0;
        spvReflectEnumerateInputVariables(&rmod, &inVarCount, nullptr);
        std::vector<SpvReflectInterfaceVariable *> inVars(inVarCount);
        if (inVarCount > 0) {
            spvReflectEnumerateInputVariables(&rmod, &inVarCount, inVars.data());
        }
        for (auto *v : inVars) {
            // Skip built-ins (gl_VertexIndex, gl_InstanceIndex, etc.).
            if (v->built_in != -1) continue;
            // SPV_REFLECT_FORMAT_* values match VkFormat enum values.
            VkVertexInputAttributeDescription attr{};
            attr.location = v->location;
            attr.binding  = 0;                                   // single interleaved binding
            attr.format   = static_cast<VkFormat>(v->format);
            attr.offset   = 0;                                   // not bound for real draws
            out.vertexAttributes.push_back(attr);
            log << "    vertexInput location=" << v->location
                << " format=" << formatName(attr.format)
                << " name=\"" << (v->name ? v->name : "") << "\"\n";
        }
    }

    log << "reflect " << stageName << ": bindings=" << bindingCount
        << " pushConstants=" << pcCount;
    if (stage == VK_SHADER_STAGE_VERTEX_BIT) {
        log << " vertexInputs=" << out.vertexAttributes.size();
    }
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        log << " fragOutputs=" << (out.hasFragmentOutputs
                                   ? (out.maxColorOutputLocation + 1) : 0);
    }
    log << "\n";

    spvReflectDestroyShaderModule(&rmod);
    return true;
}

} // namespace

extern "C" {

// Initializes a headless Vulkan instance + logical device (no swapchain/surface).
// Returns a human readable status string; check for "OK" prefix for success.
JNIEXPORT jstring JNICALL
Java_com_zm_androidtools_VulkanContext_nativeInit(JNIEnv *env, jobject /* this */) {
    std::ostringstream log;

    if (g_vk.initialized) {
        log << "OK (already initialized)\n";
        return env->NewStringUTF(log.str().c_str());
    }

    // Route VK debug-layer messages into this function's log so messages raised
    // during instance/device creation also reach the on-screen log. The guard
    // clears the sink on every return path.
    struct SinkGuard {
        explicit SinkGuard(std::ostringstream &s) { g_vkMessageSink = &s; }
        ~SinkGuard() { g_vkMessageSink = nullptr; }
    } sinkGuard(log);

    // 0. Query the loader's supported instance version. Passing an apiVersion
    // higher than what the loader/ICD supports is a very common cause of
    // VK_ERROR_INITIALIZATION_FAILED (-3) from vkCreateInstance.
    //
    // vkEnumerateInstanceVersion is a Vulkan 1.1 global command and is NOT
    // exported by libvulkan.so on older API levels (minSdk 24), so resolve it
    // dynamically. If it's unavailable the instance only supports Vulkan 1.0.
    uint32_t instanceVersion = VK_API_VERSION_1_0;
    auto pfnEnumInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
            vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"));
    if (pfnEnumInstanceVersion != nullptr) {
        if (pfnEnumInstanceVersion(&instanceVersion) != VK_SUCCESS) {
            instanceVersion = VK_API_VERSION_1_0;
        }
    }
    log << "loader instance version: "
        << VK_VERSION_MAJOR(instanceVersion) << "."
        << VK_VERSION_MINOR(instanceVersion) << "."
        << VK_VERSION_PATCH(instanceVersion) << "\n";

    // 0a. Enumerate available instance layers (so we can see if validation exists).
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    if (layerCount > 0) {
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    }
    bool hasValidationLayer = false;
    const char *kValidationLayer = "VK_LAYER_KHRONOS_validation";
    log << "instance layers (" << layerCount << "):\n";
    for (const auto &lp : availableLayers) {
        log << "  - " << lp.layerName << "\n";
        if (std::string(lp.layerName) == kValidationLayer) {
            hasValidationLayer = true;
        }
    }

    // 0b. Enumerate available instance extensions (look for debug_utils).
    // VK_EXT_debug_utils may be provided by the ICD or by the validation layer,
    // so check both the implicit (null) layer and the validation layer.
    bool hasDebugUtils = false;
    auto checkExtsFor = [&](const char *layerName) {
        uint32_t c = 0;
        vkEnumerateInstanceExtensionProperties(layerName, &c, nullptr);
        std::vector<VkExtensionProperties> exts(c);
        if (c > 0) {
            vkEnumerateInstanceExtensionProperties(layerName, &c, exts.data());
        }
        for (const auto &ep : exts) {
            if (layerName == nullptr) {
                log << "  - " << ep.extensionName << "\n";
            }
            if (std::string(ep.extensionName) == VK_EXT_DEBUG_UTILS_EXTENSION_NAME) {
                hasDebugUtils = true;
            }
        }
    };
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    log << "instance extensions (" << extCount << "):\n";
    checkExtsFor(nullptr);
    if (hasValidationLayer) {
        checkExtsFor(kValidationLayer);
    }
    log << "validation layer present: " << (hasValidationLayer ? "yes" : "no") << "\n";
    log << "debug_utils present: " << (hasDebugUtils ? "yes" : "no") << "\n";

    // 1. Create instance. Clamp apiVersion to what the loader actually supports.
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "AndroidTools";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "AndroidTools";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = instanceVersion; // never request more than supported

    std::vector<const char *> enabledLayers;
    std::vector<const char *> enabledExts;
    if (hasValidationLayer) enabledLayers.push_back(kValidationLayer);
    if (hasDebugUtils)      enabledExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VkInstanceCreateInfo instInfo{};
    instInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo        = &appInfo;
    instInfo.enabledLayerCount       = static_cast<uint32_t>(enabledLayers.size());
    instInfo.ppEnabledLayerNames     = enabledLayers.empty() ? nullptr : enabledLayers.data();
    instInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExts.size());
    instInfo.ppEnabledExtensionNames = enabledExts.empty() ? nullptr : enabledExts.data();

    // Chain a debug messenger into pNext so it captures errors raised *during*
    // vkCreateInstance / vkDestroyInstance themselves.
    VkDebugUtilsMessengerCreateInfoEXT dbgInfo{};
    if (hasDebugUtils) {
        fillDebugMessengerCreateInfo(dbgInfo);
        instInfo.pNext = &dbgInfo;
    }

    VkResult res = vkCreateInstance(&instInfo, nullptr, &g_vk.instance);
    if (res != VK_SUCCESS) {
        log << "FAIL vkCreateInstance: " << vkResultName(res) << " (" << res << ")\n";
        log << "hint: -3 here usually means apiVersion too high, a requested "
               "layer/extension is unavailable, or the ICD failed to load. "
               "Check the layer/extension lists above.\n";
        LOGE("%s", log.str().c_str());
        return env->NewStringUTF(log.str().c_str());
    }
    log << "vkCreateInstance OK\n";

    // 1a. Create the persistent debug messenger (separate from the pNext one).
    if (hasDebugUtils) {
        auto pfnCreate = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(g_vk.instance, "vkCreateDebugUtilsMessengerEXT"));
        if (pfnCreate != nullptr) {
            VkDebugUtilsMessengerCreateInfoEXT createInfo{};
            fillDebugMessengerCreateInfo(createInfo);
            VkResult dres = pfnCreate(g_vk.instance, &createInfo, nullptr, &g_vk.debugMessenger);
            log << "debug messenger: "
                << (dres == VK_SUCCESS ? "created" : "create failed") << "\n";
        } else {
            log << "debug messenger: vkCreateDebugUtilsMessengerEXT not found\n";
        }
    }

    // 2. Enumerate physical devices and pick the first.
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(g_vk.instance, &gpuCount, nullptr);
    if (gpuCount == 0) {
        log << "FAIL no Vulkan physical devices found\n";
        cleanupInstance();
        LOGE("%s", log.str().c_str());
        return env->NewStringUTF(log.str().c_str());
    }
    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(g_vk.instance, &gpuCount, gpus.data());
    g_vk.physicalDevice = gpus[0];

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(g_vk.physicalDevice, &props);
    log << "GPU: " << props.deviceName
        << " (apiVersion "
        << VK_VERSION_MAJOR(props.apiVersion) << "."
        << VK_VERSION_MINOR(props.apiVersion) << "."
        << VK_VERSION_PATCH(props.apiVersion) << ")\n";

    // 3. Find a queue family that supports graphics (also fine for compute).
    uint32_t qfCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g_vk.physicalDevice, &qfCount, nullptr);
    std::vector<VkQueueFamilyProperties> qfs(qfCount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_vk.physicalDevice, &qfCount, qfs.data());

    bool foundQueue = false;
    for (uint32_t i = 0; i < qfCount; ++i) {
        if (qfs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            g_vk.queueFamily = i;
            foundQueue = true;
            break;
        }
    }
    if (!foundQueue) {
        log << "FAIL no graphics-capable queue family\n";
        cleanupInstance();
        LOGE("%s", log.str().c_str());
        return env->NewStringUTF(log.str().c_str());
    }
    log << "queue family index = " << g_vk.queueFamily << "\n";

    // 4. Create the logical device + retrieve the queue.
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = g_vk.queueFamily;
    queueInfo.queueCount       = 1;
    queueInfo.pQueuePriorities = &priority;

    // Enable all features the GPU supports. UE shaders rely on capabilities such
    // as fragmentStoresAndAtomics / shader*ArrayDynamicIndexing; if those aren't
    // enabled, the driver rejects the pipeline with VK_ERROR_INITIALIZATION_FAILED.
    VkPhysicalDeviceFeatures supportedFeatures{};
    vkGetPhysicalDeviceFeatures(g_vk.physicalDevice, &supportedFeatures);
    log << "device features: fragmentStoresAndAtomics="
        << supportedFeatures.fragmentStoresAndAtomics
        << " vertexPipelineStoresAndAtomics="
        << supportedFeatures.vertexPipelineStoresAndAtomics << "\n";

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos    = &queueInfo;
    deviceInfo.pEnabledFeatures     = &supportedFeatures;

    res = vkCreateDevice(g_vk.physicalDevice, &deviceInfo, nullptr, &g_vk.device);
    if (res != VK_SUCCESS) {
        log << "FAIL vkCreateDevice: " << vkResultName(res) << " (" << res << ")\n";
        log << "hint: -3 here usually means a requested device extension/feature "
               "or queue config is unsupported by this GPU.\n";
        cleanupInstance();
        LOGE("%s", log.str().c_str());
        return env->NewStringUTF(log.str().c_str());
    }
    vkGetDeviceQueue(g_vk.device, g_vk.queueFamily, 0, &g_vk.queue);
    log << "vkCreateDevice OK\n";

    g_vk.initialized = true;
    log << "OK init complete\n";
    LOGI("%s", log.str().c_str());
    return env->NewStringUTF(log.str().c_str());
}

// Creates a VkShaderModule from raw SPIR-V bytes. This parses and validates the
// SPIR-V; the driver compiles it into GPU machine code later at pipeline-creation
// time. The module is destroyed before returning since no pipeline is built yet.
// Returns a status string; "OK" prefix indicates success.
JNIEXPORT jstring JNICALL
Java_com_zm_androidtools_VulkanContext_nativeCreateShaderModule(
        JNIEnv *env, jobject /* this */, jbyteArray spirvBytes) {
    std::ostringstream log;

    if (!g_vk.initialized || g_vk.device == VK_NULL_HANDLE) {
        log << "FAIL Vulkan not initialized; call init() first\n";
        return env->NewStringUTF(log.str().c_str());
    }

    VkShaderModule module = VK_NULL_HANDLE;
    {
        std::vector<uint32_t> code;
        if (readSpirvWords(env, spirvBytes, "shader", code, log)) {
            module = createShaderModuleFromWords(code, "shader", log);
        }
    }
    if (module == VK_NULL_HANDLE) {
        LOGE("%s", log.str().c_str());
        return env->NewStringUTF(log.str().c_str());
    }

    // No pipeline yet, so release the module immediately.
    vkDestroyShaderModule(g_vk.device, module, nullptr);
    log << "shader module destroyed (no pipeline created yet)\n";

    LOGI("%s", log.str().c_str());
    return env->NewStringUTF(log.str().c_str());
}

// Links a vertex + fragment SPIR-V pair into a graphics pipeline. This is where
// the driver actually compiles both SPIR-V modules into GPU machine code
// (vkCreateGraphicsPipelines). A minimal render pass / pipeline layout / fixed-
// function state is built so the pipeline can be created off-screen.
// Returns a status string; lines starting with "OK" indicate success.
JNIEXPORT jstring JNICALL
Java_com_zm_androidtools_VulkanContext_nativeCreateGraphicsPipeline(
        JNIEnv *env, jobject /* this */,
        jbyteArray vertSpirv, jbyteArray fragSpirv) {
    std::ostringstream log;

    if (!g_vk.initialized || g_vk.device == VK_NULL_HANDLE) {
        log << "FAIL Vulkan not initialized; call init() first\n";
        return env->NewStringUTF(log.str().c_str());
    }

    // Route VK debug-layer messages into this function's log so they also appear
    // in the string returned to Java (and thus the on-screen log). The guard
    // clears the sink on every return path.
    struct SinkGuard {
        explicit SinkGuard(std::ostringstream &s) { g_vkMessageSink = &s; }
        ~SinkGuard() { g_vkMessageSink = nullptr; }
    } sinkGuard(log);

    // 1. Read SPIR-V words for both stages (kept for reflection too).
    std::vector<uint32_t> vertCode, fragCode;
    if (!readSpirvWords(env, vertSpirv, "vertex", vertCode, log)) {
        LOGE("%s", log.str().c_str());
        return env->NewStringUTF(log.str().c_str());
    }
    if (!readSpirvWords(env, fragSpirv, "fragment", fragCode, log)) {
        LOGE("%s", log.str().c_str());
        return env->NewStringUTF(log.str().c_str());
    }

    // 2. Create both shader modules.
    VkShaderModule vertModule = createShaderModuleFromWords(vertCode, "vertex", log);
    if (vertModule == VK_NULL_HANDLE) {
        LOGE("%s", log.str().c_str());
        return env->NewStringUTF(log.str().c_str());
    }
    VkShaderModule fragModule = createShaderModuleFromWords(fragCode, "fragment", log);
    if (fragModule == VK_NULL_HANDLE) {
        vkDestroyShaderModule(g_vk.device, vertModule, nullptr);
        LOGE("%s", log.str().c_str());
        return env->NewStringUTF(log.str().c_str());
    }

    VkRenderPass                       renderPass     = VK_NULL_HANDLE;
    VkPipelineLayout                   pipelineLayout = VK_NULL_HANDLE;
    VkPipeline                         pipeline       = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> setLayouts;
    bool                               ok             = false;

    do {
        // 3. Reflect both stages to learn the descriptor/push-constant interface
        // and the number of fragment color outputs. Mismatched layout/render pass
        // is the usual cause of VK_ERROR_INITIALIZATION_FAILED at pipeline creation.
        ReflectedInterface iface;
        if (!reflectStage(vertCode, VK_SHADER_STAGE_VERTEX_BIT, "vertex", iface, log)) break;
        if (!reflectStage(fragCode, VK_SHADER_STAGE_FRAGMENT_BIT, "fragment", iface, log)) break;

        // 4. Build one VkDescriptorSetLayout per reflected set. Descriptor sets
        // must be contiguous from 0; fill gaps with empty layouts.
        uint32_t maxSet = 0;
        bool hasSets = !iface.sets.empty();
        for (const auto &kv : iface.sets) maxSet = std::max(maxSet, kv.first);
        uint32_t setCount = hasSets ? (maxSet + 1) : 0;

        VkResult res = VK_SUCCESS;
        for (uint32_t s = 0; s < setCount; ++s) {
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            auto it = iface.sets.find(s);
            if (it != iface.sets.end()) {
                for (const auto &bkv : it->second) bindings.push_back(bkv.second);
            }
            VkDescriptorSetLayoutCreateInfo dslInfo{};
            dslInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            dslInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            dslInfo.pBindings    = bindings.empty() ? nullptr : bindings.data();

            VkDescriptorSetLayout dsl = VK_NULL_HANDLE;
            res = vkCreateDescriptorSetLayout(g_vk.device, &dslInfo, nullptr, &dsl);
            if (res != VK_SUCCESS) {
                log << "FAIL vkCreateDescriptorSetLayout set=" << s << ": "
                    << vkResultName(res) << " (" << res << ")\n";
                break;
            }
            setLayouts.push_back(dsl);
        }
        if (res != VK_SUCCESS) break;
        log << "descriptor set layouts: " << setLayouts.size() << "\n";

        // 5. Pipeline layout from reflected sets + push constants.
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount         = static_cast<uint32_t>(setLayouts.size());
        layoutInfo.pSetLayouts            = setLayouts.empty() ? nullptr : setLayouts.data();
        layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(iface.pushConstants.size());
        layoutInfo.pPushConstantRanges    = iface.pushConstants.empty()
                                            ? nullptr : iface.pushConstants.data();
        res = vkCreatePipelineLayout(g_vk.device, &layoutInfo, nullptr, &pipelineLayout);
        if (res != VK_SUCCESS) {
            log << "FAIL vkCreatePipelineLayout: " << vkResultName(res) << " (" << res << ")\n";
            break;
        }
        log << "vkCreatePipelineLayout OK (pushConstants="
            << iface.pushConstants.size() << ")\n";

        // 6. Render pass with one color attachment per reflected fragment output.
        uint32_t colorCount = iface.hasFragmentOutputs
                              ? (iface.maxColorOutputLocation + 1) : 1;
        std::vector<VkAttachmentDescription> attachments(colorCount);
        std::vector<VkAttachmentReference>   colorRefs(colorCount);
        for (uint32_t i = 0; i < colorCount; ++i) {
            attachments[i] = {};
            attachments[i].format         = VK_FORMAT_R8G8B8A8_UNORM;
            attachments[i].samples        = VK_SAMPLE_COUNT_1_BIT;
            attachments[i].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[i].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[i].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[i].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[i].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorRefs[i].attachment = i;
            colorRefs[i].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        log << "color attachments: " << colorCount << "\n";

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = colorCount;
        subpass.pColorAttachments    = colorRefs.data();

        VkRenderPassCreateInfo rpInfo{};
        rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.attachmentCount = colorCount;
        rpInfo.pAttachments    = attachments.data();
        rpInfo.subpassCount    = 1;
        rpInfo.pSubpasses      = &subpass;

        res = vkCreateRenderPass(g_vk.device, &rpInfo, nullptr, &renderPass);
        if (res != VK_SUCCESS) {
            log << "FAIL vkCreateRenderPass: " << vkResultName(res) << " (" << res << ")\n";
            break;
        }
        log << "vkCreateRenderPass OK\n";

        // 7. Shader stages (the "link" inputs).
        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vertModule;
        stages[0].pName  = iface.vertexEntryPoint.empty()
                           ? "main" : iface.vertexEntryPoint.c_str();
        stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = fragModule;
        stages[1].pName  = iface.fragmentEntryPoint.empty()
                           ? "main" : iface.fragmentEntryPoint.c_str();

        // 8. Fixed-function state (no vertex buffers; triangle list).
        // 8. Vertex input state built from the vertex shader's reflected inputs.
        // Pack all attributes into a single interleaved binding; offsets are laid
        // out sequentially by format size. This is enough for pipeline creation
        // (we don't issue real draws), and makes the vertex input interface match
        // what the shader consumes.
        std::vector<VkVertexInputAttributeDescription> vtxAttrs = iface.vertexAttributes;
        uint32_t vtxStride = 0;
        for (auto &a : vtxAttrs) {
            a.offset = vtxStride;
            vtxStride += formatSizeBytes(a.format);
        }
        VkVertexInputBindingDescription vtxBinding{};
        vtxBinding.binding   = 0;
        vtxBinding.stride    = vtxStride;
        vtxBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        if (!vtxAttrs.empty()) {
            vertexInput.vertexBindingDescriptionCount   = 1;
            vertexInput.pVertexBindingDescriptions      = &vtxBinding;
            vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(vtxAttrs.size());
            vertexInput.pVertexAttributeDescriptions    = vtxAttrs.data();
        }

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // Use dynamic viewport/scissor so we don't need real framebuffer dimensions.
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount  = 1;

        VkDynamicState dynStates[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates    = dynStates;

        VkPipelineRasterizationStateCreateInfo raster{};
        raster.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode    = VK_CULL_MODE_NONE;
        raster.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        raster.lineWidth   = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // One blend attachment per color output.
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(colorCount);
        for (uint32_t i = 0; i < colorCount; ++i) {
            blendAttachments[i] = {};
            blendAttachments[i].colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            blendAttachments[i].blendEnable = VK_FALSE;
        }

        VkPipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.attachmentCount = colorCount;
        colorBlend.pAttachments    = blendAttachments.data();

        // 9. Create the graphics pipeline -> driver compiles SPIR-V to GPU ISA here.
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = stages;
        pipelineInfo.pVertexInputState   = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &raster;
        pipelineInfo.pMultisampleState   = &multisample;
        pipelineInfo.pColorBlendState    = &colorBlend;
        pipelineInfo.pDynamicState       = &dynamicState;
        pipelineInfo.layout              = pipelineLayout;
        pipelineInfo.renderPass          = renderPass;
        pipelineInfo.subpass             = 0;

        res = vkCreateGraphicsPipelines(g_vk.device, VK_NULL_HANDLE, 1,
                                        &pipelineInfo, nullptr, &pipeline);
        if (res != VK_SUCCESS) {
            log << "FAIL vkCreateGraphicsPipelines: " << vkResultName(res)
                << " (" << res << ")\n";
            break;
        }
        log << "OK vkCreateGraphicsPipelines succeeded "
            << "(SPIR-V compiled to GPU machine code, pipeline linked)\n";
        ok = true;
    } while (false);

    // 10. Cleanup. Shader modules can be freed right after pipeline creation.
    if (pipeline != VK_NULL_HANDLE)        vkDestroyPipeline(g_vk.device, pipeline, nullptr);
    if (pipelineLayout != VK_NULL_HANDLE)  vkDestroyPipelineLayout(g_vk.device, pipelineLayout, nullptr);
    if (renderPass != VK_NULL_HANDLE)      vkDestroyRenderPass(g_vk.device, renderPass, nullptr);
    for (VkDescriptorSetLayout dsl : setLayouts) {
        vkDestroyDescriptorSetLayout(g_vk.device, dsl, nullptr);
    }
    vkDestroyShaderModule(g_vk.device, fragModule, nullptr);
    vkDestroyShaderModule(g_vk.device, vertModule, nullptr);

    if (ok) {
        log << "pipeline + modules destroyed (cleanup done)\n";
        LOGI("%s", log.str().c_str());
    } else {
        LOGE("%s", log.str().c_str());
    }
    return env->NewStringUTF(log.str().c_str());
}

// Tears down the logical device and instance.
JNIEXPORT void JNICALL
Java_com_zm_androidtools_VulkanContext_nativeDestroy(JNIEnv * /* env */, jobject /* this */) {
    if (g_vk.device != VK_NULL_HANDLE) {
        vkDestroyDevice(g_vk.device, nullptr);
        g_vk.device = VK_NULL_HANDLE;
    }
    cleanupInstance();
    g_vk.physicalDevice = VK_NULL_HANDLE;
    g_vk.queue          = VK_NULL_HANDLE;
    g_vk.initialized    = false;
    LOGI("Vulkan destroyed");
}

// Returns whether Vulkan init succeeded (instance + device created). This is the
// authoritative success signal; callers should not parse the log string.
JNIEXPORT jboolean JNICALL
Java_com_zm_androidtools_VulkanContext_nativeIsInitialized(JNIEnv * /* env */, jobject /* this */) {
    return (g_vk.initialized && g_vk.device != VK_NULL_HANDLE) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jstring JNICALL
Java_com_zm_androidtools_MainActivity_stringFromJNI(JNIEnv *env, jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

} // extern "C"
