set(SRC_FILES
    "cpp/main.cpp"
)

add_executable(render-to-texture ${SRC_FILES})

target_link_libraries(render-to-texture PRIVATE
        IAGHI
        glm::glm
        SDL3::SDL3
)
