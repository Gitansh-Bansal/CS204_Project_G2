.data
input : .word 0
len : .word 10
array : .word -7 0 81 36 54 94 46 4 1 -1

.text
lui x5, 0x10000
lw x6, 0(x5)    # input (0/1)
lw x7, 4(x5)    # length of the array
addi x5, x5, 8  # now x5 contains the starting address of the array

lw x8, 0(x5)    # x8 : first array element

lui x15, 0x10000
addi x15, x15, 0x500    # now x15 contains the destination address


addi x16, x5, 0
addi x17, x15, 0
addi x31, x7, 0

# create a copy of array at 0x10000500
copyloop : beq x31, x0, ec
lw x30, 0(x16)
sw x30, 0(x17)
addi x16, x16, 4
addi x17, x17, 4
addi x31, x31, -1
jal x0, copyloop
ec:

beq x6, x0, unoptimised
jal x0, optimised

########################################################################

unoptimised : 
addi x31, x7, -1     # outer iterator

loop1 :
beq x31, x0, loop1end
addi x9, x15, 0       # 0 index
lw x10, 0(x9)
lw x11, 4(x9)
addi x30, x7, -1    # inner iterator
    loop2 : 
    beq x30, x0, loop2exit
    bge x11, x10, e1
    sw x10, 4(x9)
    sw x11, 0(x9)
    e1 : 
    addi x30, x30, -1
    addi x9, x9, 4
    lw x10, 0(x9)
    lw x11, 4(x9)
    jal x0, loop2
    loop2exit : 
addi x31, x31, -1

jal x0, loop1

loop1end : 
jal x0, loop3end    # after running unoptimized code, go to end

############################################################################

optimised : 
addi x31, x7, -1     # outer iterator
addi x22, x0, 0

loop3 :
beq x31, x0, loop3end
addi x9, x15, 0       # 0 index
lw x10, 0(x9)
lw x11, 4(x9)
addi x30, x7, -1    # inner iterator
sub x30, x30, x22   # ,,
addi x26, x0, 0     # x26 is the flag, set to zero
    loop4 : 
    beq x30, x0, loop4exit
    bge x11, x10, e2
    sw x10, 4(x9)           # swap
    sw x11, 0(x9)           # swap
    addi x26, x0, 1         # set flag to 1
    e2 : 
    addi x30, x30, -1
    addi x9, x9, 4
    lw x10, 0(x9)
    lw x11, 4(x9)
    jal x0, loop4
    loop4exit : 
beq x26, x0, loop3end   # if flag is still 0, exit loop3 (outer)
addi x31, x31, -1
addi x22, x22, 1
jal x0, loop3

loop3end : 

#### dynamic instruction counts (cycle count) for 5 samples : 

# example 1 : (length=10) 7 0 8 6 4 4 4 4 1 -1
#             cycle count : (unoptimised = 786) (optimised = 602)
            
# example 2 : (length=5) 3 99 4 2 56
#             cycle count : (unoptimised = 202) (optimised = 158)
            
# example 1 : (length=6) -64 79 4 -1 3 56
#             cycle count : (unoptimised = 284) (optimised = 192)
            
# example 1 : (length=9) 59 2 17 -64 79 4 -1 3 56
#             cycle count : (unoptimised = 628) (optimised = 435)
            
# example 1 : (length=12) 2 -99 67 8 43 21 19 300 4 9 89 0
#             cycle count : (unoptimised = 1094) (optimised = 780)