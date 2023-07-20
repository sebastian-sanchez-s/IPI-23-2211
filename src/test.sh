# This scripts test the outputs
# of sytX programs 

EO=syt0
TO=syt1
CASES="3x3 3x4 3x5 4x4"

echo "Validation Tests."
echo "Testing $TO using $EO as reference."
echo -n "Delete raw data? (y/n): "
read -r deleteraw

for c in $CASES
do
    echo -n "Case $c" 

    m=$(echo $c | cut -d"x" -f1)
    n=$(echo $c | cut -d"x" -f2)

    # Run syt0
    cd $EO
    make M=$m N=$n > /dev/null
    sort -o raw/$m"_"$n{,}
    cd ..
    
    # Run TO 
    cd $TO
    make M=$m N=$n > /dev/null
    cat raw/m$m"_n"$n"_s"* | sort > raw/m$m"_n"$n
    rm raw/m$m"_n"$n"_s"*
    cd ..

    # Compare
    if [[ -z $(diff -w -q $EO/raw/$m"_"$n $TO/raw/m$m"_n"$n) ]]
    then
        echo " [PASSED]."
    else
        echo " [FAILED]."
    fi

    if [[ $deleteraw == "y" ]]
    then
        rm $EO/raw/$m"_"$n
        rm $TO/raw/m$m"_n"$n
    fi
done
