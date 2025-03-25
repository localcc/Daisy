add_executable(Example_Output "main.cpp")
target_compile_features(Example_Output PRIVATE cxx_std_17)
target_link_libraries(Example_Output PRIVATE Daisy)