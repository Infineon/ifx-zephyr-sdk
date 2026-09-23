POST DAC MTB-STL Sample
#######################

Overview
********

This sample enables ``CONFIG_POST_MTB_STL_DAC`` and reports the DAC self-test
results using ``post_get_summary()`` in ``main()`` after boot POST.

Hardware Setup
**************

Use ``kit_psc3m5_evk`` with the MTB-STL module available in the west workspace.
The test routes HPPASS CSG0 DAC slice 0 internally to ADC0 channel 13 and checks
zero, midpoint, and full-scale DAC values. No external DAC-to-ADC jumper is
needed. The overlay configures the ADC channel and DAC trigger routing.
Connect the board's serial console before resetting it to capture boot results.

Coverage and Recovery
*********************

``mtb_stl_dac`` runs at boot without runtime permission. DAC codes
0, 511, and 1023 are checked against ADC results 0, 2047, and 4095 with
+/-50 ADC-count tolerance. All three must succeed for PASS. Device, routing,
setup, prime-read, or vendor-check failures produce FAIL.
This tests the combined DAC/reference/ADC path, not independent absolute
DAC accuracy.

Reserve CSG0 DAC slice 0, ADC0 channel 13, SAR group 0, the internal route,
and firmware triggers 0 and 1. These resources overlap the PSC3M5 ADC sample.
Do not combine the samples or run other ADC/DAC/routing clients concurrently
without an explicit ownership plan.

The wrapper leaves the internal DAC-to-ADC route configured, including on
early failures after routing. Although the vendor vector sequence stops the
DAC, the wrapper does not restore the previous DAC mode/trigger configuration
or ADC sequence, and does not guarantee complete cleanup on every failure.
Quiesce the shared path, disconnect test routing, and restore runtime
configuration before reuse. There is no driver-access guard
for this HPPASS wrapper and no automatic recovery in the sample.

``CONFIG_POST_MTB_STL_TEST_TIMEOUT_MS`` is not an overall deadline for this
DAC test. The vendor routine has its own guards and an SAR-busy wait without
a wrapper deadline.
See the `analog test contract
<../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/analog/README.rst>`_
for full details
(standard west workspace layout).

Building and Running
********************

From the west workspace root:

.. code-block:: shell

   west build -b kit_psc3m5_evk -d build-post-dac \
     ifx-zephyr-sdk/samples/post/post-dac-test-mtb-stl
   west flash -d build-post-dac

Results
*******

The application prints the boot summary without rerunning the tests:

.. code-block:: console

   POST summary: 1 passed, 0 failed, 0 skipped

Twister scenario ``sample.post.mtb_stl.dac`` requires at least one passed test,
zero failures (including errors), and zero skips. A startup banner alone does
not pass. Use ``post list`` and ``post status`` to inspect individual results.
FAIL and ERROR both count as failures. ``post run`` cannot rerun this wrapper;
a new boot test also requires exclusive ownership of the shared path.
