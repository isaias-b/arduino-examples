import java.time.{Duration, Instant}

import akka.actor.{Actor, ActorLogging, ActorSystem, Props}
import akka.io.IO
import akka.serial.{AccessDeniedException, Parity, Serial, SerialSettings}

object Messaging {
  private implicit val system: ActorSystem = ActorSystem("terminal-stream")
  def run(): Unit = system.actorOf(Props(new Messaging()), "messaging")
}

class Messaging extends Actor with ActorLogging {
  import Messaging._
  private val endTime = Instant.now().plus(Duration.ofSeconds(15))
  private val port = Main.defaultPort
  private val settings = SerialSettings(
    baud = 115200,
    characterSize = 8,
    twoStopBits = false,
    parity = Parity.None
  )
  IO(Serial) ! Serial.Open(port, settings)

  override def receive: Receive = opening

  def onError: Receive = {
    case Serial.CommandFailed(_: Serial.Open, _: AccessDeniedException) =>
      println("You're not allowed to open that port - now terminating")
      system.terminate()
    case Serial.CommandFailed(_: Serial.Open, reason) =>
      println(s"Could not open port for some other reason: ${reason.getMessage} - now terminating")
      system.terminate()
  }

  def opening: Receive = onError orElse  {
    case Serial.Opened(_) =>
      println("Opened port - now communicating")
      context.become(communicating)
  }

  def communicating: Receive = onError orElse {
    case Serial.Received(data) =>
      print(s"\rReceived $data")
      if (Instant.now().isAfter(endTime)) {
        println()
        println(s"Limit reached - now closing")
        val operator = sender
        context.become(closing)
        operator ! Serial.Close
      }
  }

  def closing: Receive = onError orElse  {
    case Serial.Closed =>
      println("Port closed - now terminating")
      system.terminate()
  }
}
