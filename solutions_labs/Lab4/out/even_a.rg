regular grammar for even_a
N = { E, O, S' }
T = { "a", "b" }
S = S'
P:
  S' -> "a" O | "b" E | "b" | epsilon
  E -> "a" O | "b" E | "b"
  O -> "a" E | "a" | "b" O
