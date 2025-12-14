make
CURRENT_TIME=$(date +%Y%m%d_%H%M%S)
PREFIX="EDC"
out_file="${PREFIX}_${CURRENT_TIME}.out"
for i in {1..5}
do
    echo "Run #$i" >> $out_file
    ./EDC $(($i*20)) >> $out_file
    echo "" >> $out_file
done