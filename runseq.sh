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
    ./obj/a.out $c $r
    echo "Generating banned"
    cat "./raw/Nc"$c"r"$r* > "./banned/c"$c"r"$r
    echo "Generating feasible"
    cat "./raw/Pc"$c"r"$r* > "./feasible/c"$c"r"$r
  done
done
