#include <cstdint>
#include <cstdio>
#include <cstdlib> // exit
#include <cstdarg>
#include <vector>

void ENSURE(bool cond, const char* format, ...)
{
  if(!cond)
  {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);
    va_end(args);
    exit(1);
  }
}

std::vector<uint8_t> loadFile(const char* path)
{
  auto fp = fopen(path, "rb");
  ENSURE(fp, "Can't open '%s' for reading", path);

  std::vector<uint8_t> buf(100 * 1024 * 1024);
  auto const size = fread(buf.data(), 1, buf.size(), fp);
  fclose(fp);
  buf.resize(size);
  buf.shrink_to_fit();
  return buf;
}

