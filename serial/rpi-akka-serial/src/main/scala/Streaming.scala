import java.io.IOException

import akka.actor.ActorSystem
import akka.serial.{Parity, SerialSettings}
import akka.serial.stream.Serial
import akka.stream.{ActorMaterializer, ActorMaterializerSettings, Supervision}
import akka.stream.scaladsl.{Flow, Framing, Keep, Sink, Source}
import akka.util.ByteString

import scala.concurrent.Future
import scala.concurrent.duration.{FiniteDuration, MILLISECONDS}
import scala.io.StdIn
import scala.util.Try

object Streaming {

  final val Interval = FiniteDuration(200, MILLISECONDS)
  final val InitialDelay = FiniteDuration(200, MILLISECONDS)

  private val decider: Supervision.Decider = {
    case _: IOException => Supervision.Resume
    case _              => Supervision.Stop
  }
  implicit val system: ActorSystem = ActorSystem("terminal-stream")
  implicit val materializer: ActorMaterializer = ActorMaterializer(
    ActorMaterializerSettings(system).withSupervisionStrategy(decider)
  )

  def ask(label: String, default: String): String = {
    println(label + " [" + default.toString + "]:")
    val in = StdIn.readLine()
    if (in.isEmpty) default else in
  }

  def run(): Unit = {
    import system.dispatcher
    val port = ask("Device", Main.defaultPort)
    val baud = ask("Baud rate", "115200").toInt
    val cs = ask("Char size", "8").toInt
    val tsb = ask("Use two stop bits", "false").toBoolean
    val parity = Parity(ask("Parity (0=None, 1=Odd, 2=Even)", "0").toInt)
    val settings = SerialSettings(baud, cs, tsb, parity)

    val serial: Flow[ByteString, ByteString, Future[Serial.Connection]] =
      Serial().open(port, settings).log("CONNECTION")

    def printer[A]: Sink[A, _] = Sink.foreach[A] { data =>
      println(f"incoming << $data%50s")
    }

    val ticker: Source[ByteString, _] = Source
      .tick(InitialDelay, Interval, ())
      .map(x => ByteString(x.toString))
    val split: ByteString => Option[Data] = line =>
      line.decodeString("UTF-8").split("\t") match {
        case a => Some(Data(a.flatMap(s => Try(s.toInt).toOption)))
        case _ => None
    }
    val framing = Framing.delimiter(ByteString("\n"), 100)
    val format: Data => String = d =>
      d.elements.map(x => f"$x%4d").mkString(" ")

    val connection: Future[Serial.Connection] = ticker
      .viaMat(serial)(Keep.right)
      .via(framing)
      .map(split)
      .collect { case Some(x) => x }
      .map(format)
      .log("FORMAT")
      .to(printer)
      .run()

    connection map { conn =>
      println
      println("Connected to " + conn.port)
      StdIn.readLine("Press [ENTER] to exit\n\n")
    } recover {
      case err =>
        println("Cannot connect: " + err)
        err.printStackTrace()
    } andThen {
      case _ =>
        println("Terminating...")
        system.terminate().foreach(_ => println)
    }
  }
}
