add_library(dht dht.c)
target_include_directories(dht PUBLIC Inc)
target_link_libraries(dht PUBLIC stm32cubemx dwt)
