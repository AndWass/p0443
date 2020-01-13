Preface
=======

This is my understanding of
[P0443 - A Unified Executors Proposal for C++](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0443r11.html).
This comes from reading the proposal, various slack 
conversations and the related
[P1897 - Towards C++23 executors: A proposal for an initial set of algorithms](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1897r1.html)
paper.

The text will undoubtly contain errors and misunderstandings on 
my part but I think I have managed to get a basic grasp of how 
it all is supposed to work.

Senders are the functions
=========================

In the executors proposal, senders are the functions that will
be run. Unlike normal functions, senders don't just have a single
"return" value, they have (up to) 3: `value`, `error` and `done`.
And they don't have to return their valules immediately.

Conceptually a sender can be seen as a function with the 
signature `result_channels sender_function();`.

In the executor proposal these are called _channels_, and are not 
returned from the receiver in the classical sense.

The `value` channel can contain any number of values (depending 
on what the sender does). `error` always sends a single value 
(std::exception_ptr by default) and
`done` never sends values.

Senders will start its action by being submitted together with a 
receiver. This is done by calling
`exec::submit(sender, receiver)` with a sender and a 
receiver. `exec::submit` is a customization-point that will 
either call `sender.submit(receiver)` [1] if such a member 
function is found, call `submit(sender, receiver)` [2] if an 
appropriate function is found via ADL (excluding 
`exec::submit`), or call
`exec::execute(sender, as_invocable(receiver))` [3] (if this 
is valid) which is yet another customization point.

If neither [1], [2] nor [3] is valid then the call to 
`exec::submit` results in a compile-error.

For now we will focus o cases [1] and [2] as they are what 
allows the sender (or function) to actually start execution. 

Only through `exec::submit` will the `sender` know where 
its results will go: the receiver of the function results is the 
`receiver` object (TADA!). When the sender has performed its task(s)
it must, _at some point_, call **one** of 
`exec::set_value(receiver, values...)`,
`exec::set_done()` or `exec::set_error(error)`.

The only exception to this is if `set_value` or 
`exec::set_error` exits via an exception; the sender may 
then call `exec::set_error` or `exec::set_done`. In 
this case `exec::set_error` must be called with an 
`std::exception_ptr`.

Take note of _at some point_; the sender doesn't have to call 
any of the `exec::set_*`functions directly from its submit 
method. It is perfectly legal for it to call any of the 
functions from some other thread after some expensive task has 
been performed. But it may also block until the senders task is 
finished and directly call any of the `exec::set_*` 
functions. How this is done is up to each and every sender.


Receivers are result handlers
=============================

As stated above, receivers are the recipients of sender results. 
When a sender has performed whatever it is designed to do it 
communicates to the receiver on one of the 3 channels and sends 
any result values along those channels.

The receiver then decides what to do with those values; it is 
basicly a function with the signature `void receiver_function(result_channels);`.

A call to `exec::submit(sender, receiver);` can then be 
(conceptually) rewritten as `receiver_function(sender_function());`

Note that just because receiver functions don't have return 
values doesn't mean they can't have side effects. They can very 
much do things such as submitting a new sender to a new receiver 
etc.

The following construct:

```
receiver_A(sender_A());
receiver_B(sender_B());
```

can be written as

```
auto recv_A = receive_then_submit(receiver_A{}, sender_B{}, receiver_B{});
exec::submit(sender_A{}, recv_A);
```

where `receive_then_submit` is an _adapter_ that forwards all 
channels to an instance of `receiver_A`. If `sender_A` activates 
the `value` channel the adapter would also call 
`exec::submit(instace_sender_B, instance_receiver_B);`

Schedulers create senders
=========================

Schedulers are used to create a very specific type of senders.
The sender must accept receivers with an empty value-channel,
and when submitted together with such a receiver it must
eagerly submit the receiver for execution on an execution agent.

In this case it is the receiver that peforms the bulk of the work,
the senders only role is to make sure that this work happens
on some execution agent that the sender has access to.

But again the receiver can in turn submit new senders and receivers,
alter some shared state etc.

So while the concept is simple, it encapsulates a very important piece of
work; moving execution from one context to another.

Schedulers have a requirement of either having a `schedule()` member
function, or letting the `schedule(scheduler)` function be found via
ADL. A third option is available for senders.

Senders can be schedulers
=========================

Each `sender` that can be submitted with a `receiver` with a value-less 
`set_value` channel, (`exec::set_value(receiver)` is valid) can be
used with `exec::schedule`, and if no specialization is done,
will simply cause `exec::schedule(sender)` to return sender again.

And in this special-case `exec::submit(exec::schedule(sender), receiver)`
is the same as `exec::submit(sender, receiver)`.

Senders can be executors
========================

An executors job is to, at some point (again), execute the functions 
given to it. The functions must take no arguments and return values are 
ignored (or forbidden?).

This is done through `exec::execute(executor, function);` which eventually
must result in `function()` being called.

As with all other parts of the proposal, `exec::execute(...)` is a
customization point which will first use `executor.execute(function)`,
if that doesn't work it will try to use ADL to call
`execute(executor, function)`. Finally, if nothing else works, it will
attempt to call `exec::submit(executor, as_receiver(function))`.

now `as_receiver` is an implementation-defined class, but
it has the requirement that `exec::set_value(as_receiver(function))`
will call `function()`.

So with this an executor can be a scheduler, and both schedulers and
executors can be senders. And remember case number [3] of `exec::submit`:
it will call `exec::execute(sender, as_invocable(receiver))`. This means
that senders can be executors. But this also means that invocables can be
receivers and receivers can be invocables.

What is the meaning of all this
===============================

The mental model is a bit challenging, but it helps to keep in mind
that a single sender receiver pair models either one of

```
receiver(sender()); // #1

// #2
sender();
receiver();

// #3
receiver();
```

Note that there is no case of only `sender()`. A sender will never
perform work without a receiver, even if the receiver does nothing it
must still be present.

Closing thoughts
================

I like the thought of having seperate channels, I often like to think in terms
of dataflow and this encapsulates this. It allows for the relatively easy propogation of
multiple values. I have, as a learning exercise, tried to implement some partial
functionality and it was this expeirence that made it click for me. Just reading
the paper was quite daunting. All in all I think it can be valuable.

One thing I may experiment with is to allow `set_value` and `set_error` to
return an optional error or something of the like. And see if I can do away with
exceptions since they are problematic in my main field of work which is embedded.
