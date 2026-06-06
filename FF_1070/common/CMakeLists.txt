# include_directories(
#     .
#     ${PROJECT_SOURCE_DIR}/../include/anyka37d
#     ${PROJECT_SOURCE_DIR}/../share/include
#     ${PROJECT_SOURCE_DIR}/../share/include/libxls
# )
    
# file(GLOB_RECURSE SOURCES ./*.c )
# add_library(common STATIC ${SOURCES})


include_directories(
    .
    ${PROJECT_SOURCE_DIR}/../include/anyka37d
    ${PROJECT_SOURCE_DIR}/../share/include
    ${PROJECT_SOURCE_DIR}/../share/include/libxls
)

file(GLOB_RECURSE SOURCES *.c)

# 排除旧 T300 / MIPS 平台文件
foreach(src ${SOURCES})
    if(src MATCHES ".*irq\\.c$" OR src MATCHES ".*t300_irq\\.c$")
        list(REMOVE_ITEM SOURCES ${src})
    endif()
endforeach()

add_library(common STATIC ${SOURCES})