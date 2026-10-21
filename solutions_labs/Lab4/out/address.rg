regular grammar for address
N = { Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, Q10, Q11, Q12, Q13, Q14, Q15, Q16, Q17, Q18 }
T = { digit, ".", "/" }
S = Q0
digit stands for one of: 0-9
P:
  Q0 -> digit Q1
  Q1 -> digit Q2 | "." Q4
  Q2 -> digit Q3 | "." Q4
  Q3 -> "." Q4
  Q4 -> digit Q5
  Q5 -> digit Q6 | "." Q8
  Q6 -> digit Q7 | "." Q8
  Q7 -> "." Q8
  Q8 -> digit Q9
  Q9 -> digit Q10 | "." Q12
  Q10 -> digit Q11 | "." Q12
  Q11 -> "." Q12
  Q12 -> digit Q13 | digit
  Q13 -> digit Q14 | digit | "/" Q16
  Q14 -> digit Q15 | digit | "/" Q16
  Q15 -> "/" Q16
  Q16 -> digit Q17 | digit
  Q17 -> digit Q18 | digit
