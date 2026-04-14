set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../../../.." && pwd)"

#TEST="$SCRIPT_DIR/reduce_sum.mlir"
TEST="$SCRIPT_DIR/reduce_max.mlir"
OUTDIR="/tmp/tests"

TOYC="${TOYC:-$REPO_ROOT/build/bin/toyc-ch7}"
SPLIT_FILE="${SPLIT_FILE:-$(command -v split-file-20)}"
FILECHECK="${FILECHECK:-$(command -v FileCheck-20)}"

rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

"$SPLIT_FILE" "$TEST" "$OUTDIR"

# file -> check-prefix -> expect_fail(0/1)
cases=(
  "reduction_performed.mlir:REDUCE:0"
  "rank_zero.mlir:RANK-ZERO:1"
  "axis_oob.mlir:AXIS-OOB:1"
  "axis_negative.mlir:AXIS-NEGATIVE:1"
  "result_shape.mlir:RESULT-SHAPE:1"
)
toyc=/root/workspace/llvm-project/bin/toyc-ch7
for c in "${cases[@]}"; do
  IFS=: read -r file prefix expect_fail <<< "$c"
  input="$OUTDIR/$file"

  echo "==> $file ($prefix, expect_fail=$expect_fail)"
  if [[ "$expect_fail" == "1" ]]; then
    { "$TOYC" -x=mlir -emit=mlir "$input" 2>&1 || true; } \
      | "$FILECHECK" "$TEST" --check-prefix="$prefix"
  else
    "$TOYC" -x=mlir -emit=mlir -opt "$input" 2>&1 \
      | "$FILECHECK" "$TEST" --check-prefix="$prefix"
  fi
done

echo "All split cases ran successfully."