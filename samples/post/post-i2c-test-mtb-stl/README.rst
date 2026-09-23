POST I2C MTB-STL Sample
#######################

Overview
********

This sample enables ``CONFIG_POST_MTB_STL_I2C`` and reports the I2C master/slave
self-test results using ``post_get_summary()`` in ``main()`` after boot POST.

Hardware Setup
**************

Use ``kit_psc3m5_evk`` or ``cy8cproto_041tp`` with the MTB-STL module available
in the west workspace. Wire the two on-chip SCBs to the same bus before reset:

* PSC3M5: connect master SCL P7.0 (J5.18) to slave SCL P8.1 (J5.13), and
  master SDA P7.1 (J5.14) to slave SDA P8.3 (J5.17).
* CY8CPROTO-041TP: connect master SCL P6.3 to slave SCL P2.0, and master SDA
  P6.4 to slave SDA P2.1.

Fit bus pull-ups, typically 2.2 to 4.7 kOhm to the board's I/O supply, if they
are not already present. Disconnect other devices that could drive this test
bus. The overlays select slave address 8.

Capture the serial console from reset. On CY8CPROTO-041TP the overlay moves
the console to SCB3: TX P2.5, RX P2.4, 115200 baud. Use a voltage-compatible
USB-to-UART adapter with a common ground for that console.

Clock Configuration
*******************

The I2C devices use ``zephyr,deferred-init``; their clocks do not. Clock
drivers configure the dividers before POST. The wrapper connects each SCB to
its divider but does not calculate a divider value. The sample supplies a
2 MHz internal SCB clock, distinct from the SCL bus frequency:

* PSC3M5: CLKPATH2 at 240 MHz feeds HF2 and HF4 through separate /3 dividers.
  The board configures HF2; the overlay configures HF4. SCB2 uses group 4/HF2,
  and SCB5 uses group 6/HF4. Each peripheral divider uses ``clock-div = <40>``
  to obtain 80 MHz / 40 = 2 MHz.
* CY8CPROTO-041TP: each peripheral divider uses ``clock-div = <24>`` to obtain
  48 MHz / 24 = 2 MHz from HFCLK.

The normal I2C drivers configure their own divider values during handoff.

Boot Ownership and Driver Handoff
*********************************

This is a boot test, not a runtime-safe transfer service. Reserve both SCBs,
their pins, clocks, and IRQs; prevent all normal I2C users from accessing
them until successful handoff. The wrapper applies pinctrl, initializes test
contexts and buffers, and temporarily routes the two IRQs through a private hardware-vector
copy. Other vectors and the original Zephyr tables are preserved; the MPU is
not disabled. Shared-interrupt support is not required.

After the vendor test, cleanup stops/deinitializes the test SCBs
and restores the original interrupt routing. Only vendor PASS and successful
cleanup allow first-time ``device_init()`` of both normal drivers. A vendor
FAIL, timeout, or setup/cleanup ERROR leaves them uninitialized. A subsequent
driver-initialization error is also POST ERROR; an earlier successfully
initialized controller is not rolled back.

Check both the overall POST result and device readiness before any normal
operation. Driver APIs do not enforce the POST result. Defer dependent
I2C devices too, then initialize those clients after handoff. The wrapper does
not initialize them automatically and cannot take over an already-running
controller.

The sample reports the boot POST result only. It does not register a Zephyr
I2C target or verify normal-driver transfers after handoff.
``CONFIG_POST_MTB_STL_TEST_TIMEOUT_MS`` bounds wrapper polling between
vendor calls and shutdown; it cannot interrupt a vendor function that fails
to return.

See the `SCB test contract
<../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/scb/README.rst>`_
for lifecycle restrictions and failure handling. Cross-repository links assume the standard
west workspace layout.

Building and Running
********************

From the west workspace root:

.. code-block:: shell

   west build -b kit_psc3m5_evk -d build-post-i2c \
     ifx-zephyr-sdk/samples/post/post-i2c-test-mtb-stl
   west flash -d build-post-i2c

Use ``-b cy8cproto_041tp`` with a separate build directory for the PSoC4 board.

Results
*******

The application prints the boot summary without rerunning the tests:

.. code-block:: console

   POST summary: 1 passed, 0 failed, 0 skipped

Twister scenario ``sample.post.mtb_stl.i2c`` requires at least one passed test,
zero failures (including errors), and zero skips in the POST summary.
Use ``post list`` and ``post status`` to inspect results.
FAIL and ERROR both count as failures. ``post run`` cannot rerun this wrapper:
it has no runtime permission. Reset only after checking wiring and ensuring
that the test resources can be used exclusively again.
