
FetchContent_Declare(
  ImGui
  GIT_REPOSITORY    https://github.com/ocornut/imgui.git
  GIT_TAG           master # master
  GIT_SHALLOW       TRUE
)

FetchContent_MakeAvailable(ImGui)

add_library( ImGui   INTERFACE  )
include_directories( ${imgui_SOURCE_DIR} )
