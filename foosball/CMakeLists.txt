cmake_minimum_required(VERSION 2.8)

project(soletta-demos)

add_definitions(-DSOL_FLOW_NODE_TYPE_MODULE_EXTERNAL)

set(NODE_TYPES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/node_types)
set(SCORE_COUNTER_RESOURCE ${NODE_TYPES_DIR}/oic.r.score.json)
set(MODULES_DIR ${CMAKE_BINARY_DIR}/module_descriptions)
set(STAGE_DIR ${CMAKE_BINARY_DIR}/stage)
set(SCORE_COUNTER_GEN_H ${STAGE_DIR}/score-counter-gen.h)
set(SCORE_COUNTER_GEN_C ${STAGE_DIR}/score-counter-gen.c)
set(SCORE_COUNTER_JSON ${STAGE_DIR}/score-counter.json)
set(SCORE_COUNTER_C ${STAGE_DIR}/score-counter.c)

execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=modulesdir soletta
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE SOLETTA_LIB_PREFIX
  )

execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=nodeschemapath soletta
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE SCHEMA_PATH
  )

execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=pkgdatadir soletta
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE SOLETTA_DATA_DIR
  )

add_custom_command(OUTPUT ${SCORE_COUNTER_C}
#FIXME use add_custom_command otherwise it always will build this gen files
#add_custom_command(
# OUTPUT ${SCORE_COUNTER_GEN_H} ${SCORE_COUNTER_GEN_C}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${STAGE_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MODULES_DIR}
  COMMAND ${CMAKE_FIND_ROOT_PATH}/${SOLETTA_PREFIX}/bin/sol-oic-gen.py --node-type-json=${SCORE_COUNTER_JSON}  --node-type-impl=${SCORE_COUNTER_C} --node-type-gen-c=${SCORE_COUNTER_GEN_C} --node-type-gen-h=${SCORE_COUNTER_GEN_H} ${CMAKE_CURRENT_SOURCE_DIR}/node_types
  COMMAND ${CMAKE_FIND_ROOT_PATH}/${SOLETTA_PREFIX}/bin/sol-flow-node-type-validate.py ${CMAKE_FIND_ROOT_PATH}/${SCHEMA_PATH} ${SCORE_COUNTER_JSON}
  COMMAND ${CMAKE_FIND_ROOT_PATH}/${SOLETTA_PREFIX}/bin/sol-flow-node-type-gen.py --prefix=sol-flow-node-type --genspec-schema=${CMAKE_FIND_ROOT_PATH}/${SCHEMA_PATH} ${SCORE_COUNTER_JSON} ${SCORE_COUNTER_GEN_H} ${SCORE_COUNTER_GEN_C} ${MODULES_DIR}/score_counter.json
  DEPENDS ${SCORE_COUNTER_RESOURCE}
  )

set(BUILT_SOURCES
  ${SCORE_COUNTER_GEN_H}
  ${SCORE_COUNTER_GEN_C}
  ${SCORE_COUNTER_C}
  )
SET_SOURCE_FILES_PROPERTIES(${BUILT_SOURCES} PROPERTIES
  GENERATED true
  )

include_directories(
  ${SOLETTA_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/node_types/
  )

link_directories(
  ${SOLETTA_LIBRARY_DIRS}
  )

add_library(score-counter SHARED
  ${SCORE_COUNTER_C}
  )
set_target_properties(score-counter
  PROPERTIES PREFIX ""
  )

target_link_libraries(score-counter
  ${SOLETTA_LIBRARIES}
  m)

install(TARGETS score-counter
  LIBRARY DESTINATION ${SOLETTA_LIB_PREFIX}/flow/
  )

install(FILES ${CMAKE_BINARY_DIR}/module_descriptions/score_counter.json
    DESTINATION ${SOLETTA_DATA_DIR}/flow/descriptions
)
