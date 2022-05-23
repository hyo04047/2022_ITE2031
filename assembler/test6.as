	lw 0 1 five	load reg 1 with 5 (symbolic)
	lw 1 2 3    load reg 2 with -1 (numeric)
start	add 1 2 1	decrement reg 1
	beq 0 1 -1	goto -1 of program when reg 1 == 0
	beq 0 0     start go to loop
	noop 
done halt 
five .fill 5
neg1 .fill 0
stAddr .fill start
