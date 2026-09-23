POST Comparator MTB-STL Sample
##############################

Overview
********

This sample enables ``CONFIG_POST_MTB_STL_COMPARATOR`` and reports the LPCOMP
self-test results using ``post_get_summary()`` in ``main()`` after boot POST.

Hardware Setup
**************

Use ``kit_psc3m5_evk`` with the MTB-STL module available in the west workspace.
The overlay selects LPCOMP0 channel 0, with positive input P8.0 (J5.19) and
negative input P8.1 (J5.13).

``CONFIG_POST_COMP_EXPECT_HIGH=y`` expects the positive input to be higher than
the negative input. Apply stable voltages within the comparator's specified
input range, with a common ground; do not leave the inputs floating. Connect
the board's serial console before resetting it to capture boot results.

Coverage and Resource Ownership
********************************

``mtb_stl_comparator`` runs at boot without runtime permission. An unready
device or an output different from the configured expectation produces FAIL.
The current wrapper/vendor call only reads and compares the comparator
output; it does not change routing or automatically test both polarities.

.. warning::

   One invocation checks only one state. A comparator stuck at the expected
   level can pass. Automatic two-state stimulus or AMUX input swapping is
   not implemented. A separate LOW-state test requires opposite input
   ordering and ``CONFIG_POST_COMP_EXPECT_HIGH=n`` for that run.

Use only voltages within the specified comparator common-mode range and
avoid disturbing another comparator client while applying test inputs.
The wrapper reads the comparator without reconfiguring it. Restore application
stimulus or wiring/routing as needed before normal use; PASS is not proof of transition
response or full comparator fault coverage.

See the `analog test contract
<../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/analog/README.rst>`_
for details
(standard west workspace layout).

Building and Running
********************

From the west workspace root:

.. code-block:: shell

   west build -b kit_psc3m5_evk -d build-post-comparator \
     ifx-zephyr-sdk/samples/post/post-comparator-test-mtb-stl
   west flash -d build-post-comparator

Results
*******

The application prints the boot summary without rerunning the tests:

.. code-block:: console

   POST summary: 1 passed, 0 failed, 0 skipped

Twister scenario ``sample.post.mtb_stl.comparator`` requires at least one passed
test, zero failures (including errors), and zero skips. A startup banner alone
does not pass. Use ``post list`` and ``post status`` to inspect individual results.
FAIL and ERROR both count as failures. ``post run`` cannot rerun this wrapper;
use a controlled new boot after changing the stimulus/expectation.
