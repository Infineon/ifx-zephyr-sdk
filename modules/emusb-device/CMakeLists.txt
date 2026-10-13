# SPDX-FileCopyrightText: Copyright (c) 2026 Infineon Technologies AG,
# SPDX-FileCopyrightText: or an affiliate of Infineon Technologies AG. All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# SEGGER emUSB-Device integration for Zephyr

if(CONFIG_EMUSB_DEVICE)

  # The west-managed emusb-device module supplies the vendor headers and
  # prebuilt libraries. This SDK directory supplies its Zephyr integration
  # files through MODULE_EXT_ROOT.
  get_filename_component(EMUSB_DIR ${ZEPHYR_EMUSB_DEVICE_MODULE_DIR} ABSOLUTE)

  if(NOT EXISTS ${EMUSB_DIR}/USBD/USB.h)
    message(FATAL_ERROR
      "emUSB-Device sources not found at ${EMUSB_DIR}. Run 'west update' so "
      "the 'emusb-device' project is fetched.")
  endif()

  # -------------------------------------------------------------------------
  # Public headers (API + PSE84 hardware driver declarations).
  # -------------------------------------------------------------------------
  zephyr_include_directories(
    ${EMUSB_DIR}/USBD
    ${EMUSB_DIR}/USBD/COMPONENT_PSE84
  )

  # Zephyr glue headers (ifx_emusb_usbhs.h).
  zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

  # Application-supplied emUSB configuration headers (USB_Conf.h, ...).
  if(NOT "${CONFIG_EMUSB_DEVICE_CONFIG_PATH}" STREQUAL "")
    zephyr_include_directories(${CONFIG_EMUSB_DEVICE_CONFIG_PATH})
  endif()

  # -------------------------------------------------------------------------
  # Select the matching pre-compiled archive.
  #
  # Layout (GCC / arm-zephyr-eabi is ABI-compatible with TOOLCHAIN_GCC_ARM):
  #   USBD/COMPONENT_USBD_BASE/COMPONENT_PSE84/<CFG>/[COMPONENT_USBD_EMFILE/
  #     COMPONENT_EMFILE_<FAT>/]COMPONENT_<CORE>/COMPONENT_<FP>/
  #     TOOLCHAIN_GCC_ARM/gcc.a
  # -------------------------------------------------------------------------
  if(CONFIG_EMUSB_DEVICE_DEBUG_BUILD)
    set(_emusb_cfg "CONFIG_Debug")
  else()
    set(_emusb_cfg "CONFIG_Release")
  endif()

  if(CONFIG_CPU_CORTEX_M55)
    set(_emusb_core "COMPONENT_CM55")
  elseif(CONFIG_CPU_CORTEX_M33)
    set(_emusb_core "COMPONENT_CM33")
  else()
    message(FATAL_ERROR "emUSB-Device: unsupported CPU core (need Cortex-M33 or M55).")
  endif()

  if(CONFIG_FP_HARDABI)
    set(_emusb_fp "COMPONENT_HARDFP")
  else()
    set(_emusb_fp "COMPONENT_SOFTFP")
  endif()

  set(_emusb_base ${EMUSB_DIR}/USBD/COMPONENT_USBD_BASE/COMPONENT_PSE84/${_emusb_cfg})

  # The per-config USB_Conf.h (compile-time config the library was built with)
  # is required by USB.h -> USB_ConfDefaults.h.
  zephyr_include_directories(${_emusb_base})

  if(CONFIG_EMUSB_DEVICE_EMFILE_FAT16)
    set(_emusb_lib_dir ${_emusb_base}/COMPONENT_USBD_EMFILE/COMPONENT_EMFILE_FAT16)
  elseif(CONFIG_EMUSB_DEVICE_EMFILE_FAT32)
    set(_emusb_lib_dir ${_emusb_base}/COMPONENT_USBD_EMFILE/COMPONENT_EMFILE_FAT32)
  else()
    set(_emusb_lib_dir ${_emusb_base})
  endif()

  set(_emusb_lib ${_emusb_lib_dir}/${_emusb_core}/${_emusb_fp}/TOOLCHAIN_GCC_ARM/gcc.a)

  if(NOT EXISTS ${_emusb_lib})
    message(FATAL_ERROR
      "emUSB-Device: no pre-built library for this configuration:\n"
      "  ${_emusb_lib}\n"
      "Check the core (${_emusb_core}), FP ABI (${_emusb_fp}) and build "
      "variant (${_emusb_cfg}) against the archives shipped in emusb-device.")
  endif()

  add_library(emusb_device_prebuilt STATIC IMPORTED GLOBAL)
  set_target_properties(emusb_device_prebuilt PROPERTIES IMPORTED_LOCATION ${_emusb_lib})
  zephyr_link_libraries(emusb_device_prebuilt)

  # -------------------------------------------------------------------------
  # OS abstraction layer (compiled from source so it binds to the Zephyr
  # kernel via the Infineon abstraction-rtos COMPONENT_ZEPHYR layer).
  # -------------------------------------------------------------------------
  zephyr_library_named(emusb_device)

  zephyr_library_sources(${EMUSB_DIR}/OS/usbd_os_abs_rtos.c)

  # Zephyr-native USBHS controller bring-up + interrupt wiring.
  zephyr_library_sources(${CMAKE_CURRENT_SOURCE_DIR}/src/ifx_emusb_usbhs.c)

  # DMA driver glue (cache maintenance + DMA-address validation).
  zephyr_library_sources_ifdef(CONFIG_EMUSB_DEVICE_DMA
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ifx_emusb_dma.c)

  # Due to the order in which files are pulled, in this environment. Explicitly
  # compiling with infineon_kconfig.h is needed for successful build.
  zephyr_library_compile_options(
    -imacros ${ZEPHYR_BASE}/modules/hal_infineon/infineon_kconfig.h)

  zephyr_library_compile_definitions(COMPONENT_PSE84)

  if(CONFIG_EMUSB_DEVICE_USE_PDL)
    zephyr_library_compile_definitions(USBD_USE_PDL=1)
  endif()

  if(CONFIG_EMUSB_DEVICE_RTOS_AWARE)
    zephyr_library_compile_definitions(COMPONENT_RTOS_AWARE)

    # cyabs_rtos.h from the full abstraction-rtos asset + the Zephyr backend
    # headers/sources (mirrors the hal_infineon ml-middleware integration).
    zephyr_include_directories(
      ${ZEPHYR_HAL_INFINEON_MODULE_DIR}/abstraction-rtos/include
      ${ZEPHYR_BASE}/modules/hal_infineon/abstraction-rtos/include/COMPONENT_ZEPHYR)

    if(CONFIG_USE_INFINEON_ABSTRACTION_RTOS AND !CONFIG_PSOC_EDGE_M55_SRF_SUPPORT)
    zephyr_library_sources(
      ${ZEPHYR_BASE}/modules/hal_infineon/abstraction-rtos/source/COMPONENT_ZEPHYR/cyabs_rtos_zephyr.c)
    endif()
  endif()

endif() # CONFIG_EMUSB_DEVICE
