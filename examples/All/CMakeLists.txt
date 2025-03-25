add_executable(Example_All "main.cpp")
target_compile_features(Example_All PRIVATE cxx_std_17)
target_link_libraries(Example_All PRIVATE Daisy)