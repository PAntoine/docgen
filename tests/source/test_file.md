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

[model:/hello/state_machine]

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

API tests
---------

Dump the full API for the default group:

[model:/default/api]

Now dump the API for the config_file_api group:

[model:/config_file_api/api]

Now dump the API for the test_2_group:

[model:/test_2_group/api]

Ok, Now dump only the functions from this group:

[model:/test_2_group/api/function]

And now we need to dump one function from this group: 

[model:/config_file_api/api/function/DGTF_IF_AddString]

Why not just the function name inline, from this group [model:/config_file_api/api/function/DGTF_IF_AddString/name] to prove that this can be done.

[model:/config_file_api/api/function/DGTF_IF_AddString/name]

And the prototype on its own:
[model:/config_file_api/api/function/DGTF_IF_AddString/prototype]


API Tests Continued - Now dump the types
----------------------------------------

All the types in the system:
[model:/*/api/type]

All the types in default:
[model:/default/api/type]

All the types for test_1:
[model:/test_1/api/type]

and the one specific type (from test_1):
[model:/test_1/api/type/DGTF_IF_SECTION_NAME]

and the one specific type name only:
[model:/test_1/api/type/DGTF_IF_SECTION_NAME/name]

all the names of the types in the system:
[model:/*/api/type/*/name]


