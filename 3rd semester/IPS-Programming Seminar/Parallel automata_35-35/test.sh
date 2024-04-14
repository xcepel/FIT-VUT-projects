#!/bin/bash

RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
NC=$(tput sgr0)

while read -r line; do
    # Reading each line
    ARGS=${line#./aut}
    tested="./aut""$ARGS"
    ref="./aut-ref""$ARGS"
    TEST_OUT=$(eval $tested)
    REF_OUT=$(eval $ref)

    if [[ "$TEST_OUT" == "$REF_OUT" ]]; then
        RES="${GREEN}Passed$NC"
    else
        RES="${RED}Failed$NC"
    fi
    printf '%-80s %s' "$line..." "$RES"
    printf '\n'
    if [[ "$RES" == "${RED}Failed$NC" ]]; then
        echo "Tested output:"
        echo "$TEST_OUT"
        echo "Reference output:"
        echo "$REF_OUT"
    fi
done < tests-used.txt



    
