# Copyright (c) 2026 Infineon Technologies AG,
# or an affiliate of Infineon Technologies AG.
# SPDX-License-Identifier: Apache-2.0

# Create the audio front-end library
zephyr_library()

# Add all required AFE source files - BASIC MODE ONLY
set(AFE_SOURCES
    cy_afe.c                        # Main AFE API
    cy_afe_audio_task.c             # Audio processing task
    cy_afe_audio_process.c          # Audio processing functions
    cy_afe_audio_utils.c            # Audio utilities
    cy_afe_audio_bd_calc.c          # Audio buffer calculations
    cy_afe_profiler.c              # Performance profiling
    cy_afe_speech_enh_stubs.c      # Speech enhancement stubs for basic mode
)

# Force disable speech enhancement to avoid AVC integration complexity
# Speech enhancement requires full AVC integration which is not yet available
message(STATUS "AFE: Basic mode - Using speech enhancement STUBS")
# if(CONFIG_AUDIOFE_SPEECH_ENH) 
#     list(APPEND AFE_SOURCES cy_afe_audio_speech_enh.c)
#     message(STATUS "AFE: Speech enhancement enabled")
# else()
#     message(STATUS "AFE: Speech enhancement disabled - missing AVC integration") 
# endif()

# Conditionally add debug features
if(CONFIG_AUDIOFE_LOGGING)
    list(APPEND AFE_SOURCES cy_afe_audio_debug.c)
endif()

# Conditionally add tuning features
if(CONFIG_AUDIOFE_TUNING)
    list(APPEND AFE_SOURCES 
        cy_afe_tuner_task.c
        cy_afe_tuner_process.c
        cy_afe_tuner_commands.c
    )
    message(STATUS "AFE: Tuning feature sources added")
else()
    # Add Zephyr-specific stubs when tuning is disabled
    list(APPEND AFE_SOURCES ../zephyr/cy_afe_basic_stubs.c)
    message(STATUS "AFE: Tuning disabled - using Zephyr stubs")
endif()

# Check if source files exist
foreach(source_file ${AFE_SOURCES})
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${source_file})
        message(FATAL_ERROR "Audio Front-End source file not found: ${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
    endif()
endforeach()

# Add sources to the library
zephyr_library_sources(${AFE_SOURCES})

# Add Zephyr-specific stub implementations with correct include paths
zephyr_library_sources(../zephyr/cy_afe_basic_stubs.c)

# Note: Include directories are set at module level in zephyr.cmake
# Do not set additional include directories here per Zephyr best practices

# Define preprocessor macros based on configuration
if(CONFIG_AUDIOFE_TUNING)
    zephyr_library_compile_definitions(CY_AFE_ENABLE_TUNING_FEATURE)
endif()

if(CONFIG_AUDIOFE_DEBUG)
    zephyr_library_compile_definitions(CY_AFE_ENABLE_DEBUG)
endif()

# Add CONFIG_USE_INFINEON_ABSTRACTION_RTOS as preprocessor definition
zephyr_library_compile_definitions(CONFIG_USE_INFINEON_ABSTRACTION_RTOS)