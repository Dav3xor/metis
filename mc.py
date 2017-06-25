#!/usr/bin/python

# an attempt at a compiler using shlex...
import shlex
import re

def peek(tokens):
  token = tokens.get_token()
  tokens.push_token(token)
  return token

class SyntaxError(Exception):
  def __init__(self, message, tokens=None):
    if tokens:
      print "Syntax Error: " + message + " line - " + str(tokens.lineno)
    else:
      print "Syntax Error: " + message


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
    raise SyntaxError(" label does not exist - " + label)


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
  if cur == "-":
    number += cur
    num_type = "signed"
    sign = "negative"
    tokens.get_token()
    cur  = peek(tokens)

  if cur == ".":
    number += cur
    tokens.get_token()
    cur    = peek(tokens)

  if cur.isdigit():
    number   += cur 
    tokens.get_token()
    cur = peek(tokens)

  if "." in number:
    return number

  if cur == ".":
    number += cur
    tokens.get_token()
    cur  = peek(tokens)
  else:
    return number
 
  # read numbers past the . (we got a float...)
  if cur.isdigit():
    number   += cur 
    tokens.get_token() 

  return number



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
    raise SyntaxError("invalid label - " + label, tokens)

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
      raise SyntaxError("unknown return type - " + returntype, tokens)
    print returntype
  colon = tokens.get_token()
  if colon != ':':
    raise SyntaxError("function declaration does not end in ':' - " + returntype, tokens)
  while peek(tokens) != "fin":
    handle_bs(tokens)
  print "end function"
   
def handle_assignment_operator(tokens):
  "handling assignment..."
  colon = peek(tokens)
  if colon != ':':
    return False
  tokens.get_token()
  equals = peek(tokens)
  if equals != '=':
    raise SyntaxError("= should follow :, not " + equals, tokens)
  tokens.get_token()
  return True

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
      raise SyntaxError("statement start with < but not <-", tokens)
    return token
  else:
    tokens.push_token(token)
    return None

def handle_group(tokens):
  print "group"
  handle_lexp(tokens);
  end = tokens.get_token()
  if end != ')':
   raise SyntaxError("grouped lexp doesn't end with ')'", tokens)

def handle_fcall(tokens):
  print "fcall"
  function_name = tokens.get_token()
  print "function = " + function_name
  while handle_lexp(tokens):
    comma = peek(tokens)
    if comma != ",":
      break
    else:
      tokens.get_token() # consume the comma
  return True 




def handle_ffcall(tokens):
  print "ffcall"
  handle_fcall(tokens);
  end = tokens.get_token()
  if end != '}':
   raise SyntaxError("ffcall doesn't end with '}'", tokens)

def handle_group(tokens):
  print "group"
  handle_lexp(tokens)
  end = tokens.get_token()
  if end != ")":
    raise SyntaxError("group does not end with ')'", tokens)

def handle_vector(tokens):
  while peek(tokens) != '|':
    handle_lexp(tokens)
  # consume the end |
  tokens.get_token()

def handle_matrix(tokens):
  while True:
    handle_vector(tokens)
    if peek(tokens) != '|':
      break
    # consume the next |
    tokens.get_token()

def handle_factor(tokens):
  factor_handlers = { '(': handle_group,
                      '{': handle_ffcall,
                      'true': None,
                      'false': None,
                      '|': handle_matrix }    
  print "factor"
  print peek(tokens)
  number = parse_number(tokens)
  if number:
    print "number: " + number
    if number[-1] == '.':
      tokens.push_token('.')
    return True
  else:
    token = peek(tokens)
    print token
    if token[0] == '"':
      print token[1:-1]
      tokens.get_token()
      return True
    elif token in factor_handlers:
      tokens.get_token()
      factor_handlers[token](tokens)
      return True
    elif valid_label(token):
      tokens.get_token()
      label = labels.find_label(token)
      return True
  return False

def handle_term(tokens):
  print "term"
  print peek(tokens)
  if handle_factor(tokens):
    operator = peek(tokens)
    while operator in high_precedence:
      print operator
      tokens.get_token()
      handle_factor(tokens)
      operator = peek(tokens)
    print peek(tokens) 
    return True
  return False

def handle_lexp(tokens):
  print "lexp"
  print peek(tokens)
  if handle_term(tokens):
    operator = peek(tokens)
    while operator in low_precedence:
      tokens.get_token()
      handle_term(tokens)
      operator = peek(tokens)
    return True
  return False

def handle_return(tokens):
  print "return"
  print peek(tokens)
  handle_lexp(tokens)

def handle_trait(tokens):
  print "trait"
  return True

def handle_assignment(tokens):
  token   = tokens.get_token()
  vartype = None
  varname = None

  if token in atomic_types:
    vartype = token
    varname = tokens.get_token()
  

  if handle_assignment_operator(tokens):
    if not valid_label(varname):
      raise SyntaxError("variable name: " + varname + " is not a valid label")
    return handle_lexp(tokens)
  else:
    if varname:
      tokens.push_token(varname)
    if vartype:
      tokens.push_token(vartype)
    return False
      
    
def handle_stmt(tokens):
  # statements start with a return arrow, a colon, a type signature, or a label
  print "stmt"

  retval = False

  stmt_handlers = {'<-':      handle_return,
                   '<-!':     None,
                   ':':       None}
                   

  # handle return/returnnv/raise
  token = handle_return_arrows(tokens)
  print "---" + peek(tokens) + "---" + str(token )
  if token in stmt_handlers:
    print "handler"
    #tokens.get_token()
    stmt_handlers[token](tokens)
    retval = True

  # else, the next token is our thing...
  elif not token:
    token = peek(tokens)
    if token == ":":
      print "trait"
      tokens.get_token()
      retval = handle_trait(tokens)
    else:
      print "stmt assignment"
      retval = handle_assignment(tokens)
      if not retval:
        retval = handle_fcall(tokens)
  end = tokens.get_token()
  if end != ".":
    raise SyntaxError("stmt does not end in '.' (got '"+end+"' instead)", tokens)
  return retval

def handle_include(tokens):
  dash = peek(tokens)
  while dash == '-':
    tokens.get_token()
    print "file: " + tokens.get_token()
    dash = peek(tokens)
  print peek(tokens)

def handle_exp(tokens):
  handle_lexp(tokens)
  print "comparator = " + tokens.get_token()
  handle_lexp(tokens)
  print "end exp" + peek(tokens)
def handle_if(tokens):
  print "if"
  handle_exp(tokens)
  while peek(tokens) not in ['else','fin']:
    handle_bs(tokens)
  token = peek(tokens)
  while token == "else":
    print "else"
    token = tokens.get_token()
    token = peek(tokens)
    if token == "if":
      print "if after else"
      tokens.get_token()
      handle_exp(tokens) 
    while peek(tokens) not in ['else','fin']:
      handle_bs(tokens)


def handle_while(tokens):
  print "while"
  handle_exp(tokens)
  while peek(tokens) != "fin":
    handle_bs(tokens)

def handle_for(tokens):
  if peek(tokens) != ';':
    # we have an initial condition
    handle_assignment(tokens)
def handle_typedef(tokens):
  name = tokens.get_token()
  typedef_handlers = {'def':      handle_functiondef,
                      'trait':    None}
                   
  while peek(tokens) != "fin":
    if handle_typeident(tokens):
      dot = tokens.get_token()
      if dot != ".":
        raise SyntaxError("member declaration in type doesn't end with '.'", tokens)
    elif peek(tokens) in typedef_handlers:
      typedef_handlers[tokens.get_token()](tokens) 
      if peek(tokens) != "fin":
        raise SyntaxError("block in type declaration doesn't end with 'fin'")
      else:
        tokens.get_token()
  print "end typedef"
block_handlers = {'if':        handle_if,
                  'include':   handle_include,
                  'while':     handle_while,
                  'for':       handle_for,
                  'def':       handle_functiondef,
                  'type':      handle_typedef}

def handle_block(tokens):
  print "block"
  if peek(tokens) in block_handlers:
    # do stuff
    block_handlers[tokens.get_token()](tokens)
    return True
  else:
    return False
  
  #end = tokens.get_token()
  #if end != "fin":
  #  raise SyntaxError("block does not end in 'fin'", tokens)

def handle_bs(tokens):
  # shlex removes comments for us.
  token = peek(tokens)
  print "bs - " + token
  if handle_block(tokens):
    tokens.get_token()
    print "end block"
    return True
  elif handle_stmt(tokens):
    print "end stmt"
    return True
  else:
    print "bad bs?"
    return False






with open("test.m","r") as input:
  input = input.read()

  lexer = shlex.shlex(input)
  #for token in lexer:
  #  print token
  while peek(lexer):
    handle_bs(lexer)
