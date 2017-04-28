# an attempt at a compiler using shlex...
import shlex
with open("test.m","r") as input:
  input = input.read()

  lexer = shlex.shlex(input)
  for token in lexer:
    print token
