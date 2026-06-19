.. zephyr:code-sample:: afe
   :name: Audio Front-End (AFE) Integration Sample
   :relevant-api: gpio_interface

   Demonstrates integration of Infineon Audio Front-End middleware with Zephyr RTOS.

Overview
--------

This sample demonstrates the integration of Infineon's Audio Front-End (AFE) middleware 
with Zephyr RTOS on the PSE84 AI Kit. The AFE middleware provides advanced audio 
processing capabilities including:

- **Speech Enhancement**: Noise reduction and echo cancellation
- **Beamforming**: Directional audio capture from microphone arrays  
- **Audio Voice Core (AVC)**: Advanced speech processing algorithms
- **License Management**: Time-based licensing system for audio algorithms
- **Configurator Integration**: Generated configuration support from ModusToolbox

Key Features:

- LED activity indicator during operation
- Audio Front-End middleware initialization and testing
- License status checking for AFE and AVC components
- Integration with three audio modules via Zephyr west manifest
- Support for both CM33 and CM55 cores (CM55 recommended for audio)

Audio Modules Integrated
************************

This sample integrates three west-managed audio modules:

1. **audio-front-end**: Core AFE middleware with RTOS abstraction layer
2. **audio-voice-core**: Advanced speech processing with license management  
3. **mw-utilities**: Logging and utility functions for middleware

Technical Architecture
**********************

**Module Structure:**

- **Static Modules**: Generic audio processing code (unchanged across projects)
- **Project-Specific**: Generated configurator files in ``src/generated/``
- **RTOS Abstraction**: Custom compatibility layer mapping Infineon APIs to Zephyr kernel
- **License System**: Time-limited evaluation (90,000 calls ≈ 15 minutes)

**Core Dependencies:**

- ``CONFIG_CMSIS_DSP=y``: Signal processing library
- ``CONFIG_AVC=y``: Audio Voice Core components
- ``CONFIG_AUDIOFE=y``: Audio Front-End module  
- ``CONFIG_MWUTILS=y``: Middleware utilities

Building and Running
********************

**Prerequisites:**

- Zephyr SDK 0.17.4+
- West tool for manifest management
- PSE84 AI Kit hardware
- Infineon HAL drivers

**For CM33 core (basic functionality):**

.. zephyr-app-commands::
   :zephyr-app: samples/boards/infineon/kit_pse84_ai/afe
   :board: kit_pse84_ai/pse846gps2dbzc4a/m33
   :goals: build flash

**For CM55 core (recommended for audio processing):**

.. zephyr-app-commands::
   :zephyr-app: samples/boards/infineon/kit_pse84_ai/afe
   :board: kit_pse84_ai/pse846gps2dbzc4a/m55
   :goals: build flash

**West Module Management:**

The audio modules are automatically fetched via west manifest:

.. code-block:: bash

   west update
   west build -b kit_pse84_ai/pse846gps2dbzc4a/m55 samples/boards/infineon/kit_pse84_ai/afe

Configurator Files
******************

**Generated Configuration:**

The sample includes generated configurator files in ``src/generated/``:

- ``cy_afe_configurator_settings.h``: Audio configuration parameters
- ``cy_afe_configurator_settings.c``: Audio filter settings and coefficients

**Key Configuration Parameters:**

- Sample Rate: 16 kHz
- Frame Size: 10ms (160 samples)
- Input Channels: 2 (stereo microphone input)
- Enabled Filters: AEC, Beamforming, Dereverberation, HPF, Noise Suppression

**Customization:**

For production use, replace the generated files with output from the 
ModusToolbox Audio Front-End Configurator tool.

Expected Output
***************

The application will produce console output similar to::

    AFE Sample Application Starting
    ===================================
    AFE Board: kit_pse84_ai
    AFE Main: Audio Front-End module is enabled
    AFE Main: Testing module integration...
    AFE Main: AFE License check: VALID
    AFE Main: AVC License check: VALID
    AFE Main: Audio Front-End module integration test completed
    AFE Main: LED blink thread started in background
    AFE Main: Application running...
    AFE LED Blink Thread: Starting LED blink thread
    AFE LED Blink Thread: LED configured successfully
    AFE LED Blink Thread: LED state: ON (count: 1)
    AFE Main: Main thread heartbeat: 1

**LED Behavior:**

- LED0 blinks every 1 second during operation
- Main thread prints heartbeat every 5 seconds
- LED state changes are logged to console

License Information
*******************

**Evaluation License:**

The AFE middleware includes a time-based evaluation license:

- **Duration**: 90,000 audio processing calls (≈ 15 minutes)
- **Scope**: Covers all AFE components (AEC, BF, NS, HPF, ES)
- **Status**: Checked at runtime via ``cy_afe_lib_is_license_expired()``

**Production License:**

For production deployment, contact Infineon for commercial licensing options.

Troubleshooting
***************

**Common Issues:**

1. **West Module Fetch Failures**: 
   - Ensure GitLab access credentials are configured
   - Check network connectivity and proxy settings

2. **License Expiration**: 
   - License expires after 15 minutes of audio processing
   - Restart application to reset license timer

3. **Build Errors**: 
   - Verify all dependencies: ``CONFIG_CMSIS_DSP``, ``CONFIG_AVC``, ``CONFIG_AUDIOFE``
   - Ensure Infineon HAL drivers are properly configured

4. **Audio Processing Issues**:
   - Verify generated configurator files are present in ``src/generated/``
   - Check sample rate and frame size configuration (16kHz, 10ms)

**Debug Output:**

Enable additional logging by modifying ``prj.conf``:

.. code-block:: kconfig

   CONFIG_AUDIOFE_LOGGING=y
   CONFIG_LOG_DEFAULT_LEVEL=3

Technical References
********************

- **Audio Front-End Middleware**: Infineon ModusToolbox Audio Configurator
- **CMSIS-DSP Library**: ARM digital signal processing functions
- **Zephyr RTOS**: Real-time operating system with threading support
- **PSE84 AI Kit**: Infineon edge AI development platform
    AFE LED Blink Thread: LED state: ON (count: 3)
    ...