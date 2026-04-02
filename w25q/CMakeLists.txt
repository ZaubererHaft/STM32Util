add_library(w25qxx W25Qxx.c)
target_include_directories(w25qxx PUBLIC Inc)
target_link_libraries(w25qxx PUBLIC stm32cubemx)
