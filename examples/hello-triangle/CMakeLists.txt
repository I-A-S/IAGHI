set(SRC_FILES
    "cpp/main.cpp"
)

add_executable(hello-triangle ${SRC_FILES})

target_link_libraries(hello-triangle PRIVATE
        IAGHI
        glm::glm
        SDL3::SDL3
)
