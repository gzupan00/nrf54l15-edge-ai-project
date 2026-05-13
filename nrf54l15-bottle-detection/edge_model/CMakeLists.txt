set(EI_SDK_PATH "C:/ncs/v3.2.4/modules/edge-impulse-sdk-zephyr")

include("${EI_SDK_PATH}/edge-impulse-sdk/cmake/utils.cmake")

RECURSIVE_FIND_FILE_APPEND(MODEL_SOURCE "tflite-model" "*.cpp")

zephyr_include_directories(
    .
    ${EI_SDK_PATH}
    ${EI_SDK_PATH}/edge-impulse-sdk
)

zephyr_library_sources(${MODEL_SOURCE})

