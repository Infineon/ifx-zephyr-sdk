POST UART MTB-STL Sample
########################

Overview
********

This sample enables ``CONFIG_POST_MTB_STL_UART_LOOPBACK`` and
``CONFIG_POST_MTB_STL_COMM``. ``main()`` reports the UART loopback and
communication test results using ``post_get_summary()`` after boot POST.

Hardware Setup
**************

Use ``kit_psc3m5_evk`` or ``cy8cproto_041tp`` with the MTB-STL module available
in the west workspace. Fit the following jumpers before reset:

* PSC3M5 loopback: P3.3 TX (J5.29) to P3.2 RX (J5.31).
* PSC3M5 communication: P7.1 master TX (J5.14) to P8.1 slave RX (J5.13),
  and P8.3 slave TX (J5.17) to P7.2 master RX (J5.20).
* CY8CPROTO-041TP loopback: P2.1 TX to P2.0 RX.
* CY8CPROTO-041TP communication: P4.1 master TX to P2.4 slave RX, and
  P2.5 slave TX to P4.0 master RX.

The test UARTs are separate from the console. Do not attach another transmitter
to the test pins. Connect the board's serial console before resetting it to
capture boot results.

Test Coverage and Recovery
**************************

Both wrappers run at boot without runtime permission:

* ``CONFIG_POST_MTB_STL_UART_LOOPBACK`` registers ``mtb_stl_uart_loopback``.
  The sample uses physical TX-to-RX feedback, not internal SCB loopback.
  The wrapper aborts test-context receive/transmit operations and clears
  FIFOs; the vendor completion value 3 maps to PASS.
* ``CONFIG_POST_MTB_STL_COMM`` registers ``mtb_stl_communication``.
  It exchanges a ten-byte payload with slave address 2 using two UARTs and
  a dedicated TCPWM timeout counter. PASS requires the master to report
  completed communication.

Reserve all test UARTs, pins, IRQs, clocks, and the communication timer.
Do not start normal UART transfers, receive callbacks, or ring-buffer
sessions on them during testing. The communication wrapper installs vendor
ISRs for both SCBs and the timer; the wrappers do not provide complete
driver ownership arbitration or restore the application's previous session.

After success, failure, or timeout, stop outstanding test activity and restore
runtime configuration, IRQ handling, buffers, and callbacks before reuse.
A PASS does not prove that the normal UART driver is ready, and the sample
does not implement that recovery.
``CONFIG_POST_MTB_STL_TEST_TIMEOUT_MS`` bounds the loopback polling and each
communication wait phase separately, not the total communication duration
or a vendor call that does not return.

See the `SCB test contract
<../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/scb/README.rst>`_
for full details
(standard west workspace layout).

Building and Running
********************

From the west workspace root:

.. code-block:: shell

   west build -b kit_psc3m5_evk -d build-post-uart \
     ifx-zephyr-sdk/samples/post/post-uart-test-mtb-stl
   west flash -d build-post-uart

Use ``-b cy8cproto_041tp`` with a separate build directory for the PSoC4 board.

Results
*******

A successful run with both tests enabled prints:

.. code-block:: console

   POST summary: 2 passed, 0 failed, 0 skipped

Twister scenario ``sample.post.mtb_stl.uart`` requires at least one passed
test, zero failures (including errors), and zero skips. A startup banner alone
does not pass. Use ``post list`` and ``post status`` to inspect individual results.
FAIL and ERROR both count as failures. Neither wrapper is eligible for
``post run``. Verify both named entries passed; the harness's minimum of one
pass does not establish that both expected tests were enabled.
