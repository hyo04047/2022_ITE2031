	lw 0 1 five	    # load reg1 with 5
	lw 0 2 negl	    # load reg2 with -1
    noop
    noop
    noop
start	add 1 2 1	# decrement reg1
    noop
    noop
    noop
	beq 0 1 done	# goto end of program when reg1 == 0
    noop
    noop
    noop
	beq 0 0 start	# go back to the beginning of the loop
    noop
    noop
	noop
done	halt
five	.fill 5
negl	.fill -1
