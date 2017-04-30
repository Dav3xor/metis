#!/usr/bin/python

# an attempt at a compiler using shlex...
import shlex



def handle_functiondef(tokens):
  label = tokens.get_token() 
def handle_stmt(tokens):
  token = tokens.get_token()
  # handle return/returnnv/raise
  if token == "<":
    
def handle_block(tokens):
  block_handlers = {'if':        None,
                    'include':   None,
                    'while':     None,
                    'for':       None,
                    'def':       handle_functiondef,
                    'type':      None}
  token = tokens.get_token()

  if token in block_handlers:
    # do stuff
    return block_handlers[token](tokens)
  else:
    tokens.push_token(token)
    return None

with open("test.m","r") as input:
  input = input.read()

  lexer = shlex.shlex(input)
  for token in lexer:
    print token
