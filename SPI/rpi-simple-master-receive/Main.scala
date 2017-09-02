import com.pi4j.io.spi.SpiChannel
import com.pi4j.io.spi.SpiDevice
import com.pi4j.io.spi.SpiFactory
import com.pi4j.util.Console

import java.io.IOException

object Main {
  private val samplingDelay = 20 // milliseconds
  private val nbytes = 10
  private val skippedBytes = 2
  private val printSent = false
  private val console = new Console()
  private var spi: SpiDevice = null
  private val channel = SpiChannel.CS1
  // 1000000 spi speed at 1 MHz (default)
  private val speed = 2000000
  private val data: Array[Byte] = Array.tabulate(nbytes)(0xf0 + _).map(_.toByte)
  private def formatBytes(arr: Array[Byte]) = Word.fromBytes(arr)
    .map(_.swap.toDecFormat).mkString(" ")
  //private def formatBytes(arr: Array[Byte]) = arr.map(b => f"$b%02x").mkString(" ")
  def main(args: Array[String]): Unit = {
    console.title("<-- slid spi test -->", "testing spi interface using pi4j in scala")
    console.promptForExit()
    spi = SpiFactory.getInstance(channel, speed, SpiDevice.DEFAULT_SPI_MODE)
    while(console.isRunning()) {
      if(printSent) {
        console.print(formatBytes(data))
        console.print(" -> ")
      }
      val result = spi.write(data :_*)
      console.println(formatBytes(result.drop(skippedBytes)))
      Thread.sleep(samplingDelay)
    }
    console.emptyLine()
  }
}

class Word(val msb: Byte, val lsb: Byte) {
  import Word._
  override def toString() = toHexFormat
  def toInt = byte2int(lsb) | (byte2int(msb) << 8)
  def toDecFormat = f"$toInt%5d"
  def toHexFormat = f"0x$msb%02x$lsb%02x"
  def swap = Word(lsb, msb)
}

object Word {
  private def mask(i: Int)  : Int  = (i & 0x0FF).toInt
  private def mask(b: Byte) : Byte = (b & 0x0FF).toByte
  private def int2byte(i: Int) = mask(i).toByte
  private def byte2int(b: Byte) = mask(b.toInt)
  def apply(msb: Byte, lsb: Byte) = new Word(msb, lsb)
  def apply(i: Int) = new Word(int2byte(i >> 8), int2byte(i))
  def fromBytes(bytes: Array[Byte]) = bytes.grouped(2).flatMap {
    case Array(msb, lsb) => Some(Word(msb, lsb))
    case _ => None
  }
}
