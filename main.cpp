#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>

template <typename Signature>
std::function<Signature> cast(void* f) {
  return reinterpret_cast<Signature*>(f);
}

int main(int argc, char* argv[]) {
  void (*getppid_addr)(int) = (void (*)(int))dlsym(RTLD_NEXT, "getppid");
  printf("&getppid(dlsym): %p\n", (void*)getppid_addr);

  void (*getpid_addr)(int) = (void (*)(int))dlsym(RTLD_NEXT, "getpid");
  printf("&getpid(dlsym): %p\n", (void*)getpid_addr);

  printf("&getppid(dlsym) - &getpid(dlsym): %p\n",
         (void*)((uintptr_t)getppid_addr - (uintptr_t)getpid_addr));

  void* getpid_plt_addr = reinterpret_cast<void*>(getpid);
  printf("getpid@plt address: %p\n", getpid_plt_addr);
  printf("getpid@plt asm: %p\n", (void*)(*(uint64_t*)getpid_plt_addr));
  constexpr int JMPQ_ASM_OPCODE_LEN      = 2;
  constexpr int JMPQ_ASM_INSTRUCTION_LEN = 6;
  uint32_t got_offset =
      *(uint32_t*)(((uint8_t*)getpid_plt_addr) + JMPQ_ASM_OPCODE_LEN);
  printf("offset(got - plt): %p\n", (void*)((uintptr_t)got_offset));
  uint64_t getpid_got_addr = (uint64_t)getpid_plt_addr +
                             JMPQ_ASM_INSTRUCTION_LEN + (uint64_t)got_offset;
  printf(".got.plt(getpid) adderss: %p\n", (void*)getpid_got_addr);

  printf("pre  call got asm(next instruction address of plt): %p\n",
         (void*)(*(uint64_t*)getpid_got_addr));
  // first call
  pid_t pid = getpid();
  printf("post call got asm(getpid(dlsym)): %p\n",
         (void*)(*(uint64_t*)getpid_got_addr));

  printf("&getppid - &getpid: %p\n",
         (void*)((uintptr_t)getppid_addr - (uintptr_t)getpid_addr));
  void* getppid_addr_from_got =
      (void*)((uintptr_t)(*(uint64_t*)getpid_got_addr) + (uintptr_t)0x40);
  pid_t ppid = cast<pid_t()>(getppid_addr_from_got)();

  std::cout << "pid: " << pid << std::endl;
  std::cout << "ppid(found from got): " << ppid << std::endl;
  pid_t ppid_answer = getppid();
  std::cout << "ppid(expected): " << ppid_answer << std::endl;
  return 0;
}
