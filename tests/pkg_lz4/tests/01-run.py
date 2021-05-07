#!/usr/bin/env python3

# Copyright (C) 2021 Inria
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import sys
from testrunner import run


def testfunc(child):
    child.expect(r"Data compressed with success \(ratio: (\d+.\d+)\)\r\n")
    ratio = float(child.match.group(1))
    assert ratio < 1, "No compression (ratio: {})".format(ratio)
    child.expect_exact("Data decompressed with success!")
    child.expect_exact("Validation done, decompressed string:")
    child.expect_exact(
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Lorem ipsum "
        "dolor site amat."
    )


if __name__ == "__main__":
    sys.exit(run(testfunc))
