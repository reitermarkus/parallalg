#!/bin/bash

printf "" > measure.txt

args=(100 200 500 1000 2000)

function write_into () {
    printf $1 >> $2
}

function measure () {
    write_into "$1:\n" measure.txt
    for i in "${args[@]}"
    do
        eval $2 $i >> measure.txt
        write_into "\n" measure.txt
    done
    write_into "\n\n" measure.txt
}

measure "Sequential" "./mat_mul_seq"
measure "OpenMP" "./mat_mul_omp"
measure "OpenCL" "./mat_mul_ocl"
