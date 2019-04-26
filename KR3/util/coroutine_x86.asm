.model flat
.data
.code
; ecx edx

@_kr_pri_prYield@4 PROC
	push ebx
	push esi
	push edi
	push ebp
	mov eax, esp
	mov esp, [ecx]
	mov [ecx], eax
	pop ebp
	pop edi
	pop esi
	pop ebx
    ret
@_kr_pri_prYield@4 ENDP 

@_kr_pri_prCallFirst@8 PROC
	push ebx
	push esi
	push edi
	push ebp
	mov eax, esp
	mov esp, [ecx]
	mov [ecx], eax
	push ecx
	call edx
	pop ecx;
	mov eax, esp
	mov esp, [ecx]
	mov [ecx], eax
	pop ebp
	pop edi
	pop esi
	pop ebx
    ret
@_kr_pri_prCallFirst@8 ENDP

@_kr_pri_prCall@4 PROC
	push ebx
	push esi
	push edi
	push ebp
	mov eax, esp
	mov esp, [ecx]
	mov [ecx], eax
	add ecx, 4;
	pop ebp
	pop edi
	pop esi
	pop ebx
    ret
@_kr_pri_prCall@4 ENDP

END