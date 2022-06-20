	lw 0 1 one	# register 1 will be set at WB stage
	add 1 2 1	# but, add instr. use register 1 before WB stage 
	halt		# therefore, data hazard occurs
one	.fill 1
