.. _post-mtb-stl-sample:

POST MTB-STL Sample Application
################################

Overview
********

This sample demonstrates the Power-On Self Test (POST) subsystem with
Infineon's MTB-STL (ModusToolbox Self-Test Library) integration for PSoC
devices. Board configuration files enable startup-register, CPU, RAM, stack,
flash, GPIO, watchdog, interrupt, clock, and DMA tests, plus FPU on PSC3M5.
After boot POST, ``main()`` calls ``post_get_summary()`` and prints the stored
results without rerunning the tests.

The ``ifx-zephyr-stl`` module provides both the generic POST framework and
the Infineon adaptation, including the public POST headers and devicetree
binding. The selected Zephyr integration supplies the board and
peripheral-driver support. Use only one provider of the POST framework and
binding.

Requirements
************

* ``cy8cproto_041tp`` or ``kit_psc3m5_evk``
* The workspace and vendor assets prepared using the
  `SDK setup instructions <../../../README.md#mtb-stl-post-tests>`_
* A serial console connected before reset to capture boot results
* A startup-register reference provisioned at the matching configuration
  checkpoint and preserved across flashing, as described below

The overlays reserve TCPWM and DMA resources for POST. The GPIO self-test
changes pin states; disconnect external loads or drivers on tested GPIOs.
The flash test calculates its reference checksum during the build and supports
standalone images only. MCUboot, TF-M, and partitioned/offset images are rejected
while the flash test is enabled.

Enabled Tests and Limits
************************

The following entries are selected by the current board configurations.
All run at boot; only the entries noted below have runtime permission.

* ``CONFIG_POST_MTB_STL_CPU_REGS`` / ``mtb_stl_cpu_regs``: CPU register
  checkerboard, not an exhaustive instruction test.
* ``CONFIG_POST_MTB_STL_CPU_PC`` / ``mtb_stl_cpu_pc``: execution of helpers
  placed at the overlay's two flash addresses. Check the map file when
  changing those addresses; the linker reserves the intervening space.
* ``CONFIG_POST_MTB_STL_PROGRAM_FLOW`` / ``mtb_stl_program_flow``: the
  vendor's instrumented path, not continuous application flow monitoring.
* ``CONFIG_POST_MTB_STL_FPU`` / ``mtb_stl_fpu``: FPU register checkerboard,
  PSC3M5 only here, also runtime-enabled.
* ``CONFIG_POST_MTB_STL_RAM_MARCH`` / ``mtb_stl_ram_march``: boot-only
  destructive test of the overlay's application RAM region using an
  exclusive save buffer outside ``zephyr,sram``. The protected prefix and
  save buffer are not covered; prevent DMA/concurrent access.
* ``CONFIG_POST_MTB_STL_STACK`` / ``mtb_stl_stack``: main-stack patterns
  initialized once before the thread starts, then checked at boot/runtime.
  Other stacks are not monitored. Schedule checks if ongoing detection is
  needed; unused-space/high-water scanning is no longer meaningful.
* ``CONFIG_POST_MTB_STL_FLASH`` / ``mtb_stl_flash``: boot/runtime Fletcher-64
  verification from the flash base up to the embedded checksum, excluding
  the checksum and all later bytes. Not whole-flash coverage or cryptographic
  authentication.
* ``CONFIG_POST_MTB_STL_GPIO`` / ``mtb_stl_gpio``: boot-only pull-up/pull-down
  checks. Both overlays currently select P3.0/P3.1 via
  ``gpio-port-masks = [00 00 00 03]``; disconnect conflicting loads/drivers.
* ``CONFIG_POST_MTB_STL_WDT`` / ``mtb_stl_wdt``: deliberately resets once,
  then recognizes the watchdog reset cause. Capture console output across
  the reset. The wrapper leaves WDT disabled when it returns.
* ``CONFIG_POST_MTB_STL_INTERRUPT`` / ``mtb_stl_interrupt``: checks the
  selected TCPWM interrupt path, not every vector or priority.
* ``CONFIG_POST_MTB_STL_CLOCK`` / ``mtb_stl_clock``: boot/runtime main-clock
  cross-check against WDT/ILO using the selected TCPWM. It changes watchdog
  configuration and leaves WDT disabled.
* ``CONFIG_POST_MTB_STL_DMA``: ``mtb_stl_dmac`` on the PSoC4 DMAC path or
  ``mtb_stl_dma_dw`` on the DW path. Checks vendor buffer transfers on the
  selected channel, not all DMA channels/modes. Unsupported IP registers
  no test and must not be mistaken for a pass.
* ``CONFIG_POST_MTB_STL_STARTUP_CFGREG`` /
  ``mtb_stl_run_startup_cfgreg``: read-only verification at ``POST_KERNEL``,
  priority 51, also runtime-enabled. It never saves a new reference.
  ``CONFIG_POST_MTB_STL_SAVE_STARTUP_CFGREG`` is enabled on both boards to
  expose the explicit save API and shell command, not an automatic save test.

``CONFIG_POST_MTB_STL_STACK_MARCH`` / ``mtb_stl_stack_march`` is available in
the module but is not enabled by these board configurations. It tests the
active main stack at boot, reserves twice the usable stack size for backup,
preserves live frames, and destroys unused contents. Its exclusions and
configuration constraints are documented in the memory-domain README.

Startup Reference Provisioning
******************************

Provisioning is the initial, explicitly authorized save of the known-good
configuration. It is a setup operation, not a test to repeat before every
verification. Subsequent boots and runtime checks compare against that same
saved reference without replacing it. Saving on every boot could overwrite
the original configuration with an unintended change and then falsely report
a match against the newly saved values.

.. warning::

   An unprovisioned board reports startup verification ERROR and does not
   satisfy the zero-failures harness. This sample never provisions on its own,
   even if the provisioning option is enabled.

Both sample board configurations enable
``CONFIG_POST_MTB_STL_SAVE_STARTUP_CFGREG`` by default, making
``stl_startup save`` available without an extra build option. For example,
from the west workspace root:

.. code-block:: shell

   west build -b kit_psc3m5_evk -d build-post-core \
     ifx-zephyr-sdk/samples/post/post-mtb-stl

Use ``-b cy8cproto_041tp`` with a separate directory for PSoC4. After checking
the checkpoint and storage requirements below, flash the sample image
using a procedure that preserves any existing reference. The command is
available only when both ``CONFIG_SHELL`` and the SAVE option are enabled.
Enabling SAVE does not invoke it. The module's Kconfig default remains off
for applications that do not explicitly select provisioning support.

The command explicitly calls ``mtb_stl_startup_provision()`` from
``<mtb_stl/startup.h>``. Applications without the shell can call this API and
handle its return value directly. It returns zero only after writing a blank
reference and passing readback comparison:

.. code-block:: console

   uart:~$ stl_startup save
   Startup reference save: PASS

Nonblank storage, including partial/corrupt records, is refused with
``-EALREADY``; this is not a successful save or proof of a valid reference:

.. code-block:: console

   uart:~$ stl_startup save
   Startup reference save: NOT SAVED (reference is not blank; no write performed)

This is expected on a board with an existing reference, including one saved
by earlier firmware. Repeating the command or resetting without erasing the
reference continues to report NOT SAVED. A verification PASS on that board
means the current registers match the reference, not that a new save occurred.
To exercise save PASS, use a controlled test board with the reserved reference
area deliberately prepared as blank and an approved register configuration.
Do not erase a working reference merely to obtain a PASS message.

Other errors print ``Startup reference save: FAIL (<negative errno>)`` and
return that error to the shell. For example, ``-EIO`` means a write or readback
failure, and ``-EBUSY`` means another startup-register operation is active.
The command never erases an existing reference or retries automatically.

Saving is not a POST test and is not executed by ``post run --all``. Its result
is printed immediately, not added to ``post status`` or ``post_get_summary()``.
After a successful save, use ``post list`` to find
``mtb_stl_run_startup_cfgreg``, then ``post run <test_id_hex>`` to update its
verification result. A boot-time ERROR already reported for blank storage
is not retroactively changed by the save command.

Provision at the same register-state checkpoint that verification expects,
not arbitrarily in ``main()``. The shell command saves the configuration at
the time it is invoked; it does not recreate the earlier ``POST_KERNEL``
checkpoint. Use it only at an approved, stable state. A successful shell save
and immediate runtime check do not prove that the next boot will match:
reset without erasing the reference and check boot verification as well.
If the boot and shell states differ, use an explicitly authorized pre-test
hook at the boot verification checkpoint, as described in the system-domain
README, rather than overwriting the reference after a failure. There is only
one baseline: later checks are useful only where the selected registers
should still match it.

Reserve the vendor flash rows and any affected erase sector; ensure no
overlap with the image, persistent data, or flash-checksum coverage. Kconfig
alone does not reserve them. Disable the SAVE option after commissioning,
retain verification, and preserve the reference across firmware updates and
debugger flashes. Use an approved service procedure if reprovisioning is
necessary; no automatic erase/reprovision path is provided.

See the `system-domain README
<../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/system/README.rst>`_
for the API contract,
checkpoint restrictions, and storage requirements.

Resource Recovery
*****************

PASS does not mean that test resources have been returned to normal drivers.
The timer/interrupt, clock, and DMA tests retain test configuration or IRQ/
descriptor state. Keep those resources reserved, or quiesce and restore them
before application use. The GPIO routine restores touched port configuration,
but cannot undo physical effects from changing a pin.

Configure and start the operational watchdog after POST. In particular, do
not invoke the runtime-enabled clock test while relying on that same watchdog
without an explicit ownership and recovery plan. Memory test failures require
an application failure policy; continuing to ``main()`` is not proof of safe
memory.

``CONFIG_POST_MTB_STL_TEST_TIMEOUT_MS`` bounds the flash/clock wrapper polling,
not every test or vendor call. The sample reports results but does not perform
complete resource recovery or schedule periodic runtime checks.

Building and Running
********************

From the west workspace root:

.. code-block:: shell

   west build -b kit_psc3m5_evk -d build-post-core \
     ifx-zephyr-sdk/samples/post/post-mtb-stl
   west flash -d build-post-core

Use ``-b cy8cproto_041tp`` with a separate build directory for the PSoC4 board.

Sample Output
=============

The summary format is shown below; counts depend on the board configuration
and actual test results:

.. code-block:: console

   POST summary: <passed> passed, <failed> failed, <skipped> skipped

Twister scenario ``sample.post.mtb_stl`` requires at least one passed test,
zero failures (including errors), and zero skips. A startup banner alone does
not pass. The board must have a matching startup reference before this
scenario is run; an ordinary flash that erases it invalidates this prerequisite.
``CONFIG_POST_HALT_ON_FAILURE=n`` allows the application to report
noncritical failures instead of treating arrival at ``main()`` as success.

Shell Commands
**************

The sample enables the POST shell module, providing the following commands:

* ``post list`` - Display all registered POST tests with their IDs and names
* ``post run <test_id_hex>`` - Execute one runtime-permitted test by hexadecimal ID
* ``post run --all`` - Execute all runtime-permitted tests
* ``post status`` - Show results of previously executed tests
* ``stl_startup save`` - Explicitly save a blank startup reference and report
  the result; enabled by default on both sample boards, absent if SAVE is disabled

Use ``post list`` to obtain IDs for the current build. Boot-only and destructive
tests are not rerun by the runtime command. FPU, stack-pattern, flash-checksum,
startup-register verification, and clock are the runtime-permitted entries;
the runtime flag does
not remove their resource restrictions. Later runtime results do not change
the summary line already printed by ``main()``. FAIL and ERROR both count as
failures; SKIP is not a pass. Check that all expected entries are present:
the harness's minimum of one pass does not prove full test selection.

Configuration
*************

The sample enables the following configuration options in ``prj.conf``:

* ``CONFIG_POST`` - Enable POST subsystem
* ``CONFIG_POST_VENDOR_MTB_STL`` - Enable MTB-STL test integration
* ``CONFIG_POST_SHELL`` - Enable POST shell commands
* ``CONFIG_SHELL`` - Enable Zephyr shell
* ``CONFIG_LOG`` - Enable logging subsystem

Individual test selections are in ``boards/<board>.conf``; peripheral resources
are in ``boards/<board>.overlay``.

References
**********

The following links assume the standard west workspace layout:

* `Module integration and recovery contract
  <../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/README.rst>`_
* `CPU tests
  <../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/cpu/README.rst>`_
* `Memory tests
  <../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/memory/README.rst>`_
* `System tests
  <../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/system/README.rst>`_
* `GPIO test
  <../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/gpio/README.rst>`_
* `MTB-STL Library <https://github.com/Infineon/mtb-stl>`_
