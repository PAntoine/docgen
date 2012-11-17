# Document Generator #
  
By Peter Antoine  
Copyright (c) 2011 Peter Antoine  
Released under the Artistic Licence  

## Table Of Contents ##

[Table Of Contents][doc_gen:/document?index,number,cbreak]

## Table Of Contents ##

[doc_gen:/document?index,number,reference]
[doc_gen:/document?index,line,reference]
[doc_gen:/document?index,flat,reference]

Now only three levels  

[doc_gen:/document?index,reference,levels=3]

## Alternate Table of Contents ##

[doc_gen:/document?index,flat,number,reference]

## Another Alternate Table of Contents ##


## What? ##
  
This is a simple test file to test that the document processor can generate the correct document from the
a source document file. 
  
It also, as good test code should be, is an example of how to generate the documentation and how the
documentation should be used.  

## First Test (default) ##

The following is a test 

[doc_gen:/document?index,level,number]
  
This is the simple test of the *default API* grouping. So we should be able to list the default items here. But might as well test 2 * 2.  
So, here is the default state_machine:  

[state machine][doc_gen:/hello/state_machine]

and in table form:  
[doc_gen:/hello/state_machine?table=true]


And, as the default should also have a sequence diagram, here is the default sequence diagram:  
  
[the default state machine][doc_gen:/default/sequence_diagram]

and in table form:  

[doc_gen:/default/sequence_diagram?table=true]
  
As, the sequence diagrams can be split up into the interesting bits that we want to look at, let us look
at the sequences for the server only:
  
[doc_gen:/default/sequence_diagram/server]
  
Then, we may want to look at the the client, as this is the bit that other poeple would have to look at:
  
[doc_gen:/default/sequence_diagram/client]
  
This is a bit small and a little boring, so lets do something a little bit more interesting and look at 
the media sink, as this talks to both of these things.
  
[doc_gen:/default/sequence_diagram/media_sink]

### Second Test (other) ###
  
The other does not have a sequence diagram, but does have a state machine, so lets output that in full.

[doc_gen:/other/state_machine]

Then, we should do a partial output just to prove that this works. So here is state that must always
exist within a state machine:

### Need another level three ###

[doc_gen:/other/state_machine/start]

And it would not be fair to ignore the other one that is also required within a state machine:

[doc_gen:/other/state_machine/finish]

## Final Text (test_one) ##

And finally just for completeness here is the other state machine that has been created within this
positive test set of files:

[doc_gen:/test_one/state_machine]

That is the basis of the tests.

## API tests ##
 
Dump the full API for the default group:

[doc_gen:/default/api?index,reference]

[doc_gen:/default/api?paged=true,margin=8]

Now dump the API for the config_file_api group:

[doc_gen:/config_file_api/api?index,reference]

[doc_gen:/config_file_api/api]

## Now dump the API for the test_2_group: ##

[doc_gen:/test_2_group/api?index,line,reference]
[doc_gen:/test_2_group/api]

Ok, Now dump only the functions from this group:

[doc_gen:/test_2_group/api/function]

And now we need to dump one function from this group: 

[doc_gen:/config_file_api/api/function/DGTF_IF_AddString]

Why not just the function name _inline_ , from this group [doc_gen:/config_file_api/api/function/DGTF_IF_AddString/name?inline=true] to prove that this can be done.

[doc_gen:/config_file_api/api/function/DGTF_IF_AddString/name]

And the prototype on its own:
[doc_gen:/config_file_api/api/function/DGTF_IF_AddString/prototype]

### This is just to test the changing of levels within the file ###

## API Tests Continued - Now dump the types  ##

All the types in the system:

[doc_gen:/all/api/type]

All the types in default:
[doc_gen:/default/api/type]

All the types for test_1:

[doc_gen:/test_1/api?index]

[doc_gen:/test_1/api/type]

and the one specific type (from test_1):

[doc_gen:/test_1/api/type/DGTF_IF_SECTION_NAME]

and the one specific type name only:

[doc_gen:/test_1/api/type/DGTF_IF_SECTION_NAME/name]

all the names of the types in the system:

[doc_gen:/all/api/type?index=true]

new tests: 

## dumping all the apis in the system ##

[doc_gen:/document?index,level,number,cbreak]

[doc_gen:/all/api]

# Another top level to test the top level-ness

some random text to show what sub-stuff does.

## Now Test to see if the samples work  ##
  
Dumping the first test sample:

[doc_gen:/default/sample/test_three]




