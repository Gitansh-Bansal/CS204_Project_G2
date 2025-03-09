.data
array: .byte -12 40 2 13 73 29 12 88 15 7

.text 
lui x5, 0x10000     # x5 has the starting address
lui x6, 0x10000     # (to find) end address in x6
lb x7, 0, x6        # x7 contains whatever at x6
addi x8, x0, 0      # x8 will contain the length of the array

L1: beq x7, x0, E1
addi x8, x8, 1
addi x6, x6, 1
lb x7, 0, x6
jal x0, L1

E1: addi x8, x8, -1
# now x8 contains the length of the array

addi x6, x0, 0
addi x7, x0, 0

add x30, x0, x8     # outer loop counter
add x31, x0, x8     # inner loop counter


L2: beq x30, x0, exit 
add x31, x0, x8
lui x6, 0x10000
L3: beq x31, x0, E2
lb x10, 0, x6
lb x11, 1, x6
blt x11, x10, E3
R1: addi x6, x6, 1
addi x31, x31, -1
jal x0, L3

E2: addi x30, x0, -1
jal x0, L2

E3: addi x28, x10, 0
addi x29, x11, 0
sb x28, 1, x6
sb x29, 0, x6
jal x0, R1

exit:

