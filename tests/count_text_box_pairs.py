#!/usr/bin/env python3
# Count pairwise overlapping axis-aligned text boxes (drawn==1) from --dump-text-boxes CSV.
#
# Usage:
#   count_text_box_pairs.py boxes.csv
#   count_text_box_pairs.py boxes.csv --class Text
#   count_text_box_pairs.py boxes.csv --class PointLabel
#
# With --class, only boxes of that class participate (PointLabel-PointLabel oracle).
# Without --class, all drawn boxes are counted (backward-compatible Text collision oracle).
import csv
import sys

def load(path, class_filter=None):
    boxes = []
    with open(path, newline="", encoding="utf-8") as f:
        r = csv.DictReader(f)
        for row in r:
            if int(row.get("drawn", "1")) != 1:
                continue
            if class_filter is not None:
                cls = row.get("class", "Text")
                if cls != class_filter:
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
    args = sys.argv[1:]
    class_filter = None
    if "--class" in args:
        i = args.index("--class")
        if i + 1 >= len(args):
            print("usage: count_text_box_pairs.py boxes.csv [--class Text|PointLabel]", file=sys.stderr)
            return 2
        class_filter = args[i + 1]
        del args[i : i + 2]
    if len(args) != 1:
        print("usage: count_text_box_pairs.py boxes.csv [--class Text|PointLabel]", file=sys.stderr)
        return 2
    boxes = load(args[0], class_filter)
    print(pair_count(boxes))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
