main:
    addi t0, x0, 10
    addi t1, x0, 20
    add t2, t0, t1

    addi a0, x0, 0
    addi a7, x0, 10
    ecall
