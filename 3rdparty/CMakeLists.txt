# see: https://json.nlohmann.me/integration/cmake/#json_implicitconversions
set(JSON_ImplicitConversions OFF)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
set(JSON_SystemInclude ON)
add_subdirectory(nlohmann_json)
