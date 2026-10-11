#!/usr/bin/env python3
"""Count pairwise overlapping axis-aligned text boxes (drawn==1) from --dump-text-boxes CSV."""
import csv
import sys

def load(path):
    boxes = []
    with open(path, newline="", encoding="utf-8") as f:
        r = csv.DictReader(f)
        for row in r:
            if int(row.get("drawn", "1")) != 1:
                continue
            boxes.append(
                (
                    float(row["x"]),
                    float(row["y"]),
                    float(row["w"]),
                    float(row["h"]),
                )
            )
    return boxes

def overlaps(a, b):
    ax, ay, aw, ah = a
    bx, by, bw, bh = b
    return not (ax + aw <= bx or bx + bw <= ax or ay + ah <= by or by + bh <= ay)

def pair_count(boxes):
    n = 0
    for i in range(len(boxes)):
        for j in range(i + 1, len(boxes)):
            if overlaps(boxes[i], boxes[j]):
                n += 1
    return n

def main():
    if len(sys.argv) != 2:
        print("usage: count_text_box_pairs.py boxes.csv", file=sys.stderr)
        return 2
    boxes = load(sys.argv[1])
    print(pair_count(boxes))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
