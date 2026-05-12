set(SRC_FILES
    "cpp/main.cpp"
)

add_executable(rotating-cube ${SRC_FILES})

target_link_libraries(rotating-cube PRIVATE
        IAGHI
        glm::glm
        SDL3::SDL3
)
