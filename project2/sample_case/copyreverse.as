start	lw	0	0	zero	// r0 = 0
	lw	1	1	one			// r1 = 1
	lw	2	2	two			// r2 = 2
	lw	3	3	three		// r3 = 3
	add	0	4	4			// r4 = 0
	beq	0	1	start		
	sw	0	0	A4			// A4 = 0
	lw	0	7	four		// r7 = 4
	beq	0	2	start		
	sw	0	1	A3			// A3 = 1
	add	0	1	5			// r5 = 1
	beq	0	1	start
	sw	0	2	A2			// A2 = 2
	add	1	2	6			// r6 = 3
	sw	0	3	A1			// A1 = 3
	beq	0	7	start
	nor 4	5	1			// r1 = -2
	sw	0	7	A0			// A0 = 4
	beq	0	6	start
	halt
zero	.fill	0
one		.fill	1
two		.fill	2
three	.fill	3
four	.fill	4
A0		.fill	zero
A1		.fill	one
A2		.fill	two
A3		.fill	three
A4		.fill	four
