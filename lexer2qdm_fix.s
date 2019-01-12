	.file	"lexer2qdm.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"%llx\n"
.LC1:
	.string	"Invalid character %c\n"
	.text
	.p2align 4,,15
	.type	lex, @function
lex:
.LFB39:
	.cfi_startproc
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	leaq	actionLookup.3456(%rip), %r11
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	xorl	%r10d, %r10d
	subq	$8, %rsp
	.cfi_def_cfa_offset 48
  
  movabsq	$-4953053512429003327, %r8
  leaq	letterLookup(%rip), %r9
	leaq	kwLookup(%rip), %rbx
	movabsq	$7870801458470632890, %rbp
	movabsq	$-4205821536314178611, %r12
  
.L2:
	movsbq	(%rdi), %rdx
	addq	$1, %rdi
	jmp	*(%r11,%rdx,8)
.L23:
	movsbq	(%rdi), %rax
	leaq	1(%rdi), %r13
	movq	%rax, %rdx
	subl	$48, %eax
	cmpl	$9, %eax
	jbe	.L24
	movl	%edx, %eax
	orl	$32, %eax
	movsbl	%al, %eax
	subl	$97, %eax
	cmpl	$5, %eax
	jbe	.L24
.L30:
	movq	stderr(%rip), %rdi
	leaq	.LC1(%rip), %rsi
	xorl	%eax, %eax
	call	fprintf@PLT
	xorl	%eax, %eax
.L1:
	popq	%rdx
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
.L29:
	.cfi_restore_state
	testq	%rdx, %rdx
	jne	.L30
	leaq	.LC0(%rip), %rdi
	movq	%r10, %rsi
	xorl	%eax, %eax
	call	printf@PLT
	movl	$1, %eax
	jmp	.L1
.L4:
.L17:
	addq	%rdx, %r10
	imulq	%r8, %r10
.L19:
	movsbq	(%rdi), %rdx
	addq	$1, %rdi
	jmp	*(%r11,%rdx,8)
  
.L6:
	movsbq	(%rdi), %rdx
	cmpq	$61, %rdx
	je	.L42
	imulq	%r8, %r10
	leaq	1(%rdi), %r13
	addq	%rbp, %r10
.L8:
	movq	%r13, %rdi
	jmp	*(%r11,%rdx,8)
  
.L9:
	movsbq	(%rdi), %rax
	movq	%rdx, %rcx
	imulq	%r8, %rcx
	cmpb	$0, (%r9,%rax)
	je	.L10
  
	.p2align 4,,10
	.p2align 3
.L11:
	salq	$8, %rdx
	addq	$1, %rdi
	addq	%rax, %rcx
	orq	%rax, %rdx
	movsbq	(%rdi), %rax
	imulq	%r8, %rcx
	cmpb	$0, (%r9,%rax)
	jne	.L11
  
.L10:
	leaq	0(,%rcx,4), %rsi
	andl	$496, %esi
	addq	%rbx, %rsi
	cmpq	%rdx, (%rsi)
	cmove	8(%rsi), %ecx
.L12:
	movslq	%ecx, %rcx
	addq	$1, %rdi
	movq	%rax, %rdx
	addq	%rcx, %r10
	movq	(%r11,%rax,8), %rcx
	imulq	%r8, %r10
	jmp	*%rcx
  
.L13:
	movq	%rdx, %rsi
	movsbq	(%rdi), %rdx
	imulq	%r8, %rsi
	cmpb	$0, (%r9,%rdx)
	je	.L14
	.p2align 4,,10
	.p2align 3
.L15:
	addq	$1, %rdi
	addq	%rdx, %rsi
	movsbq	(%rdi), %rdx
	imulq	%r8, %rsi
	cmpb	$0, (%r9,%rdx)
	jne	.L15
.L14:
	movslq	%esi, %rsi
	addq	$1, %rdi
	addq	%rsi, %r10
	imulq	%r8, %r10
	jmp	*(%r11,%rdx,8)
  
.L16:
	cmpb	$45, (%rdi)
	jne	.L17
.L18:
	addq	$1, %rdi
	cmpb	$10, (%rdi)
	je	.L19
	addq	$1, %rdi
	cmpb	$10, (%rdi)
	jne	.L18
	jmp	.L19
.L20:
	movzbl	(%rdi), %eax
	subq	$48, %rdx
	leal	-48(%rax), %ecx
	cmpb	$9, %cl
	ja	.L21
	.p2align 4,,10
	.p2align 3
.L22:
	leaq	(%rdx,%rdx,4), %rax
	movzbl	%cl, %ecx
	addq	$1, %rdi
	leaq	(%rcx,%rax,2), %rdx
	movzbl	(%rdi), %eax
	leal	-48(%rax), %ecx
	cmpb	$9, %cl
	jbe	.L22
.L21:
	xorb	$128, %dh
	addq	$1, %rdi
	movslq	%edx, %rsi
	movsbq	%al, %rdx
	addq	%rsi, %r10
	imulq	%r8, %r10
	jmp	*(%r11,%rdx,8)
.L24:
	subl	$48, %edx
	movl	%edx, %eax
	andl	$15, %eax
	addl	$9, %eax
	cmpb	$10, %dl
	cmovnb	%eax, %edx
	movzbl	%dl, %eax
	movsbq	1(%rdi), %rdx
	.p2align 4,,10
	.p2align 3
.L26:
	movsbl	%dl, %ecx
	subl	$48, %ecx
	cmpl	$9, %ecx
	jbe	.L28
	movl	%edx, %ecx
	orl	$32, %ecx
	movsbl	%cl, %ecx
	subl	$97, %ecx
	cmpl	$5, %ecx
	jbe	.L28
	xorb	$64, %ah
	leaq	1(%r13), %rdi
	cltq
	addq	%rax, %r10
	imulq	%r8, %r10
	jmp	*(%r11,%rdx,8)
  
	.p2align 4,,10
	.p2align 3
.L28:
	subl	$48, %edx
	salq	$4, %rax
	movl	%edx, %ecx
	andl	$15, %ecx
	addl	$9, %ecx
	cmpb	$10, %dl
	cmovnb	%ecx, %edx
	addq	$1, %r13
	movzbl	%dl, %edx
	addq	%rdx, %rax
	movsbq	0(%r13), %rdx
	jmp	.L26

.L42:
	imulq	%r8, %r10
	leaq	2(%rdi), %r13
	movsbq	1(%rdi), %rdx
	addq	%r12, %r10
	jmp	.L8
	.cfi_endproc
.LFE39:
	.size	lex, .-lex
	.section	.rodata.str1.1
.LC2:
	.string	"Usage: %s <file>\n"
.LC3:
	.string	"Cannot open %s\n"
.LC4:
	.string	"mmap failed.\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB40:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	movq	%rsi, %rbx
	subq	$152, %rsp
	.cfi_def_cfa_offset 176
	cmpl	$2, %edi
	jne	.L52
	movq	8(%rsi), %rdi
	xorl	%eax, %eax
	xorl	%esi, %esi
	call	open@PLT
	testl	%eax, %eax
	movl	%eax, %ebp
	js	.L53
	movq	%rsp, %rdx
	movl	%eax, %esi
	movl	$1, %edi
	call	__fxstat@PLT
	movq	48(%rsp), %rbx
	xorl	%r9d, %r9d
	xorl	%edi, %edi
	movl	%ebp, %r8d
	movl	$32770, %ecx
	movl	$3, %edx
	leaq	2(%rbx), %rsi
	call	mmap@PLT
	cmpq	$-1, %rax
	movb	$10, (%rax,%rbx)
	movb	$0, 1(%rax,%rbx)
	je	.L54
	movq	%rax, %rdi
	call	lex
	addq	$152, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
.L52:
	.cfi_restore_state
	movq	(%rsi), %rdx
	leaq	.LC2(%rip), %rsi
.L51:
	movq	stderr(%rip), %rdi
	xorl	%eax, %eax
	call	fprintf@PLT
	movl	$1, %edi
	call	exit@PLT
.L54:
	movq	stderr(%rip), %rcx
	leaq	.LC4(%rip), %rdi
	movl	$13, %edx
	movl	$1, %esi
	call	fwrite@PLT
	movl	$1, %edi
	call	exit@PLT
.L53:
	movq	8(%rbx), %rdx
	leaq	.LC3(%rip), %rsi
	jmp	.L51
	.cfi_endproc
.LFE40:
	.size	main, .-main
	.section	.data.rel.ro.local,"aw",@progbits
	.align 32
	.type	actionLookup.3456, @object
	.size	actionLookup.3456, 1024
actionLookup.3456:
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L2
	.quad	.L2
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L2
	.quad	.L29
	.quad	.L29
	.quad	.L4
	.quad	.L23
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L4
	.quad	.L4
	.quad	.L4
	.quad	.L4
	.quad	.L4
	.quad	.L16
	.quad	.L29
	.quad	.L29
	.quad	.L20
	.quad	.L20
	.quad	.L20
	.quad	.L20
	.quad	.L20
	.quad	.L20
	.quad	.L20
	.quad	.L20
	.quad	.L20
	.quad	.L20
	.quad	.L6
	.quad	.L4
	.quad	.L4
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L4
	.quad	.L29
	.quad	.L4
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L9
	.quad	.L13
	.quad	.L13
	.quad	.L9
	.quad	.L9
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L9
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L9
	.quad	.L9
	.quad	.L13
	.quad	.L13
	.quad	.L9
	.quad	.L13
	.quad	.L9
	.quad	.L13
	.quad	.L9
	.quad	.L9
	.quad	.L13
	.quad	.L13
	.quad	.L13
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.quad	.L29
	.section	.rodata
	.align 32
	.type	letterLookup, @object
	.size	letterLookup, 128
letterLookup:
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	-1
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.align 32
	.type	kwLookup, @object
	.size	kwLookup, 528
kwLookup:
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	6909556
	.long	259
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	25711
	.long	265
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	418531926393
	.long	257
	.zero	4
	.quad	125780071117422
	.long	260
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	1952998766
	.long	262
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	7758194
	.long	266
	.zero	4
	.quad	26982
	.long	261
	.zero	4
	.quad	7237492
	.long	267
	.zero	4
	.quad	28518
	.long	258
	.zero	4
	.quad	512852847717
	.long	264
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	28530
	.long	268
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	1701606245
	.long	263
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	6647396
	.long	256
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.quad	0
	.long	0
	.zero	4
	.ident	"GCC: (Debian 6.3.0-18+deb9u1) 6.3.0 20170516"
	.section	.note.GNU-stack,"",@progbits
