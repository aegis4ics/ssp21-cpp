/**
 * License TBD
 */
package com.automatak

import java.nio.file.{Files, StandardOpenOption, Path}
import java.nio.charset.Charset

package object render {

    class RichString(s : String) {
      def iter : Iterator[String] = Iterator(s)
    }

    implicit def stringToRichString(s : String) : RichString = new RichString(s)

    // a custom flatten that adds a blank line in between blocks
    def spaced(i: Iterator[Iterator[String]]): Iterator[String] = {

      def map(iter: Iterator[String]): Iterator[String] = if(iter.hasNext) iter ++ space else iter

      i.foldLeft(Iterator.apply[String]())((sum, i) => sum ++ map(i))
    }

    // adds commas to all but the last item
    def mapAllButLast(items : Seq[String])(func: String => String) : Iterator[String] = {
      if(items.isEmpty) items.toIterator
      else (items.dropRight(1).map(func) ++ List(items.last)).toIterator
    }

    def commas(items : Seq[String]) : Iterator[String] = mapAllButLast(items)(_ + ",")

    def space: Iterator[String] = Iterator.apply("")

    def externC(inner: => Iterator[String]): Iterator[String] = {
      Iterator("#ifdef __cplusplus", """extern "C" {""", "#endif") ++ space ++
      inner ++ space ++
      Iterator("#ifdef __cplusplus", """}""", "#endif")
    }

    def commented(lines: Iterator[String]): Iterator[String] = {
      Iterator("//") ++ lines.map(l => "// " + l) ++ Iterator("//")
    }

    def bracketWithCap(indent: Indentation, cap: String)(inner: => Iterator[String]): Iterator[String] = {
      Iterator("{") ++
        indent(
          inner
        ) ++
        Iterator("}"+cap)
    }

    def bracket(inner: => Iterator[String])(implicit indent: Indentation): Iterator[String] = bracketWithCap(indent,"")(inner)
    def bracketsOnly : Iterator[String] = Iterator("{}")


    def bracketSemiColon(inner: => Iterator[String])(implicit indent: Indentation): Iterator[String] = bracketWithCap(indent,";")(inner)

    def bracketSemiColon(cap: String)(inner: => Iterator[String])(implicit indent: Indentation): Iterator[String] = bracketWithCap(indent," %s;".format(cap))(inner)

    implicit class RichStringList(list: List[String]) {
      def spaced : String = list.mkString(" ")
    }

    def delimited(delim: String, last: Option[String] = None)(s: Iterator[String]) : Iterator[String] = new Iterator[String] {
      def hasNext: Boolean = s.hasNext

      def next(): String = {
        val ret = s.next()
        if(s.hasNext) (ret + delim) else {
          last match {
            case Some(l) => ret + l
            case None => ret
          }
        }
      }
    }

    def commaDelimitedWithSemicolon(s: Iterator[String]) : Iterator[String] = delimited(",", Some(";"))(s)

    def commaDelimited(s: Iterator[String]) : Iterator[String] = delimited(",")(s)

    def writeTo(path: Path)(lines: Iterator[String]): Unit = {

      if(!lines.isEmpty)
      {
        if(!Files.exists(path.getParent)) Files.createDirectory(path.getParent)
        if(!Files.exists(path)) Files.createFile(path)

        val writer = Files.newBufferedWriter( path, Charset.defaultCharset, StandardOpenOption.TRUNCATE_EXISTING)

        def writeLine(s: String) = {
          writer.write(s)
          writer.write(System.lineSeparator)
        }

        try { lines.foreach(writeLine) }
        finally { writer.close() }
      }



    }
}
