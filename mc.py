#!/usr/bin/python

# an attempt at a compiler using shlex...
import shlex

atomic_types = {'string':1, 'bool':1, 
                'unsigned':1, 'integer':1, 
                'float':1, 'label':1, 
                'vector':1, 'matrix':1}

low_precedence = {'+':1,'-':1}

high_precedence = {'*':1, '/':1, '%':1, 'dot':1, 'cross':1}

def valid_label(token):
  if re.match('[a-zA-Z_][a-zA-Z0-9_]', token):
    return token
  else:
    return None

def handle_functiondef(tokens):
  label = tokens.get_token()

def handle_return_arrows(tokens):
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
    return token
  else:
    tokens.push_token(israise)
    return None

def handle_group(tokens):
  handle_lexp(tokens);
  end = tokens.get_token()
  if end != ')':
   raise Exception("syntax error: grouped lexp doesn't end with ')'")

def handle_fcall(tokens):
  function_name = validate_label(tokens.get_token()
  token         = tokens.get_token()
  while token # continue here tomorrow




def handle_ffcall(tokens):
  handle_fcall(tokens);
  end = tokens.get_token()
  if end != '}':
   raise Exception("syntax error: ffcall doesn't end with '}'")


def handle_factor(tokens):
  factor_handlers = { '(': handle_group,
                      '{': handle_ffcall,
                      'true': handle_bool_true,
                      'false': handle_bool_false,
                      '"': handle_string,
                      '|': handle_matrix }                      
  token = tokens.get_token()
  if token in factor_handlers:
    factor_handlers[token](tokens)
  elif valid_label(token):

def handle_term(tokens):
  handle_factor(tokens)
  operator = tokens.get_token()
  if operator in high_precedence:
    handle_factor(tokens)
  else:
    tokens.push_token(operator)
    
def handle_lexp(tokens):
  handle_term(tokens)
  operator = tokens.get_token()
  if operator in low_precedence:
    handle_term(tokens)
  else:
    tokens.push_token(operator)

def handle_return(tokens):
  handle_lexp(tokens)

def handle_stmt(tokens):
  # statements start with a return arrow, a colon, a type signature, or a label
  stmt_handlers = {'<-':      handle_return,
                   '<-!':     None,
                   ':':       None}
                   

  # handle return/returnnv/raise
  token = handle_return_arrows(tokens)

  # else, the next token is our thing...
  if not token:
    token = tokens.get_token()

  if token in stmt_handlers:
    stmt_handlers[token](tokens)

  if token in atomic_types:
    atomic_types[token](tokens)

  if valid_label(token):
    # do label stuff here, remove print
    print "label"
      

    
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
