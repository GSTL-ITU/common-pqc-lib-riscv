#!/usr/bin/env python3
"""Cycle statistics from harness logs (UART logs of the board or the simulation).

  summarize.py build-fpga/r100/board.log [more logs ...] [--host build/host.log]

Per operation: run 0 (the fixed seed shared with the other cores), median,
minimum, maximum and mean over all runs, and cycles per instruction. With
--host, the digest lines of every run must equal those of the native build.
"""
import argparse
import re
import statistics
import sys


def parse(path):
    name, ops, digests, result = None, {}, [], None
    with open(path, errors="replace") as f:
        for line in f:
            line = line.replace("UART: ", "").strip()
            if m := re.match(r"(\S+) runs=\d+$", line):
                name = m.group(1)
            elif m := re.match(r"cycles (\S+) (\d+) (\d+) (\d+)$", line):
                ops.setdefault(m.group(1), []).append((int(m.group(2)), int(m.group(3)), int(m.group(4))))
            elif line.startswith("digest "):
                digests.append(line)
            elif line.startswith("result "):
                result = line
    return name, ops, digests, result


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("logs", nargs="+")
    ap.add_argument("--host", help="native build log: digests must match")
    a = ap.parse_args()
    ok = True
    for path in a.logs:
        name, ops, digests, result = parse(path)
        runs = max((len(v) for v in ops.values()), default=0)
        print("### %s (%s, %d runs, %s)" % (name, path, runs, result))
        if a.host:
            hdig = parse(a.host)[2]
            same = hdig == digests
            ok &= same and result == "result PASS"
            print("digests: %d, %s" % (len(digests), "match the host build" if same else "DIFFER from the host build"))
        print("| Operation | Run 0 | Median | Min | Max | Mean | CPI |")
        print("|---|---|---|---|---|---|---|")
        for op, v in ops.items():
            c = [x[1] for x in v]
            cpi = sum(x[1] for x in v) / sum(x[2] for x in v)
            print("| %s | %s | %s | %s | %s | %s | %.3f |" % (
                op, f"{c[0]:,}", f"{int(statistics.median(c)):,}", f"{min(c):,}", f"{max(c):,}",
                f"{int(statistics.fmean(c)):,}", cpi))
        print()
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
