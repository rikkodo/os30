#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os

SIZE = 1440 * 1024

IPL = ""
OUTFILE = "void.img"


# 引き数チェッカ
def chkarg():
    global IPL
    global OUTFILE
    i = 1
    while(i < len(sys.argv)):
        if (sys.argv[i] == "-ipl"):
            i += 1
            IPL = sys.argv[i]
        elif (sys.argv[i] == "-o"):
            i += 1
            OUTFILE = sys.argv[i]

        i += 1
    if (IPL == ""):
        print >> sys.stderr, "NO IPL Assigned."
        exit(1)


def doasm():
    ret = os.system("nasm %s" % IPL)
    if (ret != 0):
        print >> sys.stderr, "NASM ERROR."
        exit(1)


if __name__ == "__main__":
    if (len(sys.argv) == 0):
        print >> sys.stderr, "Usage %s -ipl <ipl.asm> [-o <outfile.img>]" % sys.argv[0]
        exit(1)

    chkarg()

    doasm()

    exit(0)
