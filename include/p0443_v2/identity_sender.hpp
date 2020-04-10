
//          Copyright Andreas Wass 2004 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <p0443_v2/connect.hpp>

namespace p0443_v2
{
template<class Sender>
struct identity_sender
{
    Sender next_;

    identity_sender(const Sender &sender): next_(sender) {}
    identity_sender(Sender&& sender): next_(std::move(sender)) {}

    template<class Receiver>
    auto connect(Receiver&& receiver) {
        return p0443_v2::connect(std::move(next_), std::forward<Receiver>(receiver));
    }
};
}