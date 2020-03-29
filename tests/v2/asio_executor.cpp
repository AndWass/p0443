#include <doctest/doctest.h>

#include <p0443_v2/schedule.hpp>
#include <p0443_v2/submit.hpp>
#include <p0443_v2/execute.hpp>
#include <p0443_v2/asio/executor.hpp>
#include <p0443_v2/asio/resolve.hpp>
#include <p0443_v2/make_receiver.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

TEST_CASE("asio: executor basic functionality")
{
    boost::asio::io_context io;

    bool fn_called = false;
    p0443_v2::execute(p0443_v2::asio::executor(io), [&]() {
        fn_called = true;
    });
    REQUIRE_FALSE(fn_called);
    io.run();
    REQUIRE(fn_called);
}

TEST_CASE("asio: resolver test")
{
    boost::asio::io_context io;
    p0443_v2::asio::resolve resolve(io, "localhost", "80");
    p0443_v2::submit(resolve,
        p0443_v2::value_channel([](boost::asio::ip::tcp::resolver::results_type results) {
            REQUIRE_FALSE(results.empty());
            for(auto res: results) {

                std::cout << res.endpoint().address().to_string() << "\n";
            }
        }));
        
    bool is_run = false;
    p0443_v2::submit(p0443_v2::schedule(resolve), p0443_v2::value_channel([&is_run] {
        is_run = true;
    }));
    REQUIRE_FALSE(is_run);
    io.run();
    REQUIRE(is_run);
}