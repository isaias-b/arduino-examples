for (
  p <- io.Source.stdin.getLines.sliding(2,1)
) {
  val List(a,b) = p
  if (a != b) println(b)
}
