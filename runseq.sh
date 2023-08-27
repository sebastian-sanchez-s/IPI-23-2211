mkdir -p banned
mkdir -p feasible

rm -f banned/* feasible/*

for c in $(seq 3 $1)
do
  for r in $(seq 3 $2)
  do
    make run "NCOL="$c "NROW="$r >/dev/null
  done
done
