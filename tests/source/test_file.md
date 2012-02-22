Document Generator
==================

By Peter Antoine

Copyright (c) 2011 Peter Antoine

Released under the Artistic Licence

What?
-----

This is a simple test file to test that the document processor can generate the correct document from the
a source document file.

It also, as good test code should be, is an example of how to generate the documentation and how the
documentation should be used.

First Test (default)
--------------------

This is the simple test of the default API grouping. So we should be able to list the default items here.
So, here is the default state_machine:

[model:/default/state_machine]

And, as the default should also have a sequence diagram, here is the default sequence diagram:

[model:/default/sequence_diagram]

As, the sequence diagrams can be split up into the interesting bits that we want to look at, let us look
at the sequences for the server only:

[model:/default/sequence_diagram/server]

Then, we may want to look at the the client, as this is the bit that other poeple would have to look at:

[model:/default/sequence_diagram/client]

This is a bit small and a little boring, so lets do something a little bit more interesting and look at 
the media sink, as this talks to both of these things.

[model:/default/sequence_diagram/media_sink]

Second Test (other)
-------------------

The other does not have a sequence diagram, but does have a state machine, so lets output that in full.

[model:/other/state_machine]

Then, we should do a partial output just to prove that this works. So here is state that must always 
exist within a state machine:

[model:/other/state_machine/start]

And it would not be fair to ignore the other one that is also required within a state machine:

[model:/other/state_machine/finish]

Final Text (test_one)
---------------------

And finally just for completeness here is the other state machine that has been created within this
positive test set of files:

[model:/test_one/state_machine]

That is the basis of the tests.

