bits 64
	global plus, minus, equals
	section .text
_plus:
	push 	rbp
	mov 	rbp, rsp
	mov 	rax, rdi	 		; pointer to 1st param
	mov 	rbx, QWORD [rax] 	; 1st param
	mov 	rcx, QWORD [rax+8]	; 2nd param
	mov		rax, rbx
	add 	rax, rcx
	pop 	rbp
	ret

_minus:
	push 	rbp
	mov 	rbp, rsp
	mov 	rax, rdi	 		; pointer to 1st param
	mov 	rbx, QWORD [rax] 	; 1st param
	mov 	rcx, QWORD [rax+8]	; 2nd param
	mov		rax, rbx
	sub 	rax, rcx
	pop 	rbp
	ret

_equals:
	push 	rbp
	mov 	rbp, rsp
	mov 	rax, rdi	 		; pointer to 1st param
	mov 	rbx, QWORD [rax] 	; 1st param
	mov 	rcx, QWORD [rax+8]	; 2nd param
	cmp		rbx, rcx
	je		LEqual
	mov 	rax, 0
	jmp		LDone
LEqual: 
	mov		rax, 1
LDone:
	pop 	rbp
	ret

	section .data
plus:
	dq 	1, _plus, 2, 0

minus:	
	dq	1, _minus, 2, 0

equals:
	dq	1, _equals, 2, 0

