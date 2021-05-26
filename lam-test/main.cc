#include <cassert>
#include <cstdint>
#include <cstdio>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <unistd.h>

static int max_tag_bits() {
  if (prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0) < 0) return 0;

  int ret;
  int err = prctl(PR_GET_TAGGED_ADDR_CTRL, &ret, 0, 0, 0);
  assert(err >= 0);
  return ret;
}

int main(int argc, char* argv[]) {
  char x = 'X';
  assert(x == 'X');

  int tag_shift = 0;
  int tag_bits = max_tag_bits();
  fprintf(stderr, "Max tag bits: %d\n", tag_bits);
  assert(tag_bits > 6);
  tag_bits = 6;

  // Enable LAM.
  int ret = prctl(PR_SET_TAGGED_ADDR_CTRL, PR_TAGGED_ADDR_ENABLE, &tag_bits,
                  &tag_shift, 0);
  assert(ret == 0);
  fprintf(stderr, "Tag bits: %d\n", tag_bits);
  fprintf(stderr, "Tag shift: %d\n", tag_shift);

  // Access a tagged pointer.
  uintptr_t tag = (uintptr_t{1} << tag_bits) - 1;
  char *tagged_x = reinterpret_cast<char *>(
      reinterpret_cast<uintptr_t>(&x) | (tag << tag_shift));
  fprintf(stderr, "Dereferencing tagged pointer: %p\n", tagged_x);
  assert(*tagged_x == 'X');

  // Pass tagged pointer to syscall.
  fprintf(stderr, "*tagged_x: ");
  ssize_t bytes_written = write(STDERR_FILENO, tagged_x, sizeof(*tagged_x));
  fprintf(stderr, "\n");
  assert(bytes_written > 0);

  return 0;
}
