
# I really should improve this fking shit.

.intel_syntax noprefix

.extern __rld_debug_ptr
.extern __rld_import_hash
.extern __rld_import_addr
.extern __rld_num_imports
.extern main

.global __rld_start

.text 0
__rld_get_nsym_gnuhash:
	enter 12,0
	mov edi , DWORD PTR[eax+4]
	mov eax , DWORD PTR[edi]
	mov ecx , 4
	mul ecx
	mov DWORD PTR[ebp-4] , eax
	mov eax , DWORD PTR[edi+4]
	mov DWORD PTR[ebp-8] , eax
	mov eax , DWORD PTR[edi+8]
	mul ecx
	add edi , eax
	add edi , 16
	mov edx , edi
	add edx , DWORD PTR[ebp-4]
	mov DWORD PTR[ebp-12] , 0x00000000
	xor ecx , ecx
	gnubucket:
		cmp ecx , DWORD PTR[ebp-4]
		je gnubucket_end
		mov eax , DWORD PTR[edi+ecx]
		test eax , eax
		jz gnubucket_next

		push ecx

		mov ecx , DWORD PTR[edi+ecx]
		sub ecx , DWORD PTR[ebp-8]
		
		xor ebx , ebx
		inc ebx
		gnuchain:
			test DWORD PTR[edx+ecx*4] , 0x00000001
			jnz gnuchain_end
			inc ebx
			inc ecx
		jmp gnuchain
		gnuchain_end:
		add DWORD PTR[ebp-12] , ebx
		pop ecx

		gnubucket_next:
		add ecx , 4
	jmp gnubucket
	gnubucket_end:

	mov ecx , DWORD PTR[ebp-12]
	add ecx , DWORD PTR[ebp-8]
	
	leave
	ret

__rld_getsym:
	enter 4,0
	mov DWORD PTR[ebp-4] , eax
	mov esi , DWORD PTR[__rld_debug_ptr]
	mov esi , DWORD PTR[esi+4]
	mov esi , DWORD PTR[esi+12]
	mov esi , DWORD PTR[esi+12]

	find_library:
		mov eax , DWORD PTR[esi+8]
		find_hash:
			mov ebx , DWORD PTR[eax]
			test ebx , ebx
			jz _find_gnuhash
			cmp ebx , 4
			je hash_found
		       	add eax , 8
		jmp find_hash 
		hash_found:
		mov ecx , DWORD PTR[eax+4]
		mov ecx , DWORD PTR[ecx+4]
		jmp _find_strtab

		_find_gnuhash:
		mov eax , DWORD PTR[esi+8]
		find_gnuhash:
			cmp DWORD PTR[eax] , 0x6ffffef5
			je gnuhash_found
			add eax , 8
		jmp find_gnuhash
		gnuhash_found:
		call __rld_get_nsym_gnuhash

		_find_strtab:
		mov eax , DWORD PTR[esi+8]
		find_strtab:
			cmp DWORD PTR[eax] , 5
			je strtab_found
			add eax , 8
		jmp find_strtab
		strtab_found:
		mov ebx , DWORD PTR[eax+4]

		mov eax , DWORD PTR[esi+8]
		find_symtab:
			cmp DWORD PTR[eax] , 6
			je symtab_found
			add eax , 8
		jmp find_symtab
		symtab_found:
		mov edx , DWORD PTR[eax+4]

		push esi
		find_sym:
			mov esi , DWORD PTR[edx]
			add esi , ebx
			xor edi , edi
			symhash:
				xor eax,eax
				lodsb
				test al,al
				jz symhash_end 
				sub eax , edi
				shl edi , 6
				add eax , edi
				shl edi , 10
				add eax , edi
				mov edi , eax
			jmp symhash
			symhash_end:
			cmp edi , DWORD PTR[ebp-4]
			jz symhash_found
			add edx , 16
		loop find_sym
		pop esi
		jmp fl_next
		symhash_found:
		mov eax , [edx+4]
		pop esi
    test eax , eax
    jz fl_next 
		add eax , DWORD PTR[esi]
		jmp _end

		fl_next:
		mov esi , DWORD PTR[esi+12]
		test esi , esi
		jz _end
	jmp find_library
	_end:
	leave
	ret


__rld_start:
push ebp
mov ebp, esp
.byte 0xbe
.int __rld_import_hash #mov esi, __rld_import_hash
.byte 0xbf
.int __rld_import_addr #mov edi, __rld_import_addr
.byte 0xb9
.int __rld_num_imports #mov ecx, __rld_num_imports

	sym:
		lodsd
		pusha
		call __rld_getsym
		mov DWORD PTR[esp+28] , eax
		popa
		stosd
	loop sym

  xor eax, eax
  call main
  xor eax, eax
  inc eax
  int 0x80
