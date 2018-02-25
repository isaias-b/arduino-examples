implicit class RichString(s: String) {
  def **(n:Int = 2) = s.flatMap(c => c.toString * n)
  def mirror = s + s.reverse
  def delay = s.tail + s.head
  def asInts = s.map(_ - '0').map(_.toInt)
  def xor(t: String) = (s.asInts xor t.asInts).asString
  def and(t: String) = (s.asInts and t.asInts).asString
  def inv = s.asInts.inv.asString
  def mkFormat = s.map {
    case '1' => '#'
    case '0' => '_'
  }
  def printExpr(expr: String) = 
    println(f"$expr%-38s  : $mkFormat")
  def printExprAs(expr: String, label: String) = 
    println(f"$expr%-30s $label%8s : $mkFormat")
}

implicit class RichInts(is: IndexedSeq[Int]) {
  def xor(bs: IndexedSeq[Int]) = is.zip(bs).map {
    case (i1,i2) => i1 ^ i2
  }
  def and(bs: IndexedSeq[Int]) = is.zip(bs).map {
    case (i1,i2) => i1 & i2
  }
  def inv = is.map(_ - 1).map(Math.abs)
  def asString = is.map(_.toString).mkString
}

val a       = "00110011110101010110101" ** 3 mirror
val b       = "01100110001010101110100" ** 3 mirror
val ad      = a.delay
val bd      = b.delay
val add     = ad.delay
val bdd     = bd.delay
val step    = a xor b
val dir     = a xor bd
val da      = a xor ad
val db      = b xor bd
val err     = da and db
val count   = da xor db


            println("INPUTS")
a           printExpr("a")
b           printExpr("b")
            println
            println("OUTPUTS")
count       printExprAs("a^a' ^ b^b'", "count")
dir         printExprAs("a^b'", "dir")
err         printExprAs("a^a' & b^b'", "err")
            println
            println("BUFFERS")
ad          printExpr("a'")
bd          printExpr("b'")
            println
            println("STEP")
a           printExpr("a")
b           printExpr("b")
step        printExprAs("a^b", "step")
            println
a           printExpr("a")
bd          printExpr("b'")
dir         printExprAs("a^b'", "dir")
            println
a           printExpr("a")
ad          printExpr("a'")
count       printExprAs("a^a' ^ b^b'", "count")
err         printExprAs("a^a' & b^b'", "err")
b           printExpr("b")
bd          printExpr("b'")
            println
