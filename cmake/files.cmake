target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/syscall.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/sysmem.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/Queue.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/Task.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/Scheduler.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/Portable.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/Rtos.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/Mutex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Startup/startup_stm32f103xx.S"
)