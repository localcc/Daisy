add_executable(Example_Input "main.cpp")
target_compile_features(Example_Input PRIVATE cxx_std_17)
target_link_libraries(Example_Input PRIVATE Daisy)