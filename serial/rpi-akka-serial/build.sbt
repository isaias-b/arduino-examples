import sbt.Keys._
import sbt._

name := "rpi"

version := "1.0"

scalaVersion := "2.12.4"
libraryDependencies += "com.typesafe" % "config" % "1.3.1"
libraryDependencies += "ch.jodersky" % "akka-serial-native" % "4.1.2"
libraryDependencies += "ch.jodersky" %% "akka-serial-core" % "4.1.2"
libraryDependencies += "ch.jodersky" %% "akka-serial-stream" % "4.1.2"
libraryDependencies += "ch.jodersky" %% "akka-serial-sync" % "4.1.2"

