POST TCPWM MTB-STL Sample
#########################

Overview
********

This sample enables the MTB-STL counter, PWM duty-cycle, and PWM gate/kill
tests. ``main()`` reports their boot results using ``post_get_summary()``.
The board configuration files select the tests; the overlays select their
TCPWM instances, clocks, and GPIOs.

Hardware Setup
**************

Use ``kit_psc3m5_evk`` or ``cy8cproto_041tp`` with the MTB-STL module available
in the west workspace. Feedback differs by hardware:

* PSC3M5: no feedback jumper is required. PWM1_5 on P4.2 uses internal
  line-output status; the configured P6.4 input is not sampled by this path.
  The separate gate/kill test uses PWM1_4 on P4.0.
* CY8CPROTO-041TP: connect P2.2 (PWM0_3 output) to P2.3 (GPIO input). The
  separate gate/kill test uses PWM0_4 on P2.4.

Leave these pins free of other drivers. Connect the board's serial console
before resetting it to capture boot results.

Test Coverage and Recovery
**************************

All three wrappers run at boot without runtime permission:

* ``CONFIG_POST_MTB_STL_COUNTER`` / ``mtb_stl_counter`` checks counter
  progress and interrupt completion using vendor timing limits.
* ``CONFIG_POST_MTB_STL_PWM`` / ``mtb_stl_pwm`` checks a nominal one-third
  duty cycle. The accepted sampled LOW/HIGH ratio is between 1.875 and 2.125.
  PSC3M5 internal feedback does not test external pin continuity.
* ``CONFIG_POST_MTB_STL_PWM_GATEKILL`` / ``mtb_stl_pwm_gatekill`` first
  verifies counter movement, issues software kill, then verifies stopping.
  It does not test an external kill-input route. A counter that was already
  stuck cannot pass both phases.

Reserve separate TCPWM instances, outputs, IRQs, and clocks for the tests.
Do not share them with application PWM/counter users or the core sample's
clock/interrupt tests without reconciling the overlays.

The counter and duty-cycle wrappers install vendor configuration/IRQ handling
without restoring the previous state. Do not assume every failure path leaves
them stopped. The gate/kill wrapper disables its PWM after the running test
completes or fails, but leaves the test configuration installed. Quiesce and
restore the intended runtime timer/PWM and IRQ configuration before reuse;
none of these samples implements full recovery.

``CONFIG_POST_MTB_STL_TEST_TIMEOUT_MS`` applies to the gate/kill stop wait.
Counter and duty-cycle measurements use their vendor timing limits instead.
Initialization/readiness errors in gate/kill return ERROR; failed checks and
stop timeout return FAIL. Inspect logs for the other wrappers' setup errors
as well as their final measurement result.

See the `TCPWM test contract
<../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/tcpwm/README.rst>`_
for full details
(standard west workspace layout).

Building and Running
********************

From the west workspace root:

.. code-block:: shell

   west build -b kit_psc3m5_evk -d build-post-tcpwm \
     ifx-zephyr-sdk/samples/post/post-tcpwm-test-mtb-stl
   west flash -d build-post-tcpwm

Use ``-b cy8cproto_041tp`` with a separate build directory for the PSoC4 board.

Results
*******

A successful run with all three tests enabled prints:

.. code-block:: console

   POST summary: 3 passed, 0 failed, 0 skipped

Twister scenario ``sample.post.mtb_stl.tcpwm`` requires at least one passed
test, zero failures (including errors), and zero skips. A startup banner alone
does not pass. Use ``post list`` and ``post status`` to inspect individual results.
FAIL and ERROR both count as failures. None of these wrappers can be rerun
using ``post run``. Check all three entries explicitly; the harness only
requires at least one pass.
