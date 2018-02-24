import akka.serial.Serial

object Main {
  val mkrzero = "/dev/cu.usbmodem1411"
  val teensy32 = "/dev/cu.usbmodem3779351"
  val defaultPort = teensy32
  Serial.debug(true)
  def main(args: Array[String]): Unit = Streaming.run()
}
