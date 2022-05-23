	lw 0 1 ten
	lw 0 2 mtwo
	lw 0 3 zero
start	add 1 2 1
	beq 1 3 2
	beq 0 0 start
	noop
done	halt
ten	.fill 10
mtwo	.fill -2
zero	.fill 0
