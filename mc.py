# an attempt at a compiler using shlex...
import shlex

block_handlers = {'if':        None,
                  'include':   None,
                  'while':     None,
                  'for':       None,
                  'def':       handle_functiondef,
                  'type':      None}



with open("test.m","r") as input:
  input = input.read()

  lexer = shlex.shlex(input)
  for token in lexer:
    print token
