add_library(IAGHI STATIC
        cpp/iaghi.cpp
        cpp/backends/vulkan/backend.cpp
        cpp/backends/vulkan/buffer.cpp
        cpp/backends/vulkan/device.cpp
        cpp/backends/vulkan/pipeline.cpp
        cpp/backends/vulkan/image.cpp
        cpp/backends/vulkan/swapchain.cpp
)

target_include_directories(IAGHI PUBLIC
        $<BUILD_INTERFACE:${IAGHI_ROOT}/include>
        $<INSTALL_INTERFACE:include>
)
target_include_directories(IAGHI PRIVATE hpp)

target_link_libraries(IAGHI PUBLIC
        libauxid
)

target_link_libraries(IAGHI PRIVATE
        volk::volk
        GPUOpen::VulkanMemoryAllocator
)

if (IAGHI_BUILD_UTILS)
    target_sources(IAGHI PRIVATE
            cpp/utils/utils.cpp
    )
    target_link_libraries(IAGHI PUBLIC
            SPIRV
            glslang
            STB_CMake
            glslang-default-resource-limits
    )
endif ()
