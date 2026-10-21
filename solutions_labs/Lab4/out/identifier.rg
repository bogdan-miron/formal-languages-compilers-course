regular grammar for identifier
N = { Q0, Q1 }
T = { letter, digit, "_" }
S = Q0
letter stands for one of: A-Z a-z
digit stands for one of: 0-9
P:
  Q0 -> letter Q1 | letter
  Q1 -> letter Q1 | letter | digit Q1 | digit | "_" Q1 | "_"
