package com.automatak.render.ssp21.messages.generators

import com.automatak.render._
import com.automatak.render.cpp._
import com.automatak.render.ssp21.messages.Struct
import com.automatak.render.ssp21.{Includes, WriteCppFiles}

case class StructGenerator(sf: Struct) extends WriteCppFiles {

  private def cppNamespace = "ssp21"
  private def headerName = sf.name + ".h"
  private def implName = sf.name + ".cpp"

  final override def mainClassName: String = sf.name

  /// These can be override in super classes
  def extraHeaderSignatures : Iterator[String] = Iterator.empty
  def extraHeaderConstants : Iterator[String] = Iterator.empty
  def extraImplFunctions : Iterator[String] = Iterator.empty
  def interfaces : String = "public IReadable, public IWritable, public IPrintable"

  final override def header(implicit indent: Indentation) : Iterator[String] = {

    def defaultConstructorSig = "%s();".format(sf.name).iter

    def readSig = "virtual ParseError read(openpal::RSlice& input) override;".iter
    def writeSig = "virtual FormatError write(openpal::WSlice& output) const override;".iter
    def printSig = "virtual void print(const char* name, IMessagePrinter& printer) const override;".iter

    def sizeBytes = sf.fixedSize match {
      case Some(size) => "static const uint32_t fixed_size_bytes = %s;".format(size).iter
      case None => "static const uint32_t min_size_bytes = %s;".format(sf.minSizeBytes).iter
    }

    def constants = sizeBytes ++ extraHeaderConstants

    def struct(implicit indent: Indentation) : Iterator[String] = {

      def fieldDefintions : Iterator[String] = sf.fields.map { f =>
        "%s %s;".format(f.cpp.cppType, f.name);
      }.toIterator

      "struct %s : %s".format(sf.name, interfaces).iter ++ bracketSemiColon {
          defaultConstructorSig ++
          space ++
          fullConstructorSig(false) ++
          space ++
          extraHeaderSignatures ++
          constants ++
          space ++
          fieldDefintions ++
          space ++
          readSig ++ space ++
          writeSig ++ space ++
          printSig
      }
    }

    def includes : Iterator[String] = {
      Includes.lines(
        List(
          Includes.readable,
          Includes.writable,
          Includes.msgPrinter
        ) ::: sf.fields.flatMap(f => f.cpp.includes.toList)
      )
    }

    def license = commented(LicenseHeader.lines)
    def content = struct

    license ++ space ++ includeGuards(sf.name)(includes ++ space ++ namespace(cppNamespace)(content))
  }

  final override def impl(implicit indent: Indentation) : Iterator[String] = {

    def license = commented(LicenseHeader.lines)

    def args = commas(sf.fields.map(_.name))

    def printArgs = commas(sf.fields.map(f => List(quoted(f.name), f.name)).flatten)
    
    def readFunc(implicit indent: Indentation) : Iterator[String] = {

      "ParseError %s::read(openpal::RSlice& input)".format(sf.name).iter ++ bracket {
        "return MessageParser::read_fields(".iter ++ indent {
          "input,".iter ++ args
        } ++ ");".iter
      }
    }

    def writeFunc(implicit indent: Indentation) : Iterator[String] = {

      "FormatError %s::write(openpal::WSlice& output) const".format(sf.name).iter ++ bracket {
        "return MessageFormatter::write_fields(".iter ++ indent {
          "output,".iter ++ args
        } ++ ");".iter
      }
    }

    def printFunc = "void %s::print(const char* name, IMessagePrinter& printer) const".format(sf.name).iter ++ bracket {
      "MessagePrinting::print_fields(".iter ++ indent {
        "printer,".iter ++ printArgs
      } ++ ");".iter
    }

    def defaultConstructorImpl(implicit indent: Indentation) : Iterator[String] = {

      val defaults = sf.fields.flatMap { f =>
        f.cpp.defaultValue.map(d => "%s(%s)".format(f.name, d))
      }

      if(defaults.isEmpty) {
        "%s::%s()".format(sf.name, sf.name).iter ++ bracketsOnly
      }
      else {

        def sig = "%s::%s() : ".format(sf.name, sf.name).iter

        sig ++ indent(commas(defaults)) ++ bracketsOnly
      }
    }

    def fullConstructorImpl(implicit indent: Indentation) : Iterator[String] = {

      def constructedFields = sf.fields.filter(_.cpp.initializeInFullConstructor)
      def names : List[String] = constructedFields.map(f => "%s(%s)".format(f.name, f.name))

      fullConstructorSig(true) ++ indent {
        commas(names)
      } ++ bracketsOnly
    }

    def funcs = {
        defaultConstructorImpl ++
        space ++
        fullConstructorImpl ++
        space ++
        readFunc ++
        space ++
        writeFunc ++
        space ++
        printFunc ++
        extraImplFunctions
    }

    def selfInclude = Includes.message(sf.name).line

    def includes = {
      Includes.lines(List(Includes.msgParser, Includes.msgFormatter, Includes.msgPrinting))
    }

    license ++ space ++ selfInclude ++ space ++ includes ++ space ++ namespace(cppNamespace)(funcs)

  }

  private def fullConstructorSig(impl: Boolean)(implicit indent: Indentation) : Iterator[String] = {

    val fields = sf.fields.filter(_.cpp.initializeInFullConstructor)

    val firstArgs : Iterator[String] = fields.dropRight(1).map(f => f.cpp.asArgument(f.name)+",").toIterator

    val last = fields.last
    val lastArgs : Iterator[String] = Iterator(last.cpp.asArgument(last.name))

    def funcName = if(impl) "%s::%s(".format(sf.name, sf.name).iter else (sf.name + "(").iter

    def terminator = if(impl) ") :".iter else ");".iter

    funcName ++ indent {
      firstArgs ++ lastArgs
    } ++ terminator
  }
}
