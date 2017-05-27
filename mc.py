#!/usr/bin/python

# an attempt at a compiler using shlex...
import shlex
import re

def peek(tokens):
  token = tokens.get_token()
  tokens.push_token(token)
  return token

class LabelStack(object):
  def __init__(self):
    self.stack = [{}]
  def push_context(self):
    self.stack.append({})
  def pop_context(self):
    self.stack.pop()

  def add_label(self,label,value):
    self.stack[-1][label] = value
  def find_label(self,label):
    for i in reversed(self.stack):
      if label in i:
        return i[label]
    raise Exception("syntax error: label does not exist - " + label)


labels = LabelStack()

atomic_types = {'string':1, 'bool':1, 
                'unsigned':1, 'integer':1, 
                'float':1, 'label':1, 
                'vector':1, 'matrix':1}

low_precedence = {'+':1,'-':1}

high_precedence = {'*':1, '/':1, '%':1, 'dot':1, 'cross':1}

def valid_label(token):
  if re.match('[a-zA-Z_][a-zA-Z0-9_]*', token):
    return token
  else:
    return None

def valid_integer(token):
  if re.match('[-+]?[0-9]+', token):
    return int(token)
  else:
    return None

def parse_number(tokens):
  cur      = peek(tokens)
  sign     = "positive"
  number   = "" 
  print "running parse_number ----------- "
  print "cur = " + cur
  if cur == "-":
    print "negative"
    number += cur
    num_type = "signed"
    sign = "negative"
    tokens.get_token()
    cur  = peek(tokens)

  if cur == ".":
    print "decimal 1"
    number += cur
    tokens.get_token()
    cur    = peek(tokens)

  if cur.isdigit():
    print "got number 1: " + cur
    number   += cur 
    tokens.get_token()
    cur = peek(tokens)

  if "." in number:
    print "decimal 2 returned " + number
    return number

  if cur == ".":
    print "decimal 3"
    number += cur
    tokens.get_token()
    cur  = peek(tokens)
  else:
    print "integer returned : " + number
    return number
 
  # read numbers past the . (we got a float...)
  if cur.isdigit():
    print "got number 2: " + cur
    number   += cur 
  

  return number


  print "finished ----------------------- "

  if not num_type:
    return None

  # ok, we have a number...

  # first burn the current token...
  tokens.get_token()


  cur = peek(tokens)
  if cur == ".":
    
    num_type = "float"

      

def valid_float(token):
  if re.match('[-+]?([0-9*[.])?[0-9]+', token):
    return float(token)
  else:
    return None

def handle_typeident(tokens):
  print "typeident"
  vartype = peek(tokens)
  if vartype not in atomic_types:
    return None
  else:
    tokens.get_token()
    label = valid_label(tokens.get_token())
    labels.add_label(label,1)
    return (vartype, label)

def handle_functiondef(tokens):
  print "functiondef"
  label = valid_label(tokens.get_token())
  args = []
  if not label:
    raise Exception("syntax error: invalid label - " + label)

  arg = handle_typeident(tokens) 
  if arg:
    args.append(arg)
    while True:
      comma = tokens.get_token()
      if comma == ',':
        arg = handle_typeident(tokens)
        if arg:
          args.append(arg)
      else:
        tokens.push_token(comma)
        break
  print args
  if handle_return_arrows(tokens) == '<-':
    returntype = tokens.get_token()
    if returntype not in atomic_types:
      raise Exception("syntax error: unknown return type - " + returntype)
    print returntype
  colon = tokens.get_token()
  if colon != ':':
    raise Exception("syntax error: function declaration does not end in ':' - " + returntype)
  while peek(tokens) != "fin":
    print "-->" + peek(tokens)
    handle_bs(tokens)
  print "end function"
   

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
    tokens.push_token(token)
    return None

def handle_group(tokens):
  print "group"
  handle_lexp(tokens);
  end = tokens.get_token()
  if end != ')':
   raise Exception("syntax error: grouped lexp doesn't end with ')'")

def handle_fcall(tokens):
  print "fcall"
  function_name = validate_label(tokens.get_token())
  token         = tokens.get_token()
  #while token # continue here tomorrow
  




def handle_ffcall(tokens):
  print "ffcall"
  handle_fcall(tokens);
  end = tokens.get_token()
  if end != '}':
   raise Exception("syntax error: ffcall doesn't end with '}'")

def handle_group(tokens):
  print "group"
  handle_lexp(tokens)
  end = tokens.get_token()
  if end != ")":
    raise Exception("syntax error: group does not end with ')'")

def handle_factor(tokens):
  factor_handlers = { '(': handle_group,
                      '{': None,
                      'true': None,
                      'false': None,
                      '"': None,
                      '|': None }    
  print "factor"
  number = parse_number(tokens)
  if number:
    print "number: " + number
  else:
    token = tokens.get_token()
    if token in factor_handlers:
      factor_handlers[token](tokens)
    elif valid_label(token):
      label = labels.find_label(token)
def handle_term(tokens):
  print "term"
  handle_factor(tokens)
  operator = tokens.get_token()
  if operator in high_precedence:
    print operator
    handle_factor(tokens)
  else:
    tokens.push_token(operator)
    
def handle_lexp(tokens):
  print "lexp"
  handle_term(tokens)
  operator = tokens.get_token()
  if operator in low_precedence:
    handle_term(tokens)
  else:
    tokens.push_token(operator)

def handle_return(tokens):
  print "return"
  handle_lexp(tokens)
def handle_trait(tokens):
  print "trait"

def handle_stmt(tokens):
  # statements start with a return arrow, a colon, a type signature, or a label
  print "stmt"
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

  elif token in atomic_types:
    atomic_types[token](tokens)

  elif token == ":":
    handle_trait(tokens)

  elif valid_label(token):
    # function call
    print "function call"
    
    function_name = token

    print function_name + " - ",

    while peek(tokens) != ".":
      arg = tokens.get_token()
      print arg + ", ",

    print "\n", 
  
  end = tokens.get_token()
  if end != ".":
    raise Exception("syntax error: stmt does not end in '.'")


    
block_handlers = {'if':        None,
                  'include':   None,
                  'while':     None,
                  'for':       None,
                  'def':       handle_functiondef,
                  'type':      None}

def handle_block(tokens):
  print "block"
  if peek(tokens) in block_handlers:
    # do stuff
    block_handlers[tokens.get_token()](tokens)
  
  #end = tokens.get_token()
  #if end != "fin":
  #  raise Exception("syntax error: block does not end in 'fin'")

def handle_bs(tokens):
  print "bs - " + peek(tokens)
  # shlex removes comments for us.
  token = peek(tokens)
  print "xxxx " + token
  if token in block_handlers:
    handle_block(tokens)
    tokens.get_token()
  else:
    handle_stmt(tokens)

with open("test.m","r") as input:
  input = input.read()

  lexer = shlex.shlex(input)
  #for token in lexer:
  #  print token
  while peek(lexer):
    handle_bs(lexer)
