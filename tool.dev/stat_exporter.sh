#!/usr/bin/env bash
#
# stat_exporter.sh — export LowResRMX activity counters to Prometheus.
#
# Reads the lifetime "stat:EVENT" counters from Redis (see project.web/DATABASE.md)
# and writes them as a Prometheus textfile that node_exporter serves through its
# textfile collector.
#
# The per-day "stat:EVENT:YYYY-MM-DD" buckets are skipped on purpose: Prometheus
# keeps its own history, so increase(lowresrmx_stat_total[1d]) in Grafana already
# reconstructs daily/hourly activity from the cumulative counter — exporting the
# dated keys would just add ~26 events x 400 days of high-cardinality duplication.
#
# Output (one metric family, one label so event names may contain ':'):
#   # HELP lowresrmx_stat_total Lifetime total count for a tracked activity event.
#   # TYPE lowresrmx_stat_total counter
#   lowresrmx_stat_total{event="view:community"} 1234
#   lowresrmx_stat_total{event="vote"} 56
#
# Meant to run from a systemd timer (or cron). Install notes at the bottom.

set -euo pipefail

REDIS_CLI="${REDIS_CLI:-redis-cli}"
REDIS_HOST="${REDIS_HOST:-127.0.0.1}"
REDIS_PORT="${REDIS_PORT:-6379}"
TEXTFILE_DIR="${TEXTFILE_DIR:-/var/lib/node_exporter/textfile_collector}"
METRIC="lowresrmx_stat_total"
OUTFILE="${TEXTFILE_DIR}/lowresrmx_stat.prom"

redis() { "$REDIS_CLI" -h "$REDIS_HOST" -p "$REDIS_PORT" "$@"; }

# Lifetime counter keys only — drop the dated per-day buckets (":YYYY-MM-DD").
mapfile -t keys < <(redis --scan --pattern 'stat:*' \
	| grep -Ev ':[0-9]{4}-[0-9]{2}-[0-9]{2}$' \
	| LC_ALL=C sort)

tmp="$(mktemp "${OUTFILE}.XXXXXX")"
trap 'rm -f "$tmp"' EXIT

{
	echo "# HELP ${METRIC} Lifetime total count for a tracked activity event (see DATABASE.md)."
	echo "# TYPE ${METRIC} counter"
	if ((${#keys[@]})); then
		mapfile -t vals < <(redis MGET "${keys[@]}")
		for i in "${!keys[@]}"; do
			val="${vals[i]:-}"
			[[ "$val" =~ ^-?[0-9]+$ ]] || continue           # skip nil / non-integer
			event="${keys[i]#stat:}"                          # strip "stat:" prefix
			event="${event//\\/\\\\}"; event="${event//\"/\\\"}"  # escape label value
			printf '%s{event="%s"} %s\n' "$METRIC" "$event" "$val"
		done
	fi
} > "$tmp"

# Atomic replace so node_exporter never reads a half-written file.
mv -f "$tmp" "$OUTFILE"
trap - EXIT
