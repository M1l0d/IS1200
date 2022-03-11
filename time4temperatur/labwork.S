  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
 
.global hexasc
.global delay
.global time2string
.global enable_interrupt
 
.data
.align 2 
.text 

hexasc:
 	addi $v0, $a0, 0x30	# hexasc will take a value from 0 to 9 and it will just add 0x30 which
 				# is an offset to convert the numbers into their ascii char
 				
	jr $ra			# jumping back to time2string
	nop
	
  time2string:
 	
 	PUSH $s0			# s0: the destination where we will save the ascii chars	
 	PUSH $s1			# s1: is the value of timeinfo ex.0x5958
 	PUSH $s2			# s2: is the the bit that will be printed 							
 	PUSH $ra			# save ra: return addr
 	PUSH $a0			# save mytime will be used in hexasc as parameter
 					
 	la $s0, ($a0)			# loading address of the ascii chars into t0
 	move $s1, $a1			# adding the values of timeinfo into t1
 	
 	# first digit
 	andi $s2, $s1, 0xF000 		# and to isolte the fourth nibble 
 	srl $a0, $s2, 12		# shifting it 12 times so it at lsb position, and setting as a0. a0 = 5
 	jal hexasc			# calling hexsac
 	nop 			
 	
 	sb $v0, 0($s0)			# stroing into byte one, v = 35
 	
 	
 	
 	jal checkValue
 	nop
 	
 	#second digit
 	andi $s2, $s1, 0xF00		# isolating third nibble a0 = 9
 	srl $a0, $s2, 8
 	jal hexasc
 	nop 
 	
 	sb $v0, 1($s0)	
 	
 	jal checkValue
 	nop			
 	
 	# :
 	addi $s2, $0, 0x3A		# here we are adding the : 
 	sb $s2, 2($s0)
 	
 	#third diggit
 	andi $s2, $s1, 0xF0		# isolating second nibble a0 = 5
 	srl $a0, $s2, 4
 	jal hexasc
 	nop 
 	
 	sb $v0, 3($s0)
 	
 	jal checkValue
 	nop		
 	
 	addi $s0, $s0, 4		# no we have added four bytes into the first address of the ascii chars, we go to next
 					# where we can also store values and it's currently 4 bytes away
 	
 	
 	sw $0, 0($s0)			# we set the values of that addr to all zeros
 		 
 	
 	#fourth digit	 
 	andi $a0, $s1, 0xF		#isolating first nibble a0 = 8
 	jal hexasc
 	nop 
 	
 	sb $v0, 0($s0)			# storing it
 	
 	jal checkValue
 	nop	
 	
 	# null mark 
 	addi $s2, $0, 0x00
 	sb $s2, 1($s0)			# adding the null mark to mark end of string 
 	
 	beq $v1, 4, HOUR
 	andi $t4, $t4, 0 		# restore t
 	nop
 	
time2StringDone:
	POP $a0				# restore a1
	POP $ra				# restoring ra
	POP $s2				# restore s2		
	POP $s1				# restore s1
	POP $s0				# restore s0
 	
 	jr $ra				# jumping back to main
 	nop
 		
 checkValue: 
 	beq $v0, 0x30, increment

 checkValueDone:
 	jr $ra
 	nop
 
 increment:
 	addi $t4, $t4, 1
 	move $v1, $t4
 	j checkValueDone
 	nop
 		
 HOUR:
 	la $t3, -4($s0)
 	sw $0, ($t3)
 	
 	addi $t4, $0, 0x48 	# H
 	sb $t4, 0($t3)
 	
 	addi $t4, $0, 0x4f	# O
 	sb $t4, 1($t3)
 	
 	addi $t4, $0, 0x55	# U
 	sb $t4, 2($t3)
 	
 	addi $t4, $0, 0x52	# R
 	sb $t4, 3($t3)
 	
 	sw $0, 4($t3) 	
 	
 	j time2StringDone
 	nop
 	
 	
 enable_interrupt:
	ei
	jr $ra
	nop
 	
 	
