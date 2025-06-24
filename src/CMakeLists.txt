
file(GLOB_RECURSE ALL_FILES CONFIGURE_DEPENDS
    "*.cpp" "*.c"
    "*.h" "*.hpp"
)

set(FILTERED_FILES "")
foreach(FILE_PATH ${ALL_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${FILE_PATH}")
    string(REPLACE "\\" "/" REL_PATH "${REL_PATH}") # normalize for Windows

    if(REL_PATH MATCHES "^graphics/SWTri/SWTri\\.(cpp|h)$")
        list(APPEND FILTERED_FILES "${FILE_PATH}")
    elseif(REL_PATH MATCHES "^graphics/SWTri/")
        continue()
    else()
        list(APPEND FILTERED_FILES "${FILE_PATH}")
    endif()
endforeach()

set(ALL_FILES ${FILTERED_FILES})

set(SOURCE_FILES "")
set(HEADER_FILES "")

foreach(FILE_PATH ${ALL_FILES})
    get_filename_component(EXT "${FILE_PATH}" EXT)
    string(TOLOWER "${EXT}" EXT)

    if(EXT STREQUAL ".cpp" OR EXT STREQUAL ".c" OR EXT STREQUAL ".cxx")
        list(APPEND SOURCE_FILES "${FILE_PATH}")
    elseif(EXT STREQUAL ".h" OR EXT STREQUAL ".hpp")
        list(APPEND HEADER_FILES "${FILE_PATH}")
    endif()
endforeach()

function(capitalize_path INPUT OUTPUT)
    set(RESULT "")
    string(REPLACE "\\" "/" INPUT_PATH "${INPUT}")
    string(REPLACE "/" ";" PATH_PARTS "${INPUT_PATH}")
    foreach(PART ${PATH_PARTS})
        # handle exceptions
        string(TOLOWER "${PART}" LOWER_PART)
        if(LOWER_PART STREQUAL "imagelib")
            set(PART "ImageLib")
        elseif(LOWER_PART STREQUAL "paklib")
            set(PART "PakLib")
        elseif(LOWER_PART STREQUAL "imgui")
            set(PART "Dear ImGui")
        elseif(LOWER_PART STREQUAL "readwrite")
            set(PART "ReadWrite")
        else()
            string(SUBSTRING "${PART}" 0 1 FIRST_CHAR)
            string(SUBSTRING "${PART}" 1 -1 REST_CHARS)
            string(TOUPPER "${FIRST_CHAR}" FIRST_CHAR)
            set(PART "${FIRST_CHAR}${REST_CHARS}")
        endif()
        string(APPEND RESULT "${PART}/")
    endforeach()
    string(REGEX REPLACE "/$" "" RESULT "${RESULT}")
    set(${OUTPUT} "${RESULT}" PARENT_SCOPE)
endfunction()

# group source files
foreach(FILE_PATH ${SOURCE_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${FILE_PATH}")
    get_filename_component(DIR_PATH "${REL_PATH}" PATH)
    capitalize_path("${DIR_PATH}" CAPITALIZED_GROUP)
    source_group("Source Files/${CAPITALIZED_GROUP}" FILES "${FILE_PATH}")
endforeach()

# group header files
foreach(FILE_PATH ${HEADER_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${FILE_PATH}")
    get_filename_component(DIR_PATH "${REL_PATH}" PATH)
    capitalize_path("${DIR_PATH}" CAPITALIZED_GROUP)
    source_group("Header Files/${CAPITALIZED_GROUP}" FILES "${FILE_PATH}")
endforeach()

set(EXTERNAL_DEPENDENCIES
    ../external/aes.c
    ../external/aes.h
)

set(ALL_GROUPED_FILES ${SOURCE_FILES} ${HEADER_FILES} ${EXTERNAL_DEPENDENCIES})

add_executable(PopTools ${ALL_GROUPED_FILES})

target_link_libraries(PopTools PUBLIC
    SDL3::SDL3-static
    tinyxml2
    zlibstatic
)
