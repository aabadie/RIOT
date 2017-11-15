#!/usr/bin/env python3

# Copyright (C) 2017 Alexandre Abadie <alexandre.abadie@inria.fr>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import os
import sys

EXPECTED_HELP = (
    'Command              Description',
    '---------------------------------------',
    'reboot               Reboot the node',
    'ifconfig             Configure network interfaces',
    '>'
)

EXPECTED_IFCONFIG = (
    r'ifconfig',
    r'Iface  \d+   HWaddr: ([0-9a-f]{2}:){1,}([0-9a-f]{2}) (Channel: [11,26]  )?(Page: \d+  )?(NID: 0x[0-9a-f]{2})?',
    r'       Source address length: \d+',
    r'       Statistics for Layer 2',
    r'        RX packets \d+  bytes \d+',
    r'        TX packets \d+ \(Multicast: \d+\)  bytes \d+',
    r'        TX succeeded \d+ errors 0'
)


def testfunc(child):
    child.sendline('help')
    for line in EXPECTED_HELP:
        child.expect_exact(line)

    child.sendline('ifconfig')
    for line in EXPECTED_IFCONFIG:
        child.expect(line)

if __name__ == "__main__":
    sys.path.append(os.path.join(os.environ['RIOTBASE'], 'dist/tools/testrunner'))
    from testrunner import run
    sys.exit(run(testfunc))
