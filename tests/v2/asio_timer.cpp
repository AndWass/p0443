#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <doctest/doctest.h>

#include <p0443_v2/asio/timer.hpp>
#include <p0443_v2/let.hpp>
#include <p0443_v2/make_receiver.hpp>
#include <p0443_v2/submit.hpp>

#include <boost/asio.hpp>
#include <ratio>

TEST_CASE("asio timer: duration timer timeout") {
    boost::asio::io_context io;
    boost::asio::steady_timer timer(io);
    bool timeout = false;
    auto timer_op =
        p0443_v2::connect(p0443_v2::asio::timer::wait_for(timer, std::chrono::milliseconds(100)),
                          p0443_v2::value_channel([&]() { timeout = true; }));
    p0443_v2::start(timer_op);
    REQUIRE(!timeout);
    auto now = std::chrono::steady_clock::now();
    io.run();
    auto now2 = std::chrono::steady_clock::now();
    REQUIRE(timeout);
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now).count();
    REQUIRE(diff > 50);
}

TEST_CASE("asio timer: timepoint timer timeout") {
    boost::asio::io_context io;
    boost::asio::steady_timer timer(io);
    bool timeout = false;
    auto now = std::chrono::steady_clock::now();

    auto timer_op =
        p0443_v2::connect(p0443_v2::asio::timer::wait_until(timer, now + std::chrono::milliseconds(100)),
                          p0443_v2::value_channel([&]() { timeout = true; }));
    p0443_v2::start(timer_op);
    REQUIRE(!timeout);
    io.run();
    auto now2 = std::chrono::steady_clock::now();
    REQUIRE(timeout);
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now).count();
    REQUIRE(diff > 50);
}