#!/usr/bin/env python3

# Copyright (C) 2026 Inria
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import sys
from testrunner import run


def testfunc(child):
    child.expect_exact("RIOT wdt_cb test application")
    child.expect(r"The watchdog will expire in (\d+)ms and reboot the MCU")
    timeout = int(child.match.group(1)) / 1e3
    # The callback prints SUCCESS right before the MCU is reset. On
    # platforms with a very short warning period the reset can cut the
    # message: only require an unambiguous prefix of it.
    child.expect(r"SUC", timeout=timeout + 1)
    # the interactive sync prompt printed after the reboot confirms the MCU
    # was actually reset by the watchdog
    child.expect_exact("Help: Press s to start test, r to print it is ready",
                       timeout=5)
    print("TEST PASSED")


if __name__ == "__main__":
    sys.exit(run(testfunc, echo=False))
