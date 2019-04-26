.data
.code
; rcx rdx r8 r9
_kr_pri_prYield@@8 PROC
	push rbx
	push rsi
	push rdi
	push rbp
	mov rax, rsp
	mov rsp, [ecx]
	mov [ecx], rax
	pop rbp
	pop rdi
	pop rsi
	pop rbx
	ret
_kr_pri_prYield@@8 ENDP

_kr_pri_prCallFirst@@16 PROC
	push rbx
	push rsi
	push rdi
	push rbp
	mov rax, rsp
	mov rsp, [rcx]
	mov [rcx], rax
	push rcx
	call rdx
	pop rcx;
	mov rax, rsp
	mov rsp, [rcx]
	mov [rcx], rax
	pop rbp
	pop rdi
	pop rsi
	pop rbx
    ret
_kr_pri_prCallFirst@@16 ENDP

_kr_pri_prCall@@8 PROC
	push rbx
	push rsi
	push rdi
	push rbp
	mov rax, rsp
	mov rsp, [rcx]
	mov [rcx], rax
	pop rbp
	pop rdi
	pop rsi
	pop rbx
    ret
_kr_pri_prCall@@8 ENDP

END