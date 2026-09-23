POST SPI MTB-STL Sample
#######################

Overview
********

This sample enables ``CONFIG_POST_MTB_STL_SPI_LOOPBACK`` and reports the SPI
self-test results using ``post_get_summary()`` in ``main()`` after boot POST.

Hardware Setup
**************

Use ``kit_psc3m5_evk`` or ``cy8cproto_041tp`` with the MTB-STL module available
in the west workspace. Set up MOSI-to-MISO loopback before reset:

* PSC3M5: connect P7.1 (MOSI, J5.14) to P7.2 (MISO, J5.20) on SCB2.
* CY8CPROTO-041TP: connect P1.5 (MOSI) to P1.6 (MISO) on SCB1.

Disconnect external SPI devices that could drive MISO during the test.
The board overlays configure the SCB, clock, and pin assignments. Connect the
board's serial console before resetting it to capture boot results.

Ownership and Recovery
**********************

The test runs at boot without runtime permission. It installs an STL ISR
and context, initializes/enables the selected SCB, clears FIFOs, and polls
the loopback transfer. Completion maps to PASS; polling timeout maps to FAIL.
Initialization errors are logged, but the wrapper continues to the vendor
test, so inspect diagnostics as well as the summary.

Reserve the SCB, pins, clock, and IRQ for this test. There is no automatic
driver ownership guard or restoration on success/failure. Do not run normal
SPI transfers concurrently or assume that a PASS hands the device back to
the driver. Quiesce test activity and restore the runtime configuration and
interrupt handling before reuse; FIFO clearing discards prior data.

``CONFIG_POST_MTB_STL_TEST_TIMEOUT_MS`` bounds wrapper polling between
vendor calls, not every internal wait. This checks the selected loopback
path and format, not an attached SPI device's behavior.
See the `SCB test contract
<../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/scb/README.rst>`_ for details
(standard west workspace layout).

Building and Running
********************

From the west workspace root:

.. code-block:: shell

   west build -b kit_psc3m5_evk -d build-post-spi \
     ifx-zephyr-sdk/samples/post/post-spi-test-mtb-stl
   west flash -d build-post-spi

Use ``-b cy8cproto_041tp`` with a separate build directory for the PSoC4 board.

Results
*******

The application prints the boot summary without rerunning the tests:

.. code-block:: console

   POST summary: 1 passed, 0 failed, 0 skipped

Twister scenario ``sample.post.mtb_stl.spi`` requires at least one passed test,
zero failures (including errors), and zero skips. A startup banner alone does
not pass. Use ``post list`` and ``post status`` to inspect individual results.
FAIL and ERROR both count as failures. ``post run`` cannot rerun this wrapper;
check wiring and resource ownership before another boot test.
