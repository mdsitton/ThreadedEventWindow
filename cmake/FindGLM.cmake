# Variables defined:
#  GLM_FOUND
#  GLM_INCLUDE_DIR
# 
# Environment variables used:
#  GLM_ROOT

find_path(GLM_INCLUDE_DIR glm/glm.hpp)
find_path(GLM_INCLUDE_DIR glm/glm.hpp
  HINTS $ENV{GLM_ROOT})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM DEFAULT_MSG GLM_INCLUDE_DIR)

mark_as_advanced(GLM_INCLUDE_DIR)
