	lw 0 1 one	# simple branch hazard
	lw 0 2 two
	noop		# to avoid data hazard
	noop		
	noop
	beq 0 0 end	# if branch taken, branch hazard happen
	add 1 1 2	# 3 noop instr should be needed after branch when branch-taken
	add 2 1 2	# should flush IFID, IDEX, EXMEM registers
    add 3 1 2   # flushed
end	lw 0 3 three	
	halt
one	.fill 1
two	.fill 2
three	.fill 3
