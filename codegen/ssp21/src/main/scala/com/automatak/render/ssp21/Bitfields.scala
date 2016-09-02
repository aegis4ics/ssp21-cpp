/**
  * License TBD
  */
package com.automatak.render.ssp21

import com.automatak.render.ssp21.messages._


object Bitfields {

  val payloadFlags: Bitfield = Bitfield(
    "payload_flags", "PayloadFlags",
    Bit("fir", true),
    Bit("fin", true)
  )

  def files: List[WriteCppFiles] = List(
    payloadFlags
  ).map(_.files)

}
