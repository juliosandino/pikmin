# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Adding library to help with sensors
set(EXTRA_COMPONENT_DIRS "$ENV{HOME}/esp/playground/esp-idf-lib/components" "$ENV{HOME}/esp/playground/esp-idf-ssd1306/components")


include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(pikmin)
