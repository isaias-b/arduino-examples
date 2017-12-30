import sbt.Keys._
import sbt._

name := "serial-pi4j-test"

version := "1.0"

scalaVersion := "2.12.2"

libraryDependencies += "com.pi4j" % "pi4j-core" % "1.1"

assemblyJarName in assembly := "run.jar"

mainClass in assembly := Some("Main")
