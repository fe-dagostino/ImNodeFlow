
FetchContent_Declare(
  ImGui
  GIT_REPOSITORY    https://github.com/ocornut/imgui.git
  GIT_TAG           master # master
  GIT_SHALLOW       TRUE
)

FetchContent_MakeAvailable(ImGui)

if (NOT TARGET ImGui)
  add_library(ImGui   INTERFACE )

  include_directories( ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends )

  if(hasParent)
    message( INFO " EXPORT ImGui SRC DIR: [${imgui_SOURCE_DIR}]" )
    message( INFO " EXPORT ImGui BIN DIR: [${imgui_BINARY_DIR}]" )
    # make both ${imgui_SOURCE_DIR} and ${imgui_BINARY_DIR} visible at parent scope.
    set( imgui_SOURCE_DIR ${imgui_SOURCE_DIR} PARENT_SCOPE )
    set( imgui_BINARY_DIR ${imgui_BINARY_DIR} PARENT_SCOPE )
  endif(hasParent)  
endif()
