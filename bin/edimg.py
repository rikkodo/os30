#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import struct

SIZE = 1440 * 1024
SECTOR = 512
UNIT = 16

IPL = ""
OUTFILE = "void.img"
TMPFILE = "tmp.img"
NASMOPT = ""

FIXED = [
    # ~~~~~
    [0x0200, [0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]],  # 0x0200
    # ~~~~~
    [0x1400, [0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]],  # 0x1400
    # ~~~~~
    [-1,     [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]],  # 0x0000
]


# 引き数チェッカ
def chkarg():
    global IPL
    global OUTFILE
    global NASMOPT

    i = 1
    while(i < len(sys.argv)):
        if (sys.argv[i] == "-ipl"):
            i += 1
            IPL = sys.argv[i]
        elif (sys.argv[i] == "-o"):
            i += 1
            OUTFILE = sys.argv[i]
        elif (sys.argv[i] == "-l"):
            i += 1
            NASMOPT += " -l %s" % sys.argv[i]

        i += 1
    if (IPL == ""):
        print >> sys.stderr, "%s: NO IPL Assigned." % sys.argv[0]
        exit(1)
    return


def doasm():
    ret = os.system("nasm %s -o %s %s" % (NASMOPT, TMPFILE, IPL))
    if (ret != 0):
        print >> sys.stderr, "%s: NASM ERROR." % sys.argv[0]
        exit(1)
    return


def catimg():
    infile = open(TMPFILE, "rb")
    outfile = open(OUTFILE, "wb")

    i = 0
    f = 0

    while (i < SIZE):
        # IPL_MODER
        if (i < SECTOR):
            buf = infile.read(UNIT)
            outfile.write(buf)
        elif (i == FIXED[f][0]):
            for j in range(0, UNIT):
                outfile.write(struct.pack("B", FIXED[f][1][j]))
            f += 1
        else:
            for j in range(0, UNIT):
                outfile.write("\0")

        i += UNIT

    os.remove(TMPFILE)
    return


if __name__ == "__main__":
    if (len(sys.argv) == 1):
        print >> sys.stderr, """Usage %s -ipl <ipl.asm>
[-o <outfile.img>]
[-l <listfile.lst>]
""" % sys.argv[0]
        exit(1)

    chkarg()

    doasm()

    catimg()

    exit(0)
