#!/bin/bash

counter=0
max=10
while [ $counter -le $max ]
do
	((counter++))
	cp streaming streaming$counter
done
counter=0
while [ $counter -le $max ]
do
	((counter++))
	cp compute_fp compute_fp$counter
done
counter=0
while [ $counter -le $max ]
do
	((counter++))
	cp matmul_aware_norecur matmul_aware_norecur$counter
done
counter=0
while [ $counter -le $max ]
do
	((counter++))
	cp spec_qsort spec_qsort$counter
done

