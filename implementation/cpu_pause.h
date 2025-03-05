#pragma once

// generated with Claude.ai, this stuff is kind of amazing
#if defined(__x86_64__) || defined(_M_X64)
// Intel x86/x64 architecture: use PAUSE instruction
#include <immintrin.h>

inline void cpu_pause() { _mm_pause(); }

#elif defined(__aarch64__) || defined(_M_ARM64)
// ARM64 architecture: use YIELD instruction
inline void cpu_pause()
{
#if defined(__GNUC__) || defined(__clang__)
    __asm__ volatile("yield" ::: "memory");
#elif defined(_MSC_VER)
    __yield();
#else
// Fallback for other compilers
#warning "No specific yield instruction for this ARM compiler"
#endif
}

#elif defined(__riscv) || defined(__riscv__)
// RISC-V architecture: use PAUSE instruction
// The implementation uses the standard RISC-V fence.i instruction, which can serve as a lightweight
// pause hint. The exact behavior might vary slightly between different RISC-V implementations, but
// it generally provides a similar function to pause/yield instructions on other architectures.
inline void cpu_pause()
{
#if __riscv_xlen == 32
    // 32-bit RISC-V
    __asm__ volatile(".insn i 0x0f, 0x0, x0, x0, 0x010" ::: "memory");
#elif __riscv_xlen == 64
    // 64-bit RISC-V
    __asm__ volatile(".insn i 0x0f, 0x0, x0, x0, 0x010" ::: "memory");
#else
#warning "Unsupported RISC-V architecture width"
#endif
}

#else
// Fallback for unsupported architectures
inline void cpu_pause()
{
// No-op or basic spinning
#warning "No architecture-specific pause instruction defined"
}
#endif
