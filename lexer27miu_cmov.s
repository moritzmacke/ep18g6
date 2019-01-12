	.file	"lexer27miu.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"Usage: %s <file>\n"
.LC1:
	.string	"Cannot open %s\n"
.LC2:
	.string	"%lx\n"
.LC3:
	.string	"Invalid character %c\n"
.LC4:
	.string	"mmap failed.\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB40:
	.cfi_startproc
	pushq	%r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	pushq	%r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	pushq	%r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	pushq	%rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	movq	%rsi, %rbx
	subq	$144, %rsp
	.cfi_def_cfa_offset 192
	cmpl	$2, %edi
	jne	.L49
	movq	8(%rsi), %rdi
	xorl	%eax, %eax
	xorl	%esi, %esi
	call	open@PLT
	testl	%eax, %eax
	movl	%eax, %ebp
	js	.L50
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
	leaq	3(%rbx), %rsi
	call	mmap@PLT
	cmpq	$-1, %rax
	movb	$10, (%rax,%rbx)
	movb	$0, 1(%rax,%rbx)
	je	.L4
	leaq	1(%rax), %r11
	movsbq	(%rax), %rax
	leaq	typeLookup(%rip), %r10
	leaq	.L28(%rip), %rbp
	leaq	kwLookup(%rip), %rbx
	xorl	%esi, %esi
	movabsq	$-4953053512429003327, %r9
	movabsq	$-1526479174790530323, %r13
	movabsq	$-4205821536314178611, %r12
	movzbl	(%r10,%rax), %edi
	.p2align 4,,10
	.p2align 3
  
.L5:
	movsbq	(%r11), %r14
	movq	%rax, %rcx
	leaq	1(%r11), %r8
	imulq	%r9, %rcx
	cmpb	$12, %dil
	movq	%r14, %rdx
	je	.L46
	cmpb	$8, %dil
	je	.L8
	cmpb	$27, %dil
	jbe	.L9
	movzbl	(%r10,%r14), %edi
	cmpb	$23, %dil
	jbe	.L38
	salq	$8, %rax
	addq	%r14, %rcx
	leaq	(%rax,%r14), %rdx
	movsbq	1(%r11), %rax
	imulq	%r9, %rcx
	movzbl	(%r10,%rax), %edi
	cmpb	$23, %dil
	ja	.L43
	jmp	.L51
	.p2align 4,,10
	.p2align 3
.L13:
	salq	$8, %rdx
	addq	%rax, %rcx
	addq	%rax, %rdx
	movsbq	2(%r8), %rax
	imulq	%r9, %rcx
	movzbl	(%r10,%rax), %edi
	cmpb	$23, %dil
	jbe	.L52
	salq	$8, %rdx
	addq	%rax, %rcx
	addq	%rax, %rdx
	movsbq	3(%r8), %rax
	imulq	%r9, %rcx
	movzbl	(%r10,%rax), %edi
	cmpb	$23, %dil
	jbe	.L53
	salq	$8, %rdx
	addq	%rax, %rcx
	addq	%rax, %rdx
	movsbq	4(%r8), %rax
	imulq	%r9, %rcx
	movzbl	(%r10,%rax), %edi
	cmpb	$23, %dil
	jbe	.L54
	salq	$8, %rdx
	addq	%rax, %rcx
	addq	%rax, %rdx
	movsbq	5(%r8), %rax
	imulq	%r9, %rcx
	movzbl	(%r10,%rax), %edi
	cmpb	$23, %dil
	jbe	.L55
	salq	$8, %rdx
	addq	%rax, %rcx
	addq	%rax, %rdx
	movsbq	6(%r8), %rax
	imulq	%r9, %rcx
	movzbl	(%r10,%rax), %edi
	cmpb	$23, %dil
	jbe	.L56
	leaq	8(%r8), %r11
	salq	$8, %rdx
	addq	%rax, %rcx
	addq	%rax, %rdx
	movsbq	-1(%r11), %rax
	imulq	%r9, %rcx
	movzbl	(%r10,%rax), %edi
	cmpb	$23, %dil
	jbe	.L10
	salq	$8, %rdx
	addq	%rax, %rcx
	addq	%rax, %rdx
	movsbq	8(%r8), %rax
	imulq	%r9, %rcx
	movzbl	(%r10,%rax), %edi
	cmpb	$23, %dil
	jbe	.L57
	movq	%r11, %r8
.L43:
	salq	$8, %rdx
	addq	%rax, %rcx
	addq	%rax, %rdx
	movsbq	1(%r8), %rax
	imulq	%r9, %rcx
	movzbl	(%r10,%rax), %edi
	cmpb	$23, %dil
	ja	.L13
	leaq	2(%r8), %r11
	.p2align 4,,10
	.p2align 3
.L10:
	leaq	0(,%rcx,4), %r8
	andl	$496, %r8d
	addq	%rbx, %r8
	cmpq	%rdx, (%r8)
	cmove	8(%r8), %rcx
.L20:
	movslq	%ecx, %rcx
	addq	%rcx, %rsi
	imulq	%r9, %rsi
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L46:
	imulq	%r9, %rsi
	movzbl	(%r10,%r14), %edi
	movq	%r14, %rax
	movq	%r8, %r11
	addq	%rcx, %rsi
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L9:
	orb	128(%r10,%r14), %dil
	cmpb	$23, %dil
	jbe	.L21
	subl	$48, %edx
	leaq	-48(%rax), %rcx
	cmpb	$9, %dl
	ja	.L40
	movq	%r8, %r11
	.p2align 4,,10
	.p2align 3
.L23:
	leaq	(%rcx,%rcx,4), %rax
	movzbl	%dl, %edx
	addq	$1, %r11
	leaq	(%rdx,%rax,2), %rcx
	movsbq	-1(%r11), %rax
	leal	-48(%rax), %edx
	cmpb	$9, %dl
	jbe	.L23
.L22:
	xorb	$128, %ch
	movzbl	(%r10,%rax), %edi
	movslq	%ecx, %rcx
	addq	%rcx, %rsi
	imulq	%r9, %rsi
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L8:
	movzbl	(%r10,%r14), %edi
	movq	%r14, %rax
	movq	%r8, %r11
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L21:
	cmpb	$20, %dil
	ja	.L46
	je	.L59
	subl	$7, %edi
	cmpb	$12, %dil
	ja	.L26
	movzbl	%dil, %edi
	movslq	0(%rbp,%rdi,4), %rcx
	addq	%rbp, %rcx
	jmp	*%rcx
	.section	.rodata
	.align 4
	.align 4
.L28:
	.long	.L27-.L28
	.long	.L26-.L28
	.long	.L26-.L28
	.long	.L26-.L28
	.long	.L26-.L28
	.long	.L26-.L28
	.long	.L26-.L28
	.long	.L26-.L28
	.long	.L26-.L28
	.long	.L29-.L28
	.long	.L30-.L28
	.long	.L29-.L28
	.long	.L29-.L28
	.section	.text.startup
.L30:
	movq	%r8, %r11
	.p2align 4,,10
	.p2align 3
.L35:
	addq	$1, %r11
	cmpb	$10, -1(%r11)
	jne	.L35
	movl	$8, %edi
	movl	$10, %eax
	jmp	.L5
.L29:
	imulq	%r9, %rsi
	movzbl	(%r10,%r14), %edi
	movq	%r14, %rax
	movq	%r8, %r11
	addq	%r13, %rsi
	jmp	.L5
.L27:
	subl	$48, %edx
	leaq	2(%r11), %r8
	movl	%edx, %eax
	andl	$15, %eax
	addl	$9, %eax
	cmpb	$10, %dl
	cmovnb	%eax, %edx
	movsbq	1(%r11), %rax
	movzbl	%dl, %edx
	jmp	.L32
	.p2align 4,,10
	.p2align 3
.L34:
	subl	$48, %eax
	salq	$4, %rdx
	movl	%eax, %ecx
	andl	$15, %ecx
	addl	$9, %ecx
	cmpb	$10, %al
	cmovnb	%ecx, %eax
	addq	$1, %r8
	movzbl	%al, %eax
	addq	%rax, %rdx
	movsbq	-1(%r8), %rax
.L32:
	movsbl	%al, %ecx
	subl	$48, %ecx
	cmpl	$9, %ecx
	jbe	.L34
	movl	%eax, %ecx
	orl	$32, %ecx
	movsbl	%cl, %ecx
	subl	$97, %ecx
	cmpl	$5, %ecx
	jbe	.L34
	xorb	$64, %dh
	movzbl	(%r10,%rax), %edi
	movq	%r8, %r11
	movslq	%edx, %rdx
	addq	%rdx, %rsi
	imulq	%r9, %rsi
	jmp	.L5
.L26:
	testq	%rax, %rax
	je	.L60
	movq	stderr(%rip), %rdi
	leaq	.LC3(%rip), %rsi
	movq	%rax, %rdx
	xorl	%eax, %eax
	call	fprintf@PLT
.L44:
	addq	$144, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%rbp
	.cfi_def_cfa_offset 32
	popq	%r12
	.cfi_def_cfa_offset 24
	popq	%r13
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L58:
	.cfi_restore_state
	movslq	8(%r8), %rcx
	jmp	.L20
	.p2align 4,,10
	.p2align 3
.L52:
	leaq	3(%r8), %r11
	jmp	.L10
	.p2align 4,,10
	.p2align 3
.L53:
	leaq	4(%r8), %r11
	jmp	.L10
	.p2align 4,,10
	.p2align 3
.L54:
	leaq	5(%r8), %r11
	jmp	.L10
	.p2align 4,,10
	.p2align 3
.L55:
	leaq	6(%r8), %r11
	jmp	.L10
	.p2align 4,,10
	.p2align 3
.L56:
	leaq	7(%r8), %r11
	jmp	.L10
	.p2align 4,,10
	.p2align 3
.L57:
	leaq	9(%r8), %r11
	jmp	.L10
	.p2align 4,,10
	.p2align 3
.L59:
	movsbq	1(%r11), %rax
	addq	$2, %r11
	imulq	%r9, %rsi
	movzbl	(%r10,%rax), %edi
	addq	%r12, %rsi
	jmp	.L5
.L40:
	movq	%r14, %rax
	movq	%r8, %r11
	jmp	.L22
.L38:
	movq	%rax, %rdx
	movq	%r8, %r11
	movq	%r14, %rax
	jmp	.L10
.L51:
	addq	$2, %r11
	jmp	.L10
.L60:
	leaq	.LC2(%rip), %rdi
	call	printf@PLT
	jmp	.L44
.L4:
	movq	stderr(%rip), %rcx
	leaq	.LC4(%rip), %rdi
	movl	$13, %edx
	movl	$1, %esi
	call	fwrite@PLT
	movl	$1, %edi
	call	exit@PLT
.L50:
	movq	8(%rbx), %rdx
	leaq	.LC1(%rip), %rsi
.L47:
	movq	stderr(%rip), %rdi
	xorl	%eax, %eax
	call	fprintf@PLT
	movl	$1, %edi
	call	exit@PLT
.L49:
	movq	(%rsi), %rdx
	leaq	.LC0(%rip), %rsi
	jmp	.L47
	.cfi_endproc
.LFE40:
	.size	main, .-main
	.section	.rodata
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
	.align 32
	.type	typeLookup, @object
	.size	typeLookup, 256
typeLookup:
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	8
	.byte	8
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
	.byte	8
	.byte	0
	.byte	0
	.byte	12
	.byte	4
	.byte	0
	.byte	0
	.byte	0
	.byte	12
	.byte	12
	.byte	12
	.byte	12
	.byte	12
	.byte	16
	.byte	0
	.byte	0
	.byte	24
	.byte	24
	.byte	24
	.byte	24
	.byte	24
	.byte	24
	.byte	24
	.byte	24
	.byte	24
	.byte	24
	.byte	20
	.byte	12
	.byte	12
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	12
	.byte	0
	.byte	12
	.byte	0
	.byte	0
	.byte	0
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	28
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	1
	.byte	2
	.byte	2
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	2
	.byte	2
	.byte	2
	.byte	0
	.byte	2
	.byte	2
	.byte	2
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	3
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.byte	2
	.ident	"GCC: (Debian 6.3.0-18+deb9u1) 6.3.0 20170516"
	.section	.note.GNU-stack,"",@progbits
