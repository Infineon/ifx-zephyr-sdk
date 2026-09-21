#!/usr/bin/env python3
"""Vendor-class bulk-endpoint throughput test for the emUSB vendor_bulk sample.

The ifx-zephyr-sdk sample samples/emusb/vendor_bulk exposes a vendor-class
(0xFF) interface with a bulk OUT and bulk IN endpoint on the SEGGER
emUSB-Device stack. In producer mode the device streams a counter pattern on
bulk IN; in loopback mode it echoes OUT back to IN.

Defaults for this sample:
    VID=0x2fe3, PID=0x000a, Interface 0, EP OUT 0x01, EP IN 0x81

NOTE: emUSB assigns the bulk endpoint addresses via USBD_AddEPEx at runtime.
With a single bulk interface these are typically OUT 0x01 / IN 0x81, but if
enumeration shows different addresses (check `lsusb -v`), override with
--ep-out / --ep-in.

Usage (Windows):
    1. Install Zadig (https://zadig.akeo.ie) and replace the driver bound to
       interface 0 of the device with WinUSB.
    2. pip install pyusb libusb-package
    3. python vendor_bulk_speed_test.py
"""

from __future__ import annotations

import argparse
import sys
import threading
import time

try:
    import libusb_package
    import usb.core
    import usb.util
except ImportError:
    sys.stderr.write(
        "Missing dependency. Install with:\n  pip install pyusb libusb-package\n"
    )
    sys.exit(1)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--vid", type=lambda x: int(x, 0), default=0x2FE3)
    p.add_argument("--pid", type=lambda x: int(x, 0), default=0x000A)
    p.add_argument("--interface", type=int, default=0)
    p.add_argument("--ep-out", type=lambda x: int(x, 0), default=0x01)
    p.add_argument("--ep-in", type=lambda x: int(x, 0), default=0x81)
    p.add_argument("--xfer", type=int, default=64 * 1024,
                   help="Per-transfer size in bytes (default 65536)")
    p.add_argument("--seconds", type=float, default=10.0,
                   help="Test duration per direction (default 10s)")
    p.add_argument("--mode", choices=("in", "out", "both", "in-async"),
                   default="both")
    p.add_argument("--queue", type=int, default=8,
                   help="Async queue depth for in-async mode (default 8)")
    p.add_argument("--timeout", type=int, default=5000,
                   help="Per-transfer timeout in ms")
    return p.parse_args()


def find_device(vid: int, pid: int) -> usb.core.Device:
    backend = libusb_package.get_libusb1_backend()
    dev = usb.core.find(idVendor=vid, idProduct=pid, backend=backend)
    if dev is None:
        raise SystemExit(
            f"Device {vid:#06x}:{pid:#06x} not found. "
            "Make sure the board is enumerated and a WinUSB driver is bound."
        )
    return dev


def benchmark_in(dev, ep_in: int, xfer: int, seconds: float, timeout: int) -> None:
    print(f"[IN ] reading from EP {ep_in:#04x}, xfer={xfer}B, {seconds}s ...")
    total = 0
    deadline = time.monotonic() + seconds
    t0 = time.monotonic()
    while time.monotonic() < deadline:
        try:
            data = dev.read(ep_in, xfer, timeout=timeout)
        except usb.core.USBTimeoutError:
            print("  timeout"); break
        total += len(data)
    elapsed = time.monotonic() - t0
    mib = total / (1024 * 1024)
    print(f"[IN ] {mib:.2f} MiB in {elapsed:.2f}s = {mib/elapsed:.3f} MiB/s")


def benchmark_in_async(vid: int, pid: int, interface: int, ep_in: int,
                       xfer: int, seconds: float, queue: int) -> None:
    """Pipelined IN benchmark using libusb1 async transfers.

    Keeps `queue` transfers in flight at all times so the device never has to
    wait for the host between submissions.
    """
    try:
        import usb1  # python-libusb1
    except ImportError:
        sys.stderr.write(
            "in-async mode needs `pip install libusb1`. Falling back to sync.\n"
        )
        return

    print(f"[ASY] reading from EP {ep_in:#04x}, xfer={xfer}B, queue={queue}, "
          f"{seconds}s ...")

    total = 0
    stop = False
    deadline_holder = [0.0]

    with usb1.USBContext() as ctx:
        handle = ctx.openByVendorIDAndProductID(vid, pid, skip_on_error=False)
        if handle is None:
            raise SystemExit("libusb1 could not open device")
        try:
            handle.setAutoDetachKernelDriver(True)
        except (usb1.USBError, AttributeError):
            pass
        handle.claimInterface(interface)

        transfers = []

        def cb(transfer):
            nonlocal total, stop
            status = transfer.getStatus()
            if status == usb1.TRANSFER_COMPLETED:
                total += transfer.getActualLength()
            elif status == usb1.TRANSFER_CANCELLED:
                return
            else:
                print(f"  transfer status {status}"); stop = True; return
            if not stop and time.monotonic() < deadline_holder[0]:
                try:
                    transfer.submit()
                except usb1.USBError:
                    stop = True

        try:
            for _ in range(queue):
                t = handle.getTransfer()
                t.setBulk(ep_in, xfer, callback=cb, timeout=5000)
                transfers.append(t)

            t0 = time.monotonic()
            deadline_holder[0] = t0 + seconds
            for t in transfers:
                t.submit()

            while time.monotonic() < deadline_holder[0] and not stop:
                ctx.handleEventsTimeout(0.1)
            # Measure elapsed at the deadline, BEFORE the cancel/drain
            # teardown below. Bytes only accrue for completed transfers during
            # the window, so including the teardown time here would divide the
            # window's bytes by a longer interval and under-report the rate.
            elapsed = time.monotonic() - t0
            stop = True
            for t in transfers:
                try:
                    t.cancel()
                except usb1.USBError:
                    pass
            # Drain pending callbacks
            for _ in range(20):
                ctx.handleEventsTimeout(0.05)
        finally:
            handle.releaseInterface(interface)
            handle.close()

    mib = total / (1024 * 1024)
    print(f"[ASY] {mib:.2f} MiB in {elapsed:.2f}s = {mib/elapsed:.3f} MiB/s")


def benchmark_out(dev, ep_out: int, xfer: int, seconds: float, timeout: int) -> None:
    print(f"[OUT] writing to EP {ep_out:#04x}, xfer={xfer}B, {seconds}s ...")
    payload = bytes((i & 0xFF) for i in range(xfer))
    total = 0
    deadline = time.monotonic() + seconds
    t0 = time.monotonic()
    while time.monotonic() < deadline:
        try:
            n = dev.write(ep_out, payload, timeout=timeout)
        except usb.core.USBTimeoutError:
            print("  timeout"); break
        total += n
    elapsed = time.monotonic() - t0
    mib = total / (1024 * 1024)
    print(f"[OUT] {mib:.2f} MiB in {elapsed:.2f}s = {mib/elapsed:.3f} MiB/s")


def benchmark_both(dev, ep_in: int, ep_out: int, xfer: int,
                   seconds: float, timeout: int) -> None:
    print(f"[DUP] full-duplex, xfer={xfer}B, {seconds}s ...")
    stats = {"in": 0, "out": 0}
    stop = threading.Event()

    def reader():
        while not stop.is_set():
            try:
                data = dev.read(ep_in, xfer, timeout=timeout)
                stats["in"] += len(data)
            except usb.core.USBTimeoutError:
                break

    def writer():
        payload = bytes((i & 0xFF) for i in range(xfer))
        while not stop.is_set():
            try:
                n = dev.write(ep_out, payload, timeout=timeout)
                stats["out"] += n
            except usb.core.USBTimeoutError:
                break

    t_r = threading.Thread(target=reader, daemon=True)
    t_w = threading.Thread(target=writer, daemon=True)
    t0 = time.monotonic()
    t_r.start(); t_w.start()
    time.sleep(seconds)
    stop.set()
    t_r.join(timeout=2); t_w.join(timeout=2)
    elapsed = time.monotonic() - t0
    mib_in = stats["in"] / (1024 * 1024)
    mib_out = stats["out"] / (1024 * 1024)
    print(f"[DUP] IN  {mib_in:.2f} MiB = {mib_in/elapsed:.3f} MiB/s")
    print(f"[DUP] OUT {mib_out:.2f} MiB = {mib_out/elapsed:.3f} MiB/s")


def main() -> None:
    args = parse_args()
    dev = find_device(args.vid, args.pid)

    # On Windows with WinUSB this is a no-op; on Linux it detaches kernel driver.
    try:
        if dev.is_kernel_driver_active(args.interface):
            dev.detach_kernel_driver(args.interface)
    except (NotImplementedError, usb.core.USBError):
        pass

    dev.set_configuration()
    usb.util.claim_interface(dev, args.interface)

    cfg = dev.get_active_configuration()
    intf = cfg[(args.interface, 0)]
    print(f"Device {args.vid:#06x}:{args.pid:#06x} interface {args.interface} "
          f"({intf.bInterfaceClass:#04x}/{intf.bInterfaceSubClass:#04x}/"
          f"{intf.bInterfaceProtocol:#04x})")
    for ep in intf:
        print(f"  EP {ep.bEndpointAddress:#04x} mps={ep.wMaxPacketSize}")

    try:
        if args.mode in ("in", "both"):
            benchmark_in(dev, args.ep_in, args.xfer, args.seconds, args.timeout)
        if args.mode == "out":
            benchmark_out(dev, args.ep_out, args.xfer, args.seconds, args.timeout)
        if args.mode == "both":
            benchmark_both(dev, args.ep_in, args.ep_out, args.xfer,
                           args.seconds, args.timeout)
        if args.mode == "in-async":
            usb.util.release_interface(dev, args.interface)
            usb.util.dispose_resources(dev)
            benchmark_in_async(args.vid, args.pid, args.interface, args.ep_in,
                               args.xfer, args.seconds, args.queue)
            return
    finally:
        usb.util.release_interface(dev, args.interface)
        usb.util.dispose_resources(dev)


if __name__ == "__main__":
    main()
