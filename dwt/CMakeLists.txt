add_library(dwt dwt.c)
target_include_directories(dwt PUBLIC Inc)
target_link_libraries(dwt PUBLIC stm32cubemx)
