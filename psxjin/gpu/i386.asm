bits 32

section .text

%include "macros.inc"

NEWSYM i386_BGR24to16
	push ebp
	mov ebp, esp
	push ebx
	push edx
	
	mov eax, [ebp+8]            ; This can hold the G value
	mov ebx, eax                ; This can hold the R value
	mov edx, eax                ; This can hold the B value
	shr ebx, 3                  ; Move the R value
	and edx, 00f80000h          ; Mask the B value
	shr edx, 9                  ; Move the B value
	and eax, 00f800h            ; Mask the G value
	shr eax, 6                  ; Move the G value
	and ebx, 0000001fh          ; Mask the R value
	or  eax, ebx                ; Add R to G value
	or  eax, edx                ; Add B to RG value
	pop edx
	pop ebx
	mov esp, ebp
	pop ebp
	ret

NEWSYM i386_shl10idiv
	push ebp
	mov ebp, esp
	push ebx
	push edx

	mov eax, [ebp+8]
	mov ebx, [ebp+12]
	mov edx, eax
    shl eax, 10
	sar edx, 22
	idiv ebx

	pop edx
	pop ebx
	mov esp, ebp
	pop ebp
	ret
