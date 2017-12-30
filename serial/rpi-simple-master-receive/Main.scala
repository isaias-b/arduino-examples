import com.pi4j.io.spi.SpiChannel
import com.pi4j.io.spi.SpiDevice
import com.pi4j.io.spi.SpiFactory
import com.pi4j.util.Console

import com.pi4j.io.serial._
import com.pi4j.util.CommandArgumentParser
import com.pi4j.util.Console

import java.io.IOException
import java.util.Date


import java.io.IOException

object Main {
  def main(args: Array[String]): Unit = {
    val console = new Console()
    val serial = SerialFactory.createInstance()
    val config = new SerialConfig()
      .device("/dev/ttyACM0")
      .baud(Baud._230400)
      .dataBits(DataBits._8)
      .parity(Parity.NONE)
      .stopBits(StopBits._1)
      .flowControl(FlowControl.NONE)

    serial.addListener(new SerialDataEventListener() {
      override def dataReceived(event: SerialDataEvent) = {
        console.print(event.getAsciiString())
      }
    })
    serial.open(config)

    console.title("<-- serial test -->", "testing serial interface using pi4j in scala")
    console.promptForExit()
    while(console.isRunning()) {}
    console.emptyLine()

    if(serial.isOpen) serial.close()
  }
}
