	.file	"lexer2qd.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"%llx\n"
.LC1:
	.string	"Invalid character %c\n"
	.text
	.p2align 4,,15
	.type	lex, @function
lex:
.LFB34:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32

	leaq	actionLookup.3246(%rip), %r10
	xorl	%esi, %esi
	movabsq	$-4953053512429003327, %r8
  leaq	letterLookup(%rip), %r9
	leaq	kwLookup(%rip), %r11
	movabsq	$7870801458470632890, %rbx
	movabsq	$-4205821536314178611, %rbp
  
.loop:
	movsbq	(%rdi), %rdx
	addq	$1, %rdi
	jmp	*(%r10,%rdx,8)
    
.fail:
	testq	%rdx, %rdx
	jne	.pfail
	leaq	.LC0(%rip), %rdi
	xorl	%eax, %eax
	call	printf@PLT
	movl	$1, %eax
	jmp	.doret
.pfail:
	movq	stderr(%rip), %rdi
	leaq	.LC1(%rip), %rsi
	xorl	%eax, %eax
	call	fprintf@PLT
	xorl	%eax, %eax
.doret:
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
  
.coln: #: 58  ASS 269  diff 211
	.cfi_restore_state
  addq %rdx, %rsi
  movsbq (%rdi), %rdx
  addq $1, %rdi
  cmp $61, %rdx
  jne .islex
  add $211, %rsi
  movsbq (%rdi), %rdx
  add $1, %rdi
.islex:
  imulq %r8, %rsi
  jmp *(%r10,%rdx,8)
  
.kltr:
	movq	%rdx, %rax
	movq	%rdx, %rcx
	movsbq	(%rdi), %rdx
	imulq	%r8, %rax
	cmpb	$0, (%r9,%rdx)
	je	.kwcheck
	.p2align 4,,10
	.p2align 3
.kltrloop:
	salq	$8, %rcx
	addq	$1, %rdi
	addq	%rdx, %rax
	orq	%rdx, %rcx
	movsbq	(%rdi), %rdx
	imulq	%r8, %rax
	cmpb	$0, (%r9,%rdx)
	jne	.kltrloop
.kwcheck:
	leaq	0(,%rax,4), %r12
	andl	$496, %r12d
	cmpq	%rcx, (%r11,%r12)
  cmove 8(%r11,%r12), %eax # rep
.L14:
	cltq
	addq	$1, %rdi
	addq	%rax, %rsi
	imulq	%r8, %rsi
	jmp	*(%r10,%rdx,8)
  
.altr:
	movq	%rdx, %rax
	movsbq	(%rdi), %rdx
	imulq	%r8, %rax
	cmpb	$0, (%r9,%rdx)
	je	.L14
	.p2align 4,,10
	.p2align 3
.altrloop:
	addq	$1, %rdi
	addq	%rdx, %rax
	movsbq	(%rdi), %rdx
	imulq	%r8, %rax
	cmpb	$0, (%r9,%rdx)
	jne	.altrloop
#	jmp	.L14
.L14a:
	cltq
	addq	$1, %rdi
	addq	%rax, %rsi
	imulq	%r8, %rsi
	jmp	*(%r10,%rdx,8)
  
.lexc:
	addq	%rdx, %rsi
	imulq	%r8, %rsi
	movsbq	(%rdi), %rdx
	addq	$1, %rdi
	jmp	*(%r10,%rdx,8)
  
.digt:
	movzbl	(%rdi), %ecx
	subq	$48, %rdx
	leal	-48(%rcx), %eax
	cmpb	$9, %al
	ja	.digtend
	.p2align 4,,10
	.p2align 3
.digtloop:
	addq	$1, %rdi
	movzbl	(%rdi), %ecx
	leaq	(%rdx,%rdx,4), %rdx
#	movzbl	%al, %eax dont need since <9 anway?
	leaq	(%rax,%rdx,2), %rdx
	leal	-48(%rcx), %eax
	cmpb	$9, %al
	jbe	.digtloop
.digtend:
	xorb	$128, %dh
	addq	$1, %rdi
	movslq	%edx, %rax
	movsbq	%cl, %rdx
	addq	%rax, %rsi
	imulq	%r8, %rsi
	jmp	*(%r10,%rdx,8)

  
.dash:
	cmpb	$45, (%rdi)
	jne	.lexc
.cmtloop:
	addq	$1, %rdi
	cmpb	$10, (%rdi)
	je	.cmtend
	addq	$1, %rdi
	cmpb	$10, (%rdi)
	jne	.cmtloop
.cmtend:
	movsbq	1(%rdi), %rdx #skip the '\n'
	addq	$2, %rdi
	jmp	*(%r10,%rdx,8)
  
  
  
.cash:
	movsbq	(%rdi), %rax
	leaq	1(%rdi), %rcx
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
  jmp .pfail
  
#hex
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
	leal	-48(%rdx), %edi
	cmpl	$9, %edi
	jbe	.L28
	movl	%edx, %edi
	orl	$32, %edi
	movsbl	%dil, %edi
	subl	$97, %edi
	cmpl	$5, %edi
	jbe	.L28
	xorb	$64, %ah
	leaq	1(%rcx), %rdi
	cltq
	addq	%rax, %rsi
	imulq	%r8, %rsi
	jmp	*(%r10,%rdx,8)
	.p2align 4,,10
	.p2align 3
.L28:
	subl	$48, %edx
	salq	$4, %rax
	movl	%edx, %edi
	andl	$15, %edi
	addl	$9, %edi
	cmpb	$10, %dl
	cmovnb	%edi, %edx
	addq	$1, %rcx
	movzbl	%dl, %edx
	addq	%rdx, %rax
	movsbq	(%rcx), %rdx
	jmp	.L26



	.cfi_endproc
.LFE34:
	.size	lex, .-lex
	.section	.rodata.str1.1
.LC2:
	.string	"Usage: %s <file>\n"
.LC3:
	.string	"rb"
.LC4:
	.string	"Cannot open %s\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB35:
	.cfi_startproc
	cmpl	$2, %edi
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	movq	%rsi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	jne	.L50
	movq	8(%rsi), %rdi
	leaq	.LC3(%rip), %rsi
	call	fopen@PLT
	testq	%rax, %rax
	movq	%rax, %rbx
	je	.L51
	movl	$2, %edx
	xorl	%esi, %esi
	movq	%rax, %rdi
	call	fseek@PLT
	movq	%rbx, %rdi
	call	ftello@PLT
	xorl	%edx, %edx
	movq	%rax, %r12
	xorl	%esi, %esi
	movq	%rbx, %rdi
	call	fseek@PLT
	leaq	2(%r12), %rdi
	call	malloc@PLT
	movq	%rbx, %rcx
	movq	%rax, %rbp
	movq	%r12, %rdx
	movl	$1, %esi
	movq	%rax, %rdi
	call	fread@PLT
	movb	$0, 0(%rbp,%r12)
	movq	%rbx, %rdi
	call	fclose@PLT
	movq	%rbp, %rdi
	call	lex
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	xorl	%eax, %eax
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
.L50:
	.cfi_restore_state
	movq	(%rsi), %rdx
	leaq	.LC2(%rip), %rsi
.L49:
	movq	stderr(%rip), %rdi
	xorl	%eax, %eax
	call	fprintf@PLT
	movl	$1, %edi
	call	exit@PLT
.L51:
	movq	8(%rbp), %rdx
	leaq	.LC4(%rip), %rsi
	jmp	.L49
	.cfi_endproc
.LFE35:
	.size	main, .-main
	.section	.data.rel.ro.local,"aw",@progbits
	.align 32
	.type	actionLookup.3246, @object
	.size	actionLookup.3246, 1024
actionLookup.3246:
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.loop
	.quad	.loop
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.loop
	.quad	.fail
	.quad	.fail
	.quad	.lexc
	.quad	.cash
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.lexc
	.quad	.lexc
	.quad	.lexc
	.quad	.lexc
	.quad	.lexc
	.quad	.dash
	.quad	.fail
	.quad	.fail
	.quad	.digt
	.quad	.digt
	.quad	.digt
	.quad	.digt
	.quad	.digt
	.quad	.digt
	.quad	.digt
	.quad	.digt
	.quad	.digt
	.quad	.digt
	.quad	.coln
	.quad	.lexc
	.quad	.lexc
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.lexc
	.quad	.fail
	.quad	.lexc
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.kltr
	.quad	.altr
	.quad	.altr
	.quad	.kltr
	.quad	.kltr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.kltr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.kltr
	.quad	.kltr
	.quad	.altr
	.quad	.altr
	.quad	.kltr
	.quad	.altr
	.quad	.kltr
	.quad	.altr
	.quad	.kltr
	.quad	.kltr
	.quad	.altr
	.quad	.altr
	.quad	.altr
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
	.quad	.fail
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
