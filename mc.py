#!/usr/bin/python

# an attempt at a compiler using shlex...
import shlex

atomic_types = ['string', 'bool', 
                'unsigned', 'integer', 
                'float', 'label', 
                'vector', 'matrix']


def handle_functiondef(tokens):
  label = tokens.get_token() 
def handle_stmt(tokens):
  # statements start with a return arrow, a colon, a type signature, or a label
  stmt_handlers = {'<-':      None,
                   '<-!':     None,
                   ':':       None}
                   

  token = tokens.get_token()
  # handle return/returnnv/raise
  if token == "<":
    token += tokens.get_token()
    if token == "<-":
      israise = tokens.get_token()
      if israise == '!':
        token += israise
      else:
        tokens.push_token(israise)
    else:
      raise Exception("syntax error: statement start with < but not <-")
      

    
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
