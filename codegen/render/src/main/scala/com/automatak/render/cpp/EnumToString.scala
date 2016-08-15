/**
 * License TBD
 */
package com.automatak.render.cpp

import com.automatak.render._

object EnumToString extends HeaderImplModelRender[EnumModel] {

  def impl: ModelRenderer[EnumModel]  = ImplRender
  def header: ModelRenderer[EnumModel]  = HeaderRender

  private def signature(name: String) : String = List(cString, List(name.toLowerCase,"_to_string(", name," arg)").mkString).mkString(" ")

  private object HeaderRender extends ModelRenderer[EnumModel] {
    def render(em: EnumModel)(implicit indent: Indentation) : Iterator[String] = Iterator(signature(em.name)+";")
  }

  private object ImplRender extends ModelRenderer[EnumModel] {

    def render(em: EnumModel)(implicit indent: Indentation) : Iterator[String] = {

      def header = Iterator(signature(em.name))
      def smr = new SwitchModelRenderer[EnumValue](ev => em.qualified(ev))(ev => quoted(ev.displayName))
      def getDefault : EnumValue = em.defaultValue.getOrElse(EnumValue("undefined",0))
      def switch = smr.render(em.nonDefaultValues, getDefault)

      header ++ bracket {
        switch
      }
    }
  }
}


