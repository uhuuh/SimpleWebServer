/*
* Tencent is pleased to support the open source community by making Libco available.

* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

.globl co_swap
co_swap:
	leaq (%rsp),%rax
  movq %rax, 104(%rdi)
  movq %rbx, 96(%rdi)
  movq %rcx, 88(%rdi)
  movq %rdx, 80(%rdi)
  movq 0(%rax), %rax
  movq %rax, 72(%rdi) 
  movq %rsi, 64(%rdi)
  movq %rdi, 56(%rdi)
  movq %rbp, 48(%rdi)
  movq %r8, 40(%rdi)
  movq %r9, 32(%rdi)
  movq %r12, 24(%rdi)
  movq %r13, 16(%rdi)
  movq %r14, 8(%rdi)
  movq %r15, (%rdi)
  xorq %rax, %rax

  movq 48(%rsi), %rbp
  movq 104(%rsi), %rsp
  movq (%rsi), %r15
  movq 8(%rsi), %r14
  movq 16(%rsi), %r13
  movq 24(%rsi), %r12
  movq 32(%rsi), %r9
  movq 40(%rsi), %r8
  movq 56(%rsi), %rdi
  movq 80(%rsi), %rdx
  movq 88(%rsi), %rcx
  movq 96(%rsi), %rbx
  leaq 8(%rsp), %rsp
  pushq 72(%rsi)

  movq 64(%rsi), %rsi
	ret

.section .note.GNU-stack,"",@progbits
