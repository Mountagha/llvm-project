set -euo pipefail

TEST="reduce_sum.mlir"
OUTDIR="/tmp/tests"

rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

split-file-20 "$TEST" "$OUTDIR"

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
    $toyc -x=mlir -emit=mlir "$input" 2>&1 \
      | FileCheck-20 "$TEST" --check-prefix="$prefix"
  else
    $toyc -x=mlir -emit=mlir -opt "$input" 2>&1 \
      | FileCheck-20 "$TEST" --check-prefix="$prefix"
  fi
done

echo "All split cases ran successfully."