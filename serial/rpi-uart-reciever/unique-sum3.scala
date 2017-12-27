for (
  p <- io.Source.stdin.getLines.map(_.toInt).sliding(3,1)
) {
  val List(a,b,c) = p
  if (a + b != b + c) println(b)
}
