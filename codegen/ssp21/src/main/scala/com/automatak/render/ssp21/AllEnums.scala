/**
 * License TBD
 */
package com.automatak.render.ssp21

import com.automatak.render.cpp.EnumConfig
import com.automatak.render.ssp21.enums._
import com.automatak.render.ssp21.enums.internal.{CryptoError, ParseResult}
import com.automatak.render.ssp21.enums.ssp21._


object AllEnums {

  def apply() : List[EnumConfig] = ssp21Enums ::: internalEnums

  private def ssp21Enums = List(
    CryptoFunction(),
    HashMode(),
    NonceMode(),
    DHMode(),
    SessionMode(),
    CertificateMode(),
    HandshakeError()
  ).map(x => EnumConfig(x, true, true))

  private def internalEnums = List(
    ParseResult(),
    CryptoError()
  ).map(x => EnumConfig(x, false, true))

}
