.code

NtQuerySystemInformation proc
	mov r10, rcx
	mov eax, 144h
	syscall
	ret
NtQuerySystemInformation endp

end
