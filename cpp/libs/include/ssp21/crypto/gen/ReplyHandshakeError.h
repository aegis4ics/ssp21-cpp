//
//  _   _         ______    _ _ _   _             _ _ _
// | \ | |       |  ____|  | (_) | (_)           | | | |
// |  \| | ___   | |__   __| |_| |_ _ _ __   __ _| | | |
// | . ` |/ _ \  |  __| / _` | | __| | '_ \ / _` | | | |
// | |\  | (_) | | |___| (_| | | |_| | | | | (_| |_|_|_|
// |_| \_|\___/  |______\__,_|_|\__|_|_| |_|\__, (_|_|_)
//                                           __/ |
//                                          |___/
// 
// This file is auto-generated. Do not edit manually
// 
// Licensed under the terms of the BSDv3 license
//

#ifndef SSP21_REPLYHANDSHAKEERROR_H
#define SSP21_REPLYHANDSHAKEERROR_H

#include "ssp21/crypto/gen/Function.h"
#include "ssp21/crypto/gen/HandshakeError.h"
#include "ssp21/crypto/IMessage.h"
#include "ssp21/crypto/EnumField.h"

namespace ssp21 {

struct ReplyHandshakeError final : public IMessage, private openpal::Uncopyable
{
    ReplyHandshakeError();

    ReplyHandshakeError(
        EnumField<HandshakeErrorSpec> handshake_error
    );

    virtual ParseError read(openpal::RSlice input) override;
    virtual FormatResult write(openpal::WSlice output) const override;
    virtual void print(IMessagePrinter& printer) const override;

    static const uint32_t fixed_size_bytes = 2;
    static const Function function = Function::reply_handshake_error;

    EnumField<HandshakeErrorSpec> handshake_error;

};

}

#endif