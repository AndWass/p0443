#pragma once

struct test_receiver
{
    bool submitted = false;
    bool *shared_submitted = nullptr;
    void set_done() {}

    template<class T>
    void set_error(T) {}

    void set_value() {
        submitted = true;
        if(shared_submitted) {
            *shared_submitted = true;
        }
    }
};

struct test_sender
{
    template<class Receiver>
    void submit(Receiver &&rcv)
    {
        rcv.set_value();
    }
};