name := "javert"
version := "0.1"
organization := "edu.ucsb.cs.pllab"

fork := true

scalacOptions ++= Seq(
  "-deprecation",
  "-feature"
)

libraryDependencies ++= Seq(
  "org.scalatest" %% "scalatest" % "3.0.5" % "test",
  "org.javassist" % "javassist" % "3.24.1-GA",
  "org.slf4j" % "slf4j-log4j12" % "1.2",
  "org.json4s" %% "json4s-core" % "3.6.3",
  "org.json4s" %% "json4s-native" % "3.6.3"
)
