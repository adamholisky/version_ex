/*
General assembly helper routines 
*/
#define CR0_FPU_EM (1 << 2)
#define CR0_FPU_EXCP (1 << 5)
#define CR4_SSE_ENABLE (1 << 9)
#define CR4_SSE_EXCP (1 << 10)

.section .text
.global enable_sse
.type enable_sse, @function
enable_sse:
	xor %rax, %rax
	mov %cr0, %rax
	and $(~CR0_FPU_EM), %rax
	or $CR0_FPU_EXCP, %rax
	mov %rax, %cr0
	fninit

	xor %rax, %rax
	mov %cr4, %rax
	or $(CR4_SSE_ENABLE | CR4_SSE_EXCP), %rax
	mov %rax, %cr4
	ret
