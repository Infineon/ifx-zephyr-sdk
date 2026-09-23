POST ADC MTB-STL Sample
#######################

Overview
********

This sample enables ``CONFIG_POST_MTB_STL_ANALOG`` and reports the ADC
self-test results using ``post_get_summary()`` in ``main()`` after boot POST.

Hardware Setup
**************

Use ``kit_psc3m5_evk`` or ``cy8cproto_041tp`` with the MTB-STL module available
in the west workspace.

PSC3M5 uses the MCPASS v1 CSG-to-SAR observability route: CSG0 DAC
slice 0 drives ADC0 channel 13 internally. No external voltage source or jumper
is required, and AN_A0 is not used. DAC codes 341 and 682 are checked against
12-bit ADC results 1365 and 2730. Both points must pass; a constant result
cannot satisfy both non-overlapping acceptance windows.

Reserve CSG0 DAC slice 0, the CSG-to-SAR route, ADC0 channel 13, and firmware
triggers 0 and 1 for this test. Do not run concurrent ADC, DAC, or comparator
operations using these resources. After acquiring the route, cleanup stops
the DAC and attempts to disconnect the internal route; disconnect failure
is reported as FAIL. It does not restore a running waveform or the previous
ADC sequence.

The PSC3M5 sample uses the internal reference. These are one-third and
two-thirds full-scale ratio checks, not independent measurements of VDDA or
the reference voltage; shared-reference faults may be undetected.
``CONFIG_POST_MTB_STL_HPPASS_ADC_ACCURACY`` sets the tolerance in ADC counts
(default +/-100, configurable from 0 to 682). This is an integration setting
to characterize on hardware, not a device accuracy guarantee or a value
derived from ``CY_CFG_PWR_VDDA_MV``. It does not rely on the STL guide's
MCPASS v3 procedure being applicable to PSC3M5. ``CONFIG_POST_ADC_REF_VOLTAGE2``
does not select a single point on PSC3M5; both are always tested.

CY8CPROTO-041TP uses an external-input test on ADC0 channel 0, P2.0.
Apply a stable input at VDDA/3, or 2*VDDA/3 with
``CONFIG_POST_ADC_REF_VOLTAGE2`` enabled. Use a common ground and keep the input
within the board's analog voltage limits. The HPPASS tolerance setting does
not affect this PSoC4 path.

Connect the board's serial console before resetting it to capture boot results.

Ownership and Recovery
**********************

Both paths run at boot without runtime permission.

On PSoC4, the overlay sets ``zephyr,deferred-init`` on ADC0, not its clock.
The clock driver initializes ``peri_clk_div4`` at boot with ``clock-div = <30>``,
providing a 1.6 MHz SAR clock from 48 MHz HFCLK. The adaptation applies the
pins and connects the divider before running the vendor test with the normal
ADC IRQ disabled. Cleanup stops conversion, waits for idle, resets
the SAR and test routing, and clears pending status. Only a vendor PASS and
successful cleanup allow first-time initialization of the normal Zephyr
driver, which establishes its own context and enables its IRQ.

Test FAIL/ERROR leaves the device uninitialized; unsafe shutdown or failed
driver initialization reports ERROR. The wrapper does not take over an
already-ready device. Check the POST result and device readiness before
channel setup or reads; driver APIs do not enforce the POST result.
Defer any dependent devices and initialize those clients after handoff.
Do not initialize ADC0 manually to bypass a failed POST.

The sample reports the boot POST result only. It does not verify a
normal-driver conversion after handoff.

PSC3M5 uses the HPPASS path without deferred initialization or this handoff.
Keep all shared ADC/CSG/trigger clients excluded and restore their configuration
before reuse, even after the DAC has stopped and routing cleanup has succeeded.
This sample does not implement full HPPASS recovery.

``CONFIG_POST_MTB_STL_TEST_TIMEOUT_MS`` bounds the HPPASS stimulus waits and
PSoC4 shutdown, not every vendor or driver call.
On PSoC4 the supplied VDDA/reference constants are 5000 mV; keep the constants,
overlay, and input stimulus consistent when adapting to another supply.
Only one external voltage is tested per PSoC4 invocation.

See the `analog test contract
<../../../../modules/lib/ifx-zephyr-stl/zephyr-adaptation-layer/post/analog/README.rst>`_
for the full error, coverage, and recovery details. Links assume the standard
west layout.

Building and Running
********************

From the west workspace root:

.. code-block:: shell

   west build -b kit_psc3m5_evk -d build-post-adc \
     ifx-zephyr-sdk/samples/post/post-adc-test-mtb-stl
   west flash -d build-post-adc

Use ``-b cy8cproto_041tp`` with a separate build directory for the PSoC4 board.

Results
*******

The application prints the boot summary without rerunning the tests:

.. code-block:: console

   POST summary: 1 passed, 0 failed, 0 skipped

Twister scenario ``sample.post.mtb_stl.adc`` covers PSC3M5 and PSoC4 and requires
at least one passed test, zero failures (including errors), and zero skips
in the POST summary. A startup banner alone does not pass.
Use ``post list`` and ``post status`` to inspect individual results.
FAIL and ERROR both count as failures. The ADC wrapper cannot be rerun using
``post run``; resetting starts a new boot test, not a runtime restore operation.
