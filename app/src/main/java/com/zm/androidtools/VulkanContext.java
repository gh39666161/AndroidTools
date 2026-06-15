package com.zm.androidtools;

/**
 * Thin JNI bridge to the native Vulkan code in native-lib.cpp.
 *
 * Flow:
 *   1. init()                  - create VkInstance + VkDevice (headless, no surface)
 *   2. createShaderModule(spv) - load SPIR-V bytes into a VkShaderModule
 *   3. destroy()               - tear down device + instance
 *
 * Note: vkCreateShaderModule only parses/validates the SPIR-V. The driver
 * compiles SPIR-V into GPU machine code later, at pipeline-creation time
 * (vkCreateGraphicsPipelines / vkCreateComputePipelines).
 */
public class VulkanContext {

    static {
        System.loadLibrary("androidtools");
    }

    private boolean initialized = false;

    /** Creates a headless Vulkan instance + logical device. */
    public String init() {
        String result = nativeInit();
        // Success is determined by the native state, not by parsing the log text.
        initialized = nativeIsInitialized();
        return result;
    }

    public boolean isInitialized() {
        return initialized;
    }

    /**
     * Creates (and immediately releases) a VkShaderModule from raw SPIR-V bytes.
     *
     * @param spirv raw .spv contents
     * @return status log; lines starting with "OK" indicate success
     */
    public String createShaderModule(byte[] spirv) {
        return nativeCreateShaderModule(spirv);
    }

    /**
     * Links a vertex + fragment SPIR-V pair into a graphics pipeline. This is the
     * step where the driver compiles both SPIR-V modules into GPU machine code
     * (vkCreateGraphicsPipelines).
     *
     * @param vertSpirv raw vertex shader .spv contents
     * @param fragSpirv raw fragment shader .spv contents
     * @return status log; lines starting with "OK" indicate success
     */
    public String createGraphicsPipeline(byte[] vertSpirv, byte[] fragSpirv) {
        return nativeCreateGraphicsPipeline(vertSpirv, fragSpirv);
    }

    /** Tears down the Vulkan device and instance. */
    public void destroy() {
        nativeDestroy();
        initialized = false;
    }

    private native String nativeInit();
    private native boolean nativeIsInitialized();
    private native String nativeCreateShaderModule(byte[] spirv);
    private native String nativeCreateGraphicsPipeline(byte[] vertSpirv, byte[] fragSpirv);
    private native void nativeDestroy();
}
