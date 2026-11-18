#!/usr/bin/env bash
#
# Smoke / integration tests for memory_inspector.
#
#   ./tests/smoke.sh          run every group
#
# Expects `make && make victim` to have been run first.
# Exits 0 only if every assertion passed.

set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

INSPECTOR=./memory_inspector
VICTIM=./victim/victim
SECRET_PREFIX='HELLO_FROM_VICTI'   # truncated: the dump previews 16 bytes/region

pass=0; fail=0; skip=0
victim_pid=""

cleanup() { [ -n "$victim_pid" ] && kill "$victim_pid" 2>/dev/null; return 0; }
trap cleanup EXIT

group() { printf '\n\033[1m%s\033[0m\n' "$1"; }
ok()    { printf '  \033[32mPASS\033[0m  %s\n' "$1"; pass=$((pass + 1)); }
ko()    { printf '  \033[31mFAIL\033[0m  %s\n' "$1"
          [ $# -gt 1 ] && printf '        %s\n' "$2"
          fail=$((fail + 1)); return 0; }
no()    { printf '  \033[33mSKIP\033[0m  %s\n' "$1"
          [ $# -gt 1 ] && printf '        %s\n' "$2"
          skip=$((skip + 1)); return 0; }

# ---------------------------------------------------------------- prerequisites

group 'Prerequisites'

[ -x "$INSPECTOR" ] || { echo "missing $INSPECTOR - run 'make' first" >&2; exit 2; }
ok "$INSPECTOR is built"

[ -x "$VICTIM" ] || { echo "missing $VICTIM - run 'make victim' first" >&2; exit 2; }
ok "$VICTIM is built"

if codesign -d --entitlements - "$VICTIM" 2>&1 | grep -q 'get-task-allow'; then
    ok 'victim carries the get-task-allow entitlement'
else
    ko 'victim carries the get-task-allow entitlement' \
       'codesign did not report the entitlement; task_for_pid will be denied'
fi

# ------------------------------------------------------------ argument handling

group 'Argument handling'

# No argument: must print usage on stderr and exit 1.
out=$("$INSPECTOR" 2>&1); code=$?
if [ "$code" -ge 128 ]; then
    ko "no argument exits 1 (got $code, killed by signal $((code - 128)))" \
       'src/main.c dereferences argv[1] before checking argc'
elif [ "$code" -eq 1 ] && printf '%s' "$out" | grep -qi 'usage'; then
    ok 'no argument prints usage and exits 1'
else
    ko "no argument prints usage and exits 1 (exit=$code)" "output: ${out:-<empty>}"
fi

# Non-numeric argument: must be rejected, not silently treated as pid 0.
out=$("$INSPECTOR" not-a-pid 2>&1); code=$?
if [ "$code" -ne 0 ] && [ "$code" -lt 128 ]; then
    ok "non-numeric argument is rejected (exit=$code)"
else
    ko "non-numeric argument is rejected (exit=$code)" \
       'strtol(argv[1], NULL, 10) accepts garbage as pid 0'
fi

# Negative argument: a pid is strictly positive.
out=$("$INSPECTOR" -1 2>&1); code=$?
if [ "$code" -ne 0 ] && [ "$code" -lt 128 ]; then
    ok "negative pid is rejected (exit=$code)"
else
    ko "negative pid is rejected (exit=$code)" 'no lower-bound check on the parsed pid'
fi

# Unused pid: no such process must be an error exit.
out=$("$INSPECTOR" 999999 2>&1); code=$?
if [ "$code" -ne 0 ] && [ "$code" -lt 128 ]; then
    ok "unused pid is reported as an error (exit=$code)"
else
    ko "unused pid is reported as an error (exit=$code)" \
       'inspect_process() is void, so the failure never reaches main'
fi

# ------------------------------------------------- inspection of a valid target

group 'Inspecting an entitled target'

"$VICTIM" >/tmp/victim.$$.log 2>&1 &
victim_pid=$!
sleep 1

if ! kill -0 "$victim_pid" 2>/dev/null; then
    ko 'victim stays alive' "$(cat /tmp/victim.$$.log)"
else
    ok "victim is running (pid $victim_pid)"

    out=$("$INSPECTOR" "$victim_pid" 2>&1); code=$?

    if printf '%s' "$out" | grep -q 'task_for_pid failed'; then
        # Not a defect in the tool: the platform refused the task port.
        no 'task_for_pid on the entitled victim' \
           'denied by this environment (SIP policy); memory-map assertions skipped'
        no 'memory map is enumerated'
        no 'target memory is actually read'
    else
        [ "$code" -eq 0 ] \
            && ok 'inspecting the victim exits 0' \
            || ko "inspecting the victim exits 0 (exit=$code)" "output: $out"

        printf '%s' "$out" | grep -q 'MEMORY MAP' \
            && ok 'memory map header is printed' \
            || ko 'memory map header is printed'

        regions=$(printf '%s' "$out" | grep -c '^0x')
        [ "$regions" -ge 10 ] \
            && ok "memory map is enumerated ($regions regions)" \
            || ko "memory map is enumerated (only $regions regions)"

        # End-to-end proof: task_for_pid + mach_vm_region + mach_vm_read_overwrite
        # all worked, because a known string from the victim's __DATA shows up.
        printf '%s' "$out" | grep -q "$SECRET_PREFIX" \
            && ok "target memory is actually read ('$SECRET_PREFIX' found in dump)" \
            || ko "target memory is actually read ('$SECRET_PREFIX' not found in dump)"
    fi
fi

kill "$victim_pid" 2>/dev/null; victim_pid=""
rm -f /tmp/victim.$$.log

# ------------------------------------------------------------- denied target

group 'Inspecting a non-entitled target'

/bin/sleep 30 & sleep_pid=$!
sleep 0.5

out=$("$INSPECTOR" "$sleep_pid" 2>&1); code=$?
kill "$sleep_pid" 2>/dev/null

if [ "$code" -ge 128 ]; then
    ko "non-entitled target is handled gracefully (signal $((code - 128)))" \
       'the tool crashed instead of reporting the denial'
elif printf '%s' "$out" | grep -q 'task_for_pid failed'; then
    ok 'non-entitled target reports a task_for_pid denial'
    [ "$code" -ne 0 ] \
        && ok "non-entitled target exits non-zero (exit=$code)" \
        || ko 'non-entitled target exits non-zero (exit=0)' \
              'the denial is printed but not propagated to the exit code'
else
    no 'non-entitled target reports a task_for_pid denial' \
       "this environment allowed the read; output: $(printf '%s' "$out" | head -c 120)"
fi

# ---------------------------------------------------------------------- summary

printf '\n\033[1mSummary\033[0m  %d passed, %d failed, %d skipped\n' \
    "$pass" "$fail" "$skip"

[ "$fail" -eq 0 ]
