	.section .mdebug.abiO32
	.previous
	.file	"ch4_1_mult.cpp"
	.text
	.globl	_Z9test_multv                   # -- Begin function _Z9test_multv
	.p2align	1
	.type	_Z9test_multv,@function
	.ent	_Z9test_multv                   # @_Z9test_multv
_Z9test_multv:
	.frame	$fp,8,$lr
	.mask 	0x00000000,0
	.set	noreorder
	.set	nomacro
# %bb.0:                                # %entry
	addiu	$sp, $sp, -8
	addiu	$2, $zero, 11
	st	$2, 4($sp)
	ld	$2, 4($sp)
	addiu	$2, $2, 1
	lui	$3, 10922
	ori	$3, $3, 43691
	mult	$2, $3
	mfhi	$3
	shr	$4, $3, 31
	shr	$3, $3, 1
	addu	$3, $3, $4
	addiu	$4, $zero, 12
	mul	$3, $3, $4
	subu	$2, $2, $3
	st	$2, 4($sp)
	ld	$2, 4($sp)
	addiu	$sp, $sp, 8
	ret	$lr
	.set	macro
	.set	reorder
	.end	_Z9test_multv
.Lfunc_end0:
	.size	_Z9test_multv, .Lfunc_end0-_Z9test_multv
                                        # -- End function
	.ident	"clang version 23.0.0git (https://github.com/Mountagha/llvm-project.git 669bbc6e0d9bab17d636f1ddac75bdf48fd66b51)"
	.section	".note.GNU-stack","",@progbits
