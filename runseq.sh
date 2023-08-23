for c in $(seq 3 $1)
do
  for r in $(seq 3 $2)
  do
    echo "Runing "$c"x"$r
    if [[ -f "banned/c"$c"r"$r ]] 
    then
      continue
    fi
    ./obj/a.out $c $r
    cat "./raw/Nc"$c"r"$r* > "./banned/c"$c"r"$r
    sort -o "./banned/c"$c"r"$r{,}
  done
done
