
FetchContent_Declare(
  ImGui
  GIT_REPOSITORY    https://github.com/ocornut/imgui.git
  GIT_TAG           master # master
  GIT_SHALLOW       TRUE
)

FetchContent_MakeAvailable(ImGui)

if (NOT TARGET ImGui)
  add_library(ImGui   INTERFACE )
endif()
