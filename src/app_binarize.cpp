// standalone file binarizer
#include <cstdio>
#include <cassert>
#include <cctype>
#include <cstdint>

// -----------------------------------------------------------------------------
// input

int frontChar;

bool empty()
{
  return frontChar == EOF;
}

char front()
{
  return frontChar;
}

void popFront()
{
  assert(!empty());
  frontChar = getc(stdin);
}

// -----------------------------------------------------------------------------
// output
void send(uint8_t byte)
{
  printf("%c", byte);
}

// -----------------------------------------------------------------------------
struct ParseError
{
};

void expect(char c)
{
  if(front() != c)
    throw ParseError();

  popFront();
}

void processStringLiteral()
{
  expect('"');

  while(front() != '"' && !empty())
  {
    send(front());
    popFront();
  }

  expect('"');
}

void processBinaryByte()
{
  int value = 0;

  for(int i = 0; i < 8; ++i)
  {
    if(front() != '0' && front() != '1')
      throw ParseError();

    value *= 2;
    value += front() - '0';
    popFront();
  }

  send(value);
}

int main()
{
  popFront();

  while(!empty())
  {
    switch(front())
    {
    case '\n':
    case ' ':
    case '\t':
      // whitespace: skip
      popFront();
      break;

    case '"':
      processStringLiteral();
      break;

    case '0':
    case '1':
      processBinaryByte();
      break;
    }
  }

  return 0;
}

