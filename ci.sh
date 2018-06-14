projects=(
  week_02/matrix_multiplication
  week_03/heat_stencil
  week_04/matrix_multiplication_profiling
  week_05/heat_stencil
  week_06/recursive_multistage_reduction
  week_07/auto_level
  week_08/count_sort
  week_10/matrix_mul_bench
  week_11/dynamic_programming
)

exit_status=0

for p in "${projects[@]}"; do
  fold_id=$(echo "${p}" | tr '/' _)
  timer_id=$RANDOM

  echo "travis_fold:start:${fold_id}"
  echo "travis_time:start:${timer_id}"

  start_time="$(date +%s)"

  mkdir -p "$(dirname "/tmp/${p}")"

  if [ "$TRAVIS_OS_NAME" = 'linux' ]; then
    make -C "${p}"
  else
    make run -C "${p}"
  fi &>"/tmp/${p}.log"

  if [ $? -eq 0 ]; then
    printf '\033[32m'
  else
    printf '\033[31m'
    exit_status=1
  fi

  echo "${p}"
  printf '\033[0m'

  cat "/tmp/${p}.log"
  rm "/tmp/${p}.log"

  end_time="$(date +%s)"
  duration=$((${end_time} - ${start_time}))

  echo "travis_time:end:${timer_id},start=${start_time},finish=${end_time},duration=${duration}"
  echo "travis_fold:end:${fold_id}"
done

exit $exit_status
