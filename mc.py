# an attempt at a compiler using shlex...
import shlex

block_handlers = {'if':        handle_if,
                  'include':   handle_include,
                  'while':     handle_while,
                  'for':       handle_for,
                  'def':       handle_functiondef,
                  'type':      handle_typedef}



with open("test.m","r") as input:
  input = input.read()

  lexer = shlex.shlex(input)
  for token in lexer:
    print token
