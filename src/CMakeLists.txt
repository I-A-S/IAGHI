add_library(IAGHI STATIC
        cpp/IAGHI.cpp
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
