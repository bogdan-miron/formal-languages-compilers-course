regular grammar for string
N = { Q0, Q1, Q2 }
T = { '"', letter, digit, punct }
S = Q0
letter stands for one of: A-Z a-z
digit stands for one of: 0-9
punct stands for one of: space ! #-/ :-@ [ ]-_ {-~
P:
  Q0 -> '"' Q1
  Q1 -> letter Q1 | digit Q1 | punct Q1 | '"' Q2 | '"'
