mkdir -p banned
mkdir -p feasible
for c in $(seq 3 $1)
do
  for r in $(seq 3 $2)
  do
    echo "Running "$c"x"$r
    if [[ -f "banned/c"$c"r"$r ]] 
    then
      continue
    fi
    make run "NCOL="$c "NROW="$r
  done
done
