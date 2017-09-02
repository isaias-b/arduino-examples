import sbt._
import Process._
import Keys._

name := "spi-pi4j-test"

version := "1.0"

scalaVersion := "2.12.2"

libraryDependencies += "com.pi4j" % "pi4j-core" % "1.1"

assemblyJarName in assembly := "run.jar"

mainClass in assembly := Some("Main")
