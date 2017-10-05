PUBLIC IsInsideVM_asm
PUBLIC idtCheck_asm
PUBLIC sgdtCheck_asm
PUBLIC sldtCheck_asm
.code

idtCheck_asm proc
	sidt tbyte ptr [rcx]
	ret
idtCheck_asm endp

sgdtCheck_asm proc
	SGDT tbyte ptr [rcx]
	ret
sgdtCheck_asm endp

sldtCheck_asm proc
	sldt word ptr[rcx]
	ret
sldtCheck_asm endp

IsInsideVM_asm proc

      push   rdx
      push   rcx
      push   rbx

      mov    rax, 'VMXh'
      mov    rbx, 0     ; any value but not the MAGIC VALUE
      mov    rcx, 10    ; get VMWare version
      mov    rdx, 'VX'  ; port number

      in     rax, dx    ; read port
                        ; on return EAX returns the VERSION
      cmp    rbx, 'VMXh'; is it a reply from VMWare?
      setz   al         ; set return value
      movzx rax,al

      pop    rbx
      pop    rcx
      pop    rdx

      ret
IsInsideVM_asm endp

END