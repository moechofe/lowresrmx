#!/usr/bin/env python3
"""
Convert an image into Retro Game Creator characters, palettes and background data.

The fantasy console picks a palette per 8x8 cell, not per pixel, and every color must come
from the fixed 64-color master palette. So this is a constrained clustering problem: choose
up to 8 palettes of 4 colors, assign each cell to one of them, then fold the cells down into
a limited character set.

Writes a runnable .rmx cartridge (ROM entries #1 palettes, #2 characters, #3 background,
plus a tiny BASIC viewer), a PNG preview decoded back out of that data, and the same hex
blocks on stdout.

Requires Pillow -- see SETUP.md.
"""

import argparse
import math
import os
import sys
from collections import Counter

try:
	from PIL import Image
except ImportError:
	sys.exit("error: Pillow is not installed -- see tool.dev/SETUP.md")

# The 64 fixed colors of the fantasy console. Copied from better_palette[] in
# backend.core/machine/video_chip.c (FAMICUBE palette by Arne); it is hardware, it never moves.
MASTER_PALETTE = [
	(0x00, 0x00, 0x00), (0xE0, 0x3C, 0x28), (0xFF, 0xFF, 0xFF), (0xD7, 0xD7, 0xD7),
	(0xA8, 0xA8, 0xA8), (0x7B, 0x7B, 0x7B), (0x34, 0x34, 0x34), (0x15, 0x15, 0x15),
	(0x0D, 0x20, 0x30), (0x41, 0x5D, 0x66), (0x71, 0xA6, 0xA1), (0xBD, 0xFF, 0xCA),
	(0x25, 0xE2, 0xCD), (0x0A, 0x98, 0xAC), (0x00, 0x52, 0x80), (0x00, 0x60, 0x4B),
	(0x20, 0xB5, 0x62), (0x58, 0xD3, 0x32), (0x13, 0x9D, 0x08), (0x00, 0x4E, 0x00),
	(0x17, 0x28, 0x08), (0x37, 0x6D, 0x03), (0x6A, 0xB4, 0x17), (0x8C, 0xD6, 0x12),
	(0xBE, 0xEB, 0x71), (0xEE, 0xFF, 0xA9), (0xB6, 0xC1, 0x21), (0x93, 0x97, 0x17),
	(0xCC, 0x8F, 0x15), (0xFF, 0xBB, 0x31), (0xFF, 0xE7, 0x37), (0xF6, 0x8F, 0x37),
	(0xAD, 0x4E, 0x1A), (0x23, 0x17, 0x12), (0x5C, 0x3C, 0x0D), (0xAE, 0x6C, 0x37),
	(0xC5, 0x97, 0x82), (0xE2, 0xD7, 0xB5), (0x4F, 0x15, 0x07), (0x82, 0x3C, 0x3D),
	(0xDA, 0x65, 0x5E), (0xE1, 0x82, 0x89), (0xF5, 0xB7, 0x84), (0xFF, 0xE9, 0xC5),
	(0xFF, 0x82, 0xCE), (0xCF, 0x3C, 0x71), (0x87, 0x16, 0x46), (0xA3, 0x28, 0xB3),
	(0xCC, 0x69, 0xE4), (0xD5, 0x9C, 0xFC), (0xFE, 0xC9, 0xED), (0xE2, 0xC9, 0xFF),
	(0xA6, 0x75, 0xFE), (0x6A, 0x31, 0xCA), (0x5A, 0x19, 0x91), (0x21, 0x16, 0x40),
	(0x3D, 0x34, 0xA5), (0x62, 0x64, 0xDC), (0x9B, 0xA0, 0xEF), (0x98, 0xDC, 0xFF),
	(0x5B, 0xA8, 0xFF), (0x0A, 0x89, 0xFF), (0x02, 0x4A, 0xCA), (0x00, 0x17, 0x7D),
]

CHAR_SIZE = 8
PIXELS_PER_CHAR = CHAR_SIZE * CHAR_SIZE
CHARACTER_BYTES = 16
NUM_MASTER_COLORS = 64
NUM_PALETTES = 8
PALETTE_SIZE = 4
MAX_CHARACTERS = 256
MAX_CELLS = 64

# union CharacterAttributes, backend.core/machine/video_chip.h
ATTR_FLIP_X = 0x08
ATTR_FLIP_Y = 0x10

BLANK_CHARACTER = tuple([0] * PIXELS_PER_CHAR)


# --- color space ------------------------------------------------------------------------
# Nearest-color search runs in Oklab rather than raw RGB: the FAMICUBE ramp is very uneven,
# and Euclidean RGB distance picks visibly wrong hues on skin tones and dark blues.

def _srgb_to_linear(value):
	value /= 255.0
	if value <= 0.04045:
		return value / 12.92
	return ((value + 0.055) / 1.055) ** 2.4


def rgb_to_oklab(rgb):
	r = _srgb_to_linear(rgb[0])
	g = _srgb_to_linear(rgb[1])
	b = _srgb_to_linear(rgb[2])
	l = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b
	m = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b
	s = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b
	l = l ** (1.0 / 3.0)
	m = m ** (1.0 / 3.0)
	s = s ** (1.0 / 3.0)
	return (
		0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s,
		1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s,
		0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s,
	)


MASTER_LAB = [rgb_to_oklab(color) for color in MASTER_PALETTE]

# COLOR_DIST[a][b] is the squared Oklab distance between two master colors. Every palette
# cost in this script reduces to lookups in here, which is what keeps it fast in pure Python.
COLOR_DIST = []
for _a in MASTER_LAB:
	COLOR_DIST.append([
		(_a[0] - _b[0]) ** 2 + (_a[1] - _b[1]) ** 2 + (_a[2] - _b[2]) ** 2
		for _b in MASTER_LAB
	])


def nearest_master_color(rgb):
	lab = rgb_to_oklab(rgb)
	best_index = 0
	best_distance = None
	for index, other in enumerate(MASTER_LAB):
		distance = (lab[0] - other[0]) ** 2 + (lab[1] - other[1]) ** 2 + (lab[2] - other[2]) ** 2
		if best_distance is None or distance < best_distance:
			best_index = index
			best_distance = distance
	return best_index


# --- palette selection ------------------------------------------------------------------

def palette_lut(palette):
	"""For each master color, the distance to the closest color in this palette."""
	lut = []
	for color in range(NUM_MASTER_COLORS):
		row = COLOR_DIST[color]
		best = row[palette[0]]
		for entry in palette[1:]:
			value = row[entry]
			if value < best:
				best = value
		lut.append(best)
	return lut


def histogram_cost(items, lut):
	total = 0.0
	for color, count in items:
		total += count * lut[color]
	return total


def choose_palette_colors(items, slots, fixed):
	"""Pick `slots` master colors that best cover this histogram, on top of `fixed`.

	Exhaustive search is C(64, 4) = 635k combinations per palette per pass, far too slow.
	Greedy forward selection followed by swap refinement reaches the same answer in practice
	for a tiny fraction of the work.
	"""
	if not items:
		return tuple([fixed[0] if fixed else 0] * slots)

	chosen = []
	if fixed:
		best_so_far = palette_lut(fixed)
	else:
		best_so_far = [float("inf")] * NUM_MASTER_COLORS

	# Greedy: repeatedly add whichever master color reduces the total cost the most.
	while len(chosen) < slots:
		best_candidate = None
		best_cost = None
		for candidate in range(NUM_MASTER_COLORS):
			if candidate in chosen or candidate in fixed:
				continue
			row = COLOR_DIST[candidate]
			cost = 0.0
			for color, count in items:
				distance = row[color]
				current = best_so_far[color]
				cost += count * (distance if distance < current else current)
			if best_cost is None or cost < best_cost:
				best_candidate = candidate
				best_cost = cost
		if best_candidate is None:
			break
		chosen.append(best_candidate)
		row = COLOR_DIST[best_candidate]
		for color in range(NUM_MASTER_COLORS):
			if row[color] < best_so_far[color]:
				best_so_far[color] = row[color]

	while len(chosen) < slots:
		chosen.append(chosen[-1] if chosen else (fixed[0] if fixed else 0))

	# Swap refinement: try replacing each slot with every master color, keep any improvement.
	current_cost = histogram_cost(items, palette_lut(tuple(fixed) + tuple(chosen)))
	improved = True
	while improved:
		improved = False
		for slot in range(slots):
			original = chosen[slot]
			for candidate in range(NUM_MASTER_COLORS):
				if candidate == original or candidate in chosen or candidate in fixed:
					continue
				chosen[slot] = candidate
				cost = histogram_cost(items, palette_lut(tuple(fixed) + tuple(chosen)))
				if cost < current_cost - 1e-12:
					current_cost = cost
					original = candidate
					improved = True
				else:
					chosen[slot] = original
	return tuple(chosen)


def select_palettes(tile_histograms, count, slots, fixed, iterations):
	"""k-means over cells: each cluster is a palette, each tile joins its cheapest one.

	Tiles are grouped by histogram first -- an image has far fewer distinct cell palettes
	than it has cells, and the cost of a tile depends only on its histogram.
	"""
	groups = {}
	for index, histogram in enumerate(tile_histograms):
		key = tuple(sorted(histogram.items()))
		if key in groups:
			groups[key][1].append(index)
		else:
			groups[key] = (list(key), [index])
	group_list = list(groups.values())

	# Deterministic farthest-point seeding: start from the most colorful cell, then keep
	# adding the cell that the current palettes serve worst.
	candidates = []
	for items, _members in group_list:
		ranked = sorted(items, key=lambda entry: -entry[1])
		colors = [color for color, _count in ranked if color not in fixed][:slots]
		while len(colors) < slots:
			colors.append(colors[-1] if colors else (fixed[0] if fixed else 0))
		candidates.append(tuple(colors))

	seed = max(range(len(group_list)), key=lambda i: (len(group_list[i][0]), -i))
	palettes = [candidates[seed]]
	used = {candidates[seed]}
	while len(palettes) < count:
		luts = [palette_lut(tuple(fixed) + p) for p in palettes]
		worst = None
		worst_cost = None
		for index, (items, members) in enumerate(group_list):
			if candidates[index] in used:
				continue
			cost = min(histogram_cost(items, lut) for lut in luts) * len(members)
			if worst_cost is None or cost > worst_cost:
				worst = candidates[index]
				worst_cost = cost
		if worst is None:
			# Fewer distinct cell palettes than the budget; the extras stay unused.
			palettes.append(palettes[-1])
		else:
			palettes.append(worst)
			used.add(worst)

	def assign(assignment):
		"""Send every cell to its cheapest palette; returns the per-palette color totals."""
		luts = [palette_lut(tuple(fixed) + p) for p in palettes]
		buckets = [Counter() for _ in palettes]
		changed = False
		for items, members in group_list:
			best_palette = 0
			best_cost = None
			for index, lut in enumerate(luts):
				cost = histogram_cost(items, lut)
				if best_cost is None or cost < best_cost:
					best_palette = index
					best_cost = cost
			bucket = buckets[best_palette]
			for color, count in items:
				bucket[color] += count * len(members)
			for member in members:
				if assignment[member] != best_palette:
					assignment[member] = best_palette
					changed = True
		return buckets, changed

	assignment = [0] * len(tile_histograms)
	for _pass in range(iterations):
		buckets, changed = assign(assignment)
		if not changed and _pass > 0:
			break
		for index, bucket in enumerate(buckets):
			if bucket:
				palettes[index] = choose_palette_colors(list(bucket.items()), slots, fixed)

	# The loop refines palettes after assigning, so assign once more against the final ones.
	assign(assignment)
	return palettes, assignment


# --- character encoding -----------------------------------------------------------------

def encode_character(pixels):
	"""Pack an 8x8 tile of 2-bit values into the 16-byte planar layout.

	data[0..7] is the low bit-plane for rows 0..7, data[8..15] the high plane, bit 7 leftmost.
	Ground truth is video_getCharacterPixel() in backend.core/machine/video_chip.c.
	"""
	data = bytearray(CHARACTER_BYTES)
	for y in range(CHAR_SIZE):
		low = 0
		high = 0
		for x in range(CHAR_SIZE):
			value = pixels[y * CHAR_SIZE + x]
			low |= (value & 1) << (7 - x)
			high |= ((value >> 1) & 1) << (7 - x)
		data[y] = low
		data[y | 8] = high
	return bytes(data)


def decode_character(data, offset):
	pixels = []
	for y in range(CHAR_SIZE):
		low = data[offset + y]
		high = data[offset + (y | 8)]
		for x in range(CHAR_SIZE):
			shift = 7 - x
			pixels.append(((low >> shift) & 1) | (((high >> shift) & 1) << 1))
	return tuple(pixels)


def flip_pixels(pixels, flip_x, flip_y):
	result = []
	for y in range(CHAR_SIZE):
		source_y = CHAR_SIZE - 1 - y if flip_y else y
		for x in range(CHAR_SIZE):
			source_x = CHAR_SIZE - 1 - x if flip_x else x
			result.append(pixels[source_y * CHAR_SIZE + source_x])
	return tuple(result)


def tile_variants(pixels, use_flips):
	"""Every orientation of a tile, paired with the attr bits that produce it."""
	if not use_flips:
		return [(pixels, 0)]
	return [
		(pixels, 0),
		(flip_pixels(pixels, True, False), ATTR_FLIP_X),
		(flip_pixels(pixels, False, True), ATTR_FLIP_Y),
		(flip_pixels(pixels, True, True), ATTR_FLIP_X | ATTR_FLIP_Y),
	]


def palette_levels(palette):
	"""Rank a palette's four slots by Oklab lightness.

	Slot numbers themselves carry no color meaning -- choose_palette_colors hands them out in
	greedy frequency order -- so comparing tiles by slot index measures nothing. Ranking by
	lightness gives the ordering the raw indices lack.
	"""
	order = sorted(range(PALETTE_SIZE), key=lambda slot: MASTER_LAB[palette[slot]][0])
	levels = [0] * PALETTE_SIZE
	for rank, slot in enumerate(order):
		levels[slot] = rank
	return levels


def pack_thermometer(pixels, levels):
	"""Three 64-bit planes holding each pixel's lightness rank in thermometer code.

	0 -> 000, 1 -> 001, 2 -> 011, 3 -> 111. Hamming distance over this encoding is exactly the
	difference in rank, so tile distance stays three XOR popcounts while becoming monotone in
	how different the pixels actually look.
	"""
	first = 0
	second = 0
	third = 0
	for value in pixels:
		level = levels[value]
		first = (first << 1) | (level >= 1)
		second = (second << 1) | (level >= 2)
		third = (third << 1) | (level >= 3)
	return first, second, third


def thermometer_distance(a, b):
	return ((a[0] ^ b[0]).bit_count() + (a[1] ^ b[1]).bit_count() + (a[2] ^ b[2]).bit_count())


# --- ROM entries ------------------------------------------------------------------------

def rom_entry(index, comment, data):
	"""Match data_export() in backend.core/datamanager/data_manager.c exactly: uppercase
	hex, 16 bytes per line, blank line after the entry."""
	lines = ["#%d:%s" % (index, comment)]
	for start in range(0, len(data), 16):
		lines.append("".join("%02X" % byte for byte in data[start:start + 16]))
	return "\n".join(lines) + "\n\n"


# --- pipeline ---------------------------------------------------------------------------

def resolve_cell_size(image, width, height):
	if width is None and height is None:
		width = max(1, -(-image.width // CHAR_SIZE))
		height = max(1, -(-image.height // CHAR_SIZE))
	elif width is None:
		width = max(1, round(height * CHAR_SIZE * image.width / image.height / CHAR_SIZE))
	elif height is None:
		height = max(1, round(width * CHAR_SIZE * image.height / image.width / CHAR_SIZE))

	clipped = False
	if width > MAX_CELLS:
		width = MAX_CELLS
		clipped = True
	if height > MAX_CELLS:
		height = MAX_CELLS
		clipped = True
	if clipped:
		print("warning: clipped to the %dx%d cell background limit" % (MAX_CELLS, MAX_CELLS),
			file=sys.stderr)
	return width, height


def snap_image(image):
	"""Every pixel to its nearest master color. Memoized -- after a nearest-neighbour resize
	the distinct-color count is small, and the Oklab search is the expensive part."""
	cache = {}
	indices = []
	raw = image.tobytes()
	for offset in range(0, len(raw), 3):
		pixel = raw[offset:offset + 3]
		index = cache.get(pixel)
		if index is None:
			index = nearest_master_color(pixel)
			cache[pixel] = index
		indices.append(index)
	return indices


def build_tiles(indices, cells_x, cells_y):
	width = cells_x * CHAR_SIZE
	tiles = []
	for cell_y in range(cells_y):
		for cell_x in range(cells_x):
			pixels = []
			base = cell_y * CHAR_SIZE * width + cell_x * CHAR_SIZE
			for y in range(CHAR_SIZE):
				row = base + y * width
				pixels.extend(indices[row:row + CHAR_SIZE])
			tiles.append(pixels)
	return tiles


def quantize_tiles(tiles, palettes, assignment):
	"""Turn each tile into 8x8 slot indices (0..3) using its assigned palette."""
	luts = []
	for palette in palettes:
		lut = []
		for color in range(NUM_MASTER_COLORS):
			row = COLOR_DIST[color]
			best_slot = 0
			best = row[palette[0]]
			for slot in range(1, PALETTE_SIZE):
				value = row[palette[slot]]
				if value < best:
					best_slot = slot
					best = value
			lut.append(best_slot)
		luts.append(lut)

	result = []
	for index, tile in enumerate(tiles):
		lut = luts[assignment[index]]
		result.append(tuple(lut[color] for color in tile))
	return result


def dedupe_characters(quantized, use_flips):
	"""Fold tiles onto a shared character set, matching flipped orientations too.

	Character 0 is reserved blank, per the repo convention that every #2 entry starts with
	an all-zero character.
	"""
	characters = [BLANK_CHARACTER]
	lookup = {BLANK_CHARACTER: 0}
	cells = []
	for pixels in quantized:
		found = None
		for variant, flip in tile_variants(pixels, use_flips):
			index = lookup.get(variant)
			if index is not None:
				found = (index, flip)
				break
		if found is None:
			# Store the smallest orientation so equivalent tiles always agree on a canonical
			# form; the flip bits that turn it back into this tile go in the cell.
			variants = tile_variants(pixels, use_flips)
			canonical, flip = min(variants, key=lambda entry: entry[0])
			index = len(characters)
			characters.append(canonical)
			lookup[canonical] = index
			found = (index, flip)
		cells.append(found)
	return characters, cells


def merge_characters(characters, cells, budget, use_flips, palettes, assignment, passes):
	"""Cluster the characters down to `budget` with k-means.

	Picking the most-used tiles as anchors and snapping the rest onto them looks reasonable but
	biases the result spatially: on a photo most characters are used exactly once, so the ranking
	falls through to a tie-break on first-seen order and the top of the image is kept verbatim
	while the bottom is merged away. Clustering removes that -- every character can move, and a
	cluster's representative is recomputed as the compromise that costs its members least, so a
	character already in use degrades a little in order to serve several cells.

	Flips compose by XOR: if a member matched orientation `h` of its cluster, a cell that reached
	that member through flip `g` reaches the representative through `g ^ h`.
	"""
	if len(characters) <= budget:
		return characters, cells, 0, 0.0

	usage = Counter(index for index, _flip in cells)
	# Each character is compared under the palette it is mostly drawn with.
	palette_of = {}
	for index, (character, _flip) in enumerate(cells):
		palette_of.setdefault(character, Counter())[assignment[index]] += 1
	palette_of = {c: p.most_common(1)[0][0] for c, p in palette_of.items()}

	levels = [palette_levels(palette) for palette in palettes]
	# Cost of drawing slot `a` as slot `b`, per palette, in Oklab distance.
	slot_cost = [
		[[math.sqrt(COLOR_DIST[palette[a]][palette[b]]) for b in range(PALETTE_SIZE)]
			for a in range(PALETTE_SIZE)]
		for palette in palettes
	]

	# Seed with the most-used characters, tie-broken deterministically but without positional
	# meaning, so the starting set is not itself biased toward the top of the image.
	ranked = sorted(range(1, len(characters)),
		key=lambda i: (-usage.get(i, 0), hash(characters[i])))
	centroids = [BLANK_CHARACTER] + [characters[i] for i in ranked[:budget - 1]]

	packed = [pack_thermometer(characters[i], levels[palette_of.get(i, 0)])
		for i in range(len(characters))]

	mapping = None
	for _pass in range(passes):
		# Pack every centroid orientation once per palette, not once per comparison -- there are
		# only 8 palettes, and doing this inside the loop below costs 25x the total runtime.
		centroid_packs = [
			[[(pack_thermometer(variant, level), flip)
				for variant, flip in tile_variants(centroid, use_flips)]
				for level in levels]
			for centroid in centroids
		]
		members = [[] for _ in centroids]
		mapping = {0: (0, 0)}
		for index in range(1, len(characters)):
			source = packed[index]
			palette = palette_of.get(index, 0)
			best = None
			for cluster in range(len(centroids)):
				for candidate, flip in centroid_packs[cluster][palette]:
					distance = thermometer_distance(source, candidate)
					if best is None or distance < best[0]:
						best = (distance, cluster, flip)
			mapping[index] = (best[1], best[2])
			members[best[1]].append((index, best[2]))

		for cluster in range(1, len(centroids)):
			if not members[cluster]:
				continue
			# Per pixel, the slot whose total weighted color error across the members is lowest.
			totals = [[0.0] * PALETTE_SIZE for _ in range(PIXELS_PER_CHAR)]
			for index, flip in members[cluster]:
				pixels = flip_pixels(characters[index], flip & ATTR_FLIP_X, flip & ATTR_FLIP_Y)
				weight = usage.get(index, 1)
				costs = slot_cost[palette_of.get(index, 0)]
				for position, value in enumerate(pixels):
					row = costs[value]
					total = totals[position]
					for slot in range(PALETTE_SIZE):
						total[slot] += weight * row[slot]
			centroids[cluster] = tuple(
				min(range(PALETTE_SIZE), key=lambda slot: total[slot]) for total in totals)

	new_cells = [(mapping[index][0], flip ^ mapping[index][1]) for index, flip in cells]

	# Mean Oklab distance per pixel between what each cell asked for and what it now draws.
	error = 0.0
	for index, (character, flip) in enumerate(new_cells):
		costs = slot_cost[assignment[index]]
		old_character, old_flip = cells[index]
		wanted = flip_pixels(characters[old_character], old_flip & ATTR_FLIP_X, old_flip & ATTR_FLIP_Y)
		drawn = flip_pixels(centroids[character], flip & ATTR_FLIP_X, flip & ATTR_FLIP_Y)
		for a, b in zip(wanted, drawn):
			if a != b:
				error += costs[a][b]
	mean_error = error / (len(new_cells) * PIXELS_PER_CHAR) if new_cells else 0.0
	return centroids, new_cells, len(characters) - budget, mean_error


def render_preview(palette_bytes, character_bytes, cells, cells_x, cells_y):
	"""Decode the emitted bytes back into an image, so the preview doubles as a check that
	the encoder and the console's own decoder agree."""
	image = Image.new("RGB", (cells_x * CHAR_SIZE, cells_y * CHAR_SIZE))
	pixels = image.load()
	decoded = [
		decode_character(character_bytes, offset)
		for offset in range(0, len(character_bytes), CHARACTER_BYTES)
	]
	for cell_y in range(cells_y):
		for cell_x in range(cells_x):
			character, attr = cells[cell_y * cells_x + cell_x]
			tile = flip_pixels(decoded[character], attr & ATTR_FLIP_X, attr & ATTR_FLIP_Y)
			palette = attr & 0x07
			for y in range(CHAR_SIZE):
				for x in range(CHAR_SIZE):
					slot = tile[y * CHAR_SIZE + x]
					color = palette_bytes[palette * PALETTE_SIZE + slot]
					pixels[cell_x * CHAR_SIZE + x, cell_y * CHAR_SIZE + y] = MASTER_PALETTE[color]
	return image


def build_program(cells_x, cells_y, free_color0):
	lines = []
	if free_color0:
		# Layer 0 shows color 0 as the backdrop unless this is on -- manual.md, SYSTEM setting 1.
		lines.append("system 1,1")
	lines.append("bg source rom(3)")
	lines.append("bg copy 0,0,%d,%d to 0,0" % (cells_x, cells_y))
	lines.append("do")
	lines.append("  wait vbl")
	lines.append("loop")
	return "\n".join(lines) + "\n\n"


def main():
	parser = argparse.ArgumentParser(
		description="Convert an image into Retro Game Creator characters, palettes and background.")
	parser.add_argument("input", help="source image (any format Pillow reads)")
	parser.add_argument("-o", "--output", help="output .rmx (default: alongside the input)")
	parser.add_argument("--width", type=int, help="target width in cells (1..%d)" % MAX_CELLS)
	parser.add_argument("--height", type=int, help="target height in cells (1..%d)" % MAX_CELLS)
	parser.add_argument("-c", "--characters", type=int, default=MAX_CHARACTERS,
		help="character budget including the reserved blank (1..%d, default %d)"
			% (MAX_CHARACTERS, MAX_CHARACTERS))
	parser.add_argument("-p", "--palettes", type=int, default=NUM_PALETTES,
		help="palette budget (1..%d, default %d)" % (NUM_PALETTES, NUM_PALETTES))
	parser.add_argument("--backdrop", type=int,
		help="master color 0..63 for slot 0 (default: the image's most common color)")
	parser.add_argument("--free-color0", action="store_true",
		help="let each palette use all 4 slots; the program then needs SYSTEM 1,1")
	parser.add_argument("--no-flip", action="store_true",
		help="do not match flipped tiles when deduplicating")
	parser.add_argument("--no-preview", action="store_true", help="skip the PNG preview")
	parser.add_argument("--iterations", type=int, default=8,
		help="palette clustering passes (default 8)")
	parser.add_argument("--merge-passes", type=int, default=4,
		help="character clustering passes, used only when over budget (default 4)")
	args = parser.parse_args()

	if not 1 <= args.characters <= MAX_CHARACTERS:
		parser.error("--characters must be between 1 and %d" % MAX_CHARACTERS)
	if not 1 <= args.palettes <= NUM_PALETTES:
		parser.error("--palettes must be between 1 and %d" % NUM_PALETTES)
	if args.backdrop is not None and not 0 <= args.backdrop < NUM_MASTER_COLORS:
		parser.error("--backdrop must be between 0 and %d" % (NUM_MASTER_COLORS - 1))
	for name in ("width", "height"):
		value = getattr(args, name)
		if value is not None and value < 1:
			parser.error("--%s must be at least 1" % name)

	output_path = args.output or os.path.splitext(args.input)[0] + ".rmx"

	image = Image.open(args.input).convert("RGB")
	cells_x, cells_y = resolve_cell_size(image, args.width, args.height)
	image = image.resize((cells_x * CHAR_SIZE, cells_y * CHAR_SIZE), Image.NEAREST)

	indices = snap_image(image)
	backdrop = args.backdrop if args.backdrop is not None else Counter(indices).most_common(1)[0][0]
	fixed = () if args.free_color0 else (backdrop,)
	slots = PALETTE_SIZE - len(fixed)

	tiles = build_tiles(indices, cells_x, cells_y)
	histograms = [Counter(tile) for tile in tiles]
	free_colors, assignment = select_palettes(
		histograms, args.palettes, slots, fixed, max(1, args.iterations))
	palettes = [tuple(fixed) + colors for colors in free_colors]

	quantized = quantize_tiles(tiles, palettes, assignment)
	characters, cells = dedupe_characters(quantized, not args.no_flip)
	unique = len(characters)
	characters, cells, merged, mean_error = merge_characters(
		characters, cells, args.characters, not args.no_flip, palettes, assignment,
		max(1, args.merge_passes))

	cells = [
		(character, flip | assignment[index])
		for index, (character, flip) in enumerate(cells)
	]

	# #1 is clamped to 32 bytes at startup, so always emit exactly 32 -- a short entry would
	# leave the remaining palettes holding whatever was in the registers before.
	palette_bytes = bytearray(NUM_PALETTES * PALETTE_SIZE)
	for index in range(NUM_PALETTES):
		source = palettes[index] if index < len(palettes) else palettes[0]
		for slot in range(PALETTE_SIZE):
			palette_bytes[index * PALETTE_SIZE + slot] = source[slot]

	# #2 is NOT clamped -- startup_sequence.c memcpy()s entries[2].length bytes straight into
	# videoRam.characters, so anything past 4096 corrupts the rest of struct Machine.
	character_bytes = bytearray()
	for pixels in characters:
		character_bytes.extend(encode_character(pixels))
	assert len(character_bytes) <= MAX_CHARACTERS * CHARACTER_BYTES

	background_bytes = bytearray([0, 0, cells_x, cells_y])
	for character, attr in cells:
		background_bytes.append(character)
		background_bytes.append(attr)

	entries = (
		rom_entry(1, "MAIN PALETTES", palette_bytes)
		+ rom_entry(2, "MAIN CHARACTERS", character_bytes)
		+ rom_entry(3, "MAIN BG", background_bytes)
	)
	with open(output_path, "w") as handle:
		handle.write(build_program(cells_x, cells_y, args.free_color0))
		handle.write(entries)

	preview_path = None
	if not args.no_preview:
		preview_path = os.path.splitext(output_path)[0] + ".preview.png"
		render_preview(palette_bytes, character_bytes, cells, cells_x, cells_y).save(preview_path)

	sys.stdout.write(entries)
	print("%s: %dx%d cells (%dx%d pixels)"
		% (args.input, cells_x, cells_y, cells_x * CHAR_SIZE, cells_y * CHAR_SIZE),
		file=sys.stderr)
	print("palettes: %d, backdrop color %d%s"
		% (len(palettes), backdrop, " (slot 0 free)" if args.free_color0 else ""),
		file=sys.stderr)
	if merged:
		print("characters: %d unique -> %d, merged %d, mean error %.4f per pixel"
			% (unique, len(characters), merged, mean_error), file=sys.stderr)
	else:
		print("characters: %d of %d" % (len(characters), args.characters), file=sys.stderr)
	print("wrote %s%s" % (output_path, " and " + preview_path if preview_path else ""),
		file=sys.stderr)


if __name__ == "__main__":
	main()
