#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib> // exit
#include <stdexcept>
#include <vector>

void ENSURE(bool cond, const char *format, ...)
{
  if(!cond) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);
    va_end(args);
    throw std::runtime_error("ENSURE failed");
  }
}

std::vector<uint8_t> loadFile(const char *path)
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

void probeIsobmff(uint8_t *data, size_t size)
{
  ENSURE(size >= 8, "ISOBMFF probing: not enough bytes (%d bytes available). Aborting.", (int)size);

  uint64_t boxSize = 0;

  for(auto i = 0; i < 4; ++i)
    boxSize = (boxSize << 8) + data[i];

  if(boxSize == 1) {
    boxSize = 0;

    for(auto i = 0; i < 4; ++i)
      boxSize = (boxSize << 8) + data[i];
  }

  ENSURE(boxSize == 0 || boxSize >= 8, "ISOBMFF probing: first box size too small (%d bytes). Aborting.", (int)boxSize);
  ENSURE(
    boxSize <= size, "ISOBMFF probing: first box size too big (%d bytes when file size is %d bytes). Aborting.",
    (int)boxSize, size);

  for(auto i = 4; i < 7; ++i)
    ENSURE(
      isalpha(data[i]) || isdigit(data[i]) || isspace(data[i]),
      "Box type is neither an alphanumerics nor a space (box[%d]=\"%c\" (%d)). Aborting.", i, (char)data[i],
      (int)data[i]);
}
