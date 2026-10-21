regular grammar for number
N = { Q0, Q1, Q2 }
T = { "0", nonzero }
S = Q0
nonzero stands for one of: 1-9
P:
  Q0 -> "0" Q1 | "0" | nonzero Q2 | nonzero
  Q2 -> "0" Q2 | "0" | nonzero Q2 | nonzero
