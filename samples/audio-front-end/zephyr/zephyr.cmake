if(CONFIG_AUDIOFE)

  message(STATUS "AFE: basic mode integration enabled")

  # AFE module headers.
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/source/include)

  # Zephyr compatibility layer headers.
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/include)

  # Core compile definitions for middleware compatibility.
  zephyr_compile_definitions(
    ENABLE_AFE_MW_SUPPORT=1
    ENABLE_AUDIO_FRONT_END_LOGS=3
    CONFIG_USE_INFINEON_ABSTRACTION_RTOS=1
  )

  if(CONFIG_AUDIOFE_TUNING)
    zephyr_compile_definitions(CY_AFE_ENABLE_TUNING_FEATURE=1)
  else()
    zephyr_compile_definitions(CY_AFE_ENABLE_TUNING_FEATURE=0)
  endif()

  add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/source source)

endif()
