
                        DOCUMENT GENERATOR
					      Peter Antoine
					  2011 (c) Peter Antoine

version: 1.0
date:    5th Nov 2011

[doc_gen:/document?index,line,number,levels=3,reference]

1 Statement Of Purpose

The doc_gen application is designed to create system documents from comments in the source code.
It is especially designed to generate Sequence and State Machine diagrams.

1.1 Reasoning.
As with all document generators from code the main reason is to aid the keeping of documentation and
code in sync. This is designed to allow for the extracted information to be able to be formatted in
other ways and to have the documentation to be defined by a definition file so the documentation can
be properly formatted. It is also good to be able to code actual documents and have the app stitch 
the two together in a sensible way.

1.2 Responsibilities.

	1. Locate all the special markers in the code tree and add them to a DB.
	2. Build the connected graphs for the different items.
	3. collate the document mark-up with the generated data.
	4. generate the resulting mark-up.
	5. for all steps above validate the data and return warning/errors.

2 System Operation.

2.1 Functional Operation

To allow for build-tree conditional compilation. (In file conditionals will be ignored) the system will
work in the same way as the C style compilers. That is a doc_compile function will run on all the source
code file and produce data files that hold all the extracted information, after while a compile step will
run that will then produce the final diagrams and then link in all the external data files and produce
the final mark-up.

The "compiler" will create the output files in the same way that compilers do and have the equivalent of
the "-o" (output) flag. There should be no requirement for the the compiler to include any other file or
configuration file as it will just create an "object" file of all the data items that it finds.

The "linker" will take all the "object" files and validate then and then generate the mark-up. The compiler
will take the list of created "object" files plus the support files to create the final mark-up. Also, the 
"linker" may need to accept a configuration file for producing the final output.

2.1.1 Peters Document Source Compiler (pdsc)

This function will create the document object file (.dof) that is used by the document linker to to produce
the final mark-up. The function will simply take in a source file and produce an object file as specified.
This function can fail if an error is detected in the mark-up of the document.

The objects in a source file can either be grouped explicitly by name. Though the name can be a macro that
is passed into the compile function. This allow for multi items the be created and have meaningful and non
contrived names.

2.1.2 Peter Document Linker (pdl)

This function will produce the final mark-up for the document. It will take a list of files. I will spit
out a marked up function.

2.1.3 Peter's Document Proccesor (pdp)

This utility will take in the linked document from the pdl and process it with the other .md documents
to produce the final doc for output. This is what produces the final document. This is the function handles
the markup (Markdown).

This application will treat all files that are included within this group as one document to be output. This
will allow for references, index's and table of contents to be produced from multiple files to a single
output document.

This application will also generate the documents with the output plugins that are requested. It will allow
for the same model to be produce the same document in different formats.

3 Operation Rational

The use of a compiler and a linker basically has the following reasons:

3.1 Map Reduce
As will all compilers the compile and link stages can be broken up and done in parallel if the build system
will allow.

3.2 Build System Friendly
As the compile function can be placed in a makefile in the same way that the source compiler function can, it
means that an almost duplicate rule can be used to generate the document object file.

3.3 Partial Build
Again as the make system can be used to select which source files have changed and which needs to be rebuilt
this means that rebuilding the documents are the same as rebuilding the source and will speed up the creation
of the documents.

4 Design.

4.1 Mark-up overview

The mark-up style will follow the current conventions that are used in JavaDoc, Doxygen and others. Mostly 
because they are familiar to most programmers and a lot of editors will recognise most of the syntax.

So all mark-up fields will be defined within a C style comment with two leading **'s.

i.e

	/**
	 * This is a comment block.
	 */

The basic atom are have the following format:
  @<atom_type> -- atom value --

The atom value can be one of the following types:

name         	 format/description
-------------    --------------------------------------------------------------------------------------
name             <atom_group>.<name>
literal          numeric or string literal
trigger          Logical statement for stating when the item is actioned.
text line        single line of text ended at the end of line.
text string      block text string ending with the <atom_type> or end of the comment.
sample           A selection of the code/document that will be placed in the file without formatting.
mutliline        This allows for a selection of text that will be placed in the output, this text
                 will be formatted using the Markdown format.

Also, the system will allow for macros '\$macro_name' to replace all or part of the strings. This is
mostly to allow for group type names to be tailored by the build. This will allow for the same file
to be used by several build and be part of different system or sub systems.

To reduce the amount cruft that the editor has to add to the source files, the @file atom block can
be defined at the top of the source file. This block defines the global groups for the all the atom groups.
You also can define the sub-groups i.e. the timeline name.

4.2 Structure

All the graphs/diagrams are defined by nodes. All nodes need to have a type and a name. Also depending
on what type of node it is then they may have other dependencies. For example that a state_machine node
will have a @next: as all state machine nodes (with the exception of sm_end) have a next state.

All the required atoms MUST be in the same comment block. If they are not present or cannot be inferred
then the compiler will raise a warning and/or fail.

The types of atom are TYPE, ACTION, ORDER, PAYLOAD and GROUPING. 

The non-diagram types that are added do not follow this format.

4.2.1 TYPE
Each node is a comment and must have one of the atoms that define the TYPE of node within the comment
block. So for a state machine node the comment block should have the @state atom within. With the
sequence diagram the @message atom must be defined.

4.2.2 ACTION
Each node should have an ACTION atom that will tell the compiler what the next node to transverse to is.
Without an action atom then node will be assumed to be a terminating node.

4.2.3 ORDER
If a node is defined in several parts then the actions will be defined in the order that they are compiled.
Unless a ORDER atom is found. I.e. if you require a message to be sent before another message that is not
dependent on the previous atom then use @after: {name}, with {name} being the name of the previous atom.

4.2.4 PAYLOAD
The payload atom defines the content of the action. It may contain what the logical action would be i.e.
the content of the message. Also, it may define other attributes of the action, i.e. when or if the action
would occur. Examples of these atoms would be @condition, @if or @parameter.

4.2.5 GROUPING
Some of the atoms may need to be grouped together as a logical grouping. i.e. for the sequence diagram they
have an "activation" that is a group of logically connected messages. So the grouping atoms are the atoms
that are used to create these things. for the example given the group atom would be @activation. This would
allow the mark-up to create a group and give it the attribute "activation". Groups are not required but
the system to draw graphs. The grouping should come from the connections created by the actions.

The group atoms do not override or interfere with the atom_group as this is used to control the connection
of the graphs and is used to sub-divide the graphs. The GROUP names should not have an atom_group as this
would remove the point of them as logical grouping on the diagrams and not as structural atoms.

4.3.6 STRUCTURE
These atoms are for aiding the structure of the documentation. They are there for helping the compiler 
organise the state of the atoms.

4.3 Document Input File Format

The structure of the finished document is defined by the document files. These file describe what elements
from the model that is built from the source files are to be output in the finished document.

These files are to be based on Mark-Down and any output produced by the system can be referenced by
using the Markdown reference escape [...] with the URI of model:/<atom_name>, i,e,

	[model:/group_name/state_machine]
	[model:/group_name/timeline]

the default diagrams can be references via:

	[model:/<group_name>/state_machine]
	[model:/<group_name>/timeline]

which will cause the main state machine diagram to be inserted at that point in the diagram. All items that
have been produced by the system can be referenced more than once.

Sub-diagrams can be built by the system if the sub-part is supplied on the model line.

The model that the document is built on is based on a linked data blob that is selected by the build system.
This allows for the build systems to produce different documents based on the build configuration. This means
that only one confiuration, that of the build is required to be maintained.

NOTE: it is intended for the system to be extended to handle other data styles and tables and these will
be added as there own URLs.

4.4 ATOM descriptions (by data type)

The current diagrams/data types supported are the Sequence Diagram and the State Machine.

4.4.1 State Machine Diagrams

4.4.1.1 Structure of a state machine

State Machines are described in the following way:

	[start] ----> [state_1] -----> [state_2] -----> [state_3] ----> [finish]
	   |             |               ^                 |
	   |             |               |                 |
	   |             v               |                 |
	   +-------> [state_4] ----------+                 |
                     ^                                 |
                     +---------------------------------+

(As an example)

This diagram has no triggers or conditions for the state transitions that can be specified for each of the
transitions.

4.4.1.2 Definitions of the allowed atoms

@state: {state_name}
 This defines the name of the state. It is also the type atom.
 This atom does not have to be unique if it occurs more than once in a tree then they are seen as the same state.
 The states MUST not have the same @next as this cannot be built in the tree.#

@next: {state_name}
 This defines the next transition of the state machine.
 This atom must point to a state this has been defined within the group of states.

@condition: {state}
 This atom defines the condition that causes the transition to occur. This is mostly documentary and can be text.

@trigger: <trigger_name>
 This atom describes FORMALLY when the trigger occurs. This can be used to generate a formal state machine. The
 trigger name will be created and checks can be made to see if the triggers are sent and received. This atom will
 be displayed as a comment and also can be used to prove that the state machine is correct.

 Note 1: that the trigger does not need to belong to group that the state machine is in. As it can be used to link
 state machines in the system together. If the {location} states internal then the trigger will need to be generated
 with the same group.

 Also, triggers can work across the diagram types and also can just be fired. This will help stitch the whole system 
 together.
 
 Note 2: @trigger can also be stand alone if the trigger for a state machine is generate outside of the state machine.

@triggers:
This sends the trigger for the @trigger to receive. It can send a trigger across state machines or sequence diagrams.

4.4.1.3 Validity of States
	- All states must have the following atoms: @state and @next.
	- The state pointed to by @next must exist and be defined within the same group.
	- If a state has a @trigger the @triggers atom must be called at least once. (does not need to be within the group).
	- A @trigger and @condition atom cannot exist within the same state.
	- If the atom_group is used then this will divide the state machine.
	- The state "finish" is implied and does not need to be defined. But, if it is defined it cannot have a @next atom.
	- @state is allowed to to have a group, if it does not it defaults the global group.
	- all other atoms are not allowed to specify the group (except @state, @trigger and @triggers).

4.4.2 Sequence Diagram

4.4.2.1 Structure of a Sequence Diagram

    [timeline_1]       [timeline_2]       [timeline_3]       [timeline_4]
    	 |-------mess01----->|                  |                  |     
    	 |                   |-mess02(a,b,c,d)->|                  |     
    	 |<---------------mess04----------------|                  |     
    	 |                   |-----mess05----->[ ]                 |     
    	 |                   |                 [ ]----mess06------>|     
    	 |                   |                 [ ]----mess07------>|     
    	 |                   |                 [ ]<---mess08-------|     
    	 |                   |<----mess09------[ ]                 |     
    	 |<------mess10------|                  |                  |     
    	 |<----mess11-[cona]-|                  |                  |     
    	 |<----mess12-[conb]-|                  |                  |     

(as an example)

	mess01: Simple message sent from timeline1 to timeline2
	mess02: message sent from timeline_2 to timeline_3 with parameters.
	mess04: Simple message sent from timeline_3 to timeline_1.
	mess05: Message sent from timeline_2 to timeline_3 and received within an activation.
	mess06: sent from within an activation.
	mess07: send after mess6 within an activation.
	mess08: waitfor mess7 and sent to an activation.
	mess09: waitfor mess8 then sent to timeline_2.
	mess10: waitfor mess9 and sent to timeline_1.
	mess11: sent on condition cona from timeline_2 to timeline_1.
	mess12: sent on condition conb from timeline_2 to timeline_1.

These are the currently supported messages and conditionals supported.

4.4.2.2 Definitions of the allowed atoms.

@send: {name}
 Send the named message.

@call: {name}
 Call the function as part of the sequence.

@to: {life_line}
 Defines the receiving timeline.
 {limitations????}

@condition: {condition string}
 This is the textual condition that the message is sent on.

@trigger: <trigger_name>
 Same as defined for the state machine.

@waitfor: {message_name}
 This states that the message must be after the named message has arrived.

@after: {message_name}
 This states that the message will be sent after named message.
 The named message MUST be sent from the same timeline.

@timeline: {name}
 The name of the timeline that the message is to be sent from.

@responds: {message_name}
 This the same as to, except that the response goes to the
 timeline that sent the original message message_name.

@repeats: {repeats string}
 This atom states that the specific message repeats for the
 condition given (this is text string).

@activation: {activation_name}
 This is a called activation that groups messages together and can be called from other

@broadcast: {message_name}
 This is a message that is sent to all timelines. Each instance of a broadcast can only
 be matched once per timeline.

4.4.2.3 Validity Of Sequences
	1. All Sequences must have either @call or @send.
	2. All Sequences must have a @to or @responds.
	3. Only one @waitfor atom can be specified per node.
	4. If @condition is specified the @trigger can not be. (and vis versa).
	5. If @trigger is specified then one @triggers must be specified (does not have to be within a sequence diagram).
	6. The message pointed to by @responds must have been sent to the current timeline.
	7. The @after clause can be specified on a message or an activation, from the same timeline.
	8. All messages must what a corresponding @waitsfor.

4.4.3 Global file format ATOMS

4.4.3.1 Structure file header format

There is no structure for this.

4.4.3.2 Valid atoms for the file state.

@file:
This defines the atom group that defines the defaults for the file. Any atom that is found in this group will
be the defaults for the file.

@author:
This atom defines the author of the file.

@group:
It defines the default group for the all the atom groups in the file. It means that all other un-defined groups
will default to this group.

@timeline:
This atom when in the file group will define the default timeline for all other sequence diagram atoms.

4.4.3.3 Validity of File formats

All states that are listed in 4.4.3.3 are valid and can be given in the @file atom block. They can only be one
given once and should be at the top of the source file.

4.4.4 API ATOMS
The following atoms are are used to define API's that are present within a source file. These should be used to
define the API calls. It is assumed that functions that do not have the @api atom in the block preceding the
definition of the function will not be part of the API.

4.4.4.1 Structure API

The API will find functions. The structure of the definition of a API function should be:

	/**
	 * comment block with the api definition in.
	 */
	<variable amount of white space>
	function_prototype parameters

These will be defined by the language type.

4.4.4.2 Valid atoms for the API

@api:		{function_name}
The name of the function as it should appear in the api.

@brief:		{ brief description of the fuction}
The brief description of the function.

@action:	{action description}
A text section that defines the action of the api. This is a multiline block of text that ends at the next
atom or the end of the comment block. Also, this can be added to the main body of the function if another
action block is found.

@group:		{group_name}
The group that the atom belongs to.

@parameter:	<name>	<description>	One of the parameters, If any.
Definition of one of the parameters for the function.

@returns:	{value} {reason}
Definition of one of the return types for the function.

@ignore:	This stops all collection of data from the header block.
This tag can be used as a nice way of ending a multi-line comment.  It essentially ends the comment block as
far as the processor is concerned.

4.4.4.3 Validity of API Atoms.
1. One one @api will be allowed in a comment block.
2. Only one @brief will be accept in a block.
3. @action must follow a @api or be in an @api block.
4. @action will be attached to any previous @action found for the the @api.
5. @api must be before the function.
6. all atoms found within the function following the @api will be attached to the @api.

4.4.5 Data Type ATOMS

The following is to describe the data type definition atoms.

4.4.5.1 Structure Data Type ATOMS
No inherent type for data types as they will produce a text table within the target document.

4.4.5.2 Valid atoms for the Data Types
@group:	{group_name}
The group that the type belongs to.

@type: {type_name}
This defines the type block and tells the processor that the type that follows is to be decoded.

@defines: {define_name}
This denotes that the following defines are part of the API. All the defines that follow this bloc will be part
of the define block. 

@end_define
This atom is an optional atom the follows a @define so the parser can stop looking for defines to add.

@record: {record_name}
This atom defines a block that defines a record.

@field:	{type}	{name} {description}
The above defines a field in the record.

4.4.5.3 Validity of Data Type Atoms.
	1. One one data definition per block
	2. The @type and @defines must come before a data type or a #define respectively.
	3. @end_define must only be used after a @define and after at least one #define.

4.4.6 Application ATOMS
The Usage atoms are there to be able to describe the usage of an application. This will allow for the man pages for
the application to be generated from the code.

4.4.6.1 Structure of Application ATOMS
The usage atoms are based on the ones required to produce man pages. As a man page can have any structure that you care for
the atoms are quite free form to allow for the user to structure the manual pages as they see fit. If the application has
a command line the synopsis for the command line can be generated from the @option atoms. Also, if the function has internal
commands these can be formatted from the @command atom.

4.4.6.2 Valid atoms for Application
@application:	{name of the application}
This is the name of the application as will displayed in the manual pages.

@group: {group name}
The group that the atom belongs to.

@option: {option description}
This is the option. It is a single word in a free format. I.e. -p or --path or path, it is up to the application type to
define this.

@required:	{yes}\|{no}
If this is within a option block defines if the option is required on the command line. If the option is within an named
option, it defines if the option is required within the option group.

@multiple:	{yes}\|{no}
This atom states that the atom can accept multiple values.

@command:	{name}	{brief description}
This is a command that goes in to the command list.

@section: {section name}
This defines the section. All the text that follows the atom will be part of the section.

@subsection:	{sub-section name}
As with the @section this defines the subsection and the text of the section will follow the atom.

@synopsis: {name}	{list of options}
This is the standard synopsis of the application flags and usage.

@desc: {multi-line description of the application}
The basic description of the application.

@value:	 {definition of the values that are supported}
This defines the values that are allowed for the option.

4.4.6.3 Validity of Application ATOMS

1. There must be a least 1 @application atom within the source file.
2. All other atoms will bind to the last @application atom.
3. @value must follow a @option and will bind with the nearest one in the same file.
4. @required and @multiple must follow a @option and will bind with the nearest one in the same file.
5. If there is a @section or @subsection is found then it is added to the end of the section. If this
	happens across source files then the order that they are added is not defined and they should be
	written to handle this.


4.5 Intermediate object file format.

A simple file format that has the basic information.

	magic no = 0x70647363  (pdsc)
	version  = 0xMMmm      (Major/Minor)
	date     = 0xddmmyymmsspp (1 byte padding to keep the size word aligned)
	size     = number of records in the file
	name     = [length]<file-name as passed into the compiler>	(length is an unsigned int)
	<records>

There are three types of records: group, name, string.

The records all have the same format.

[type:ATOM:GROUP:<string>]

For the group, obviously ATOM and GROUP will always be zero, and the group name will be the string.
The others all fields are valid.

Also, the groups are referenced in order that they are found in the file. 

So the layout of the record is:

	byte 0 = type
	byte 1 = atom
	word 2 = group/function
	word 4 = name_size
	byte 6 - name_size bytes = name (this is not null terminated)

As function are groupless and just exist, they can share the same field.

4.6 Linking Rules

4.6.1 Global Structure.

The structure of the document will have a list of groups, each group is allowed to hold one of each 
of the supported diagrams (at the moment a state_machine and a sequence_diagram).

Also, the document will also have a list of triggers as this work outside of the diagrams. The triggers
are like all other elements in the system, if a trigger is referenced before it is defined then it will
be created as a ghost trigger and when it is defined this flag will be removed.

There is also an un-named default group that all items that do not have a specific group name belong to.
This group has the same requirements as all groups that only one diagram of any type can be used.

4.6.2 Linking the State_Machine

Each state is simply a list of transitions. As each of the atoms in the state machine are added then
the new transition will be added to the state that it belongs to. If it points to a state that does
not exist that state will be created as a "ghost" state and the connection made. When the state is
defined at a later state then the ghost flag will be removed.

As the states are added any transitions that are identical will be flagged as errors.

Also, transitions will be within the same state machine an this can be checked at this point.

Any triggers that are references will be looked up in the trigger list, if it already exists then it
will be referenced. Any trigger that is waited on, and is not defined will be created as a ghost 
trigger and when a definition is created then the ghost flag will be removed.

If the state_machine must be formal then all the states with transitions must have either a @waitfor
or a @condition specified.

4.6.3 Linking the sequence diagrams

The structure of the sequence diagram is a series of linked items. There are three element types to
the sequence diagram, the message/function call, then timeline and the activity.

4.6.3.1 Messages

Messages are transitions between timelines. Messages belong to the timeline that they are sent from. The
receiving timeline must have a @waitfor for each message sent. This means that all messages sent are
handled by some code. All messages are unique. If a message of the same name is sent it needs to be matched
each time, and the nearest match will be used.

Broadcast is a speciality message that is sent to all timelines that exist. They do not need to be matched
except once. They can only be matched once in each timeline.

4.6.3.2 Timelines

Timelines is a sequential ordering of messages. The time line is made up of message or activity connections.
These connections reference specific messages or an activity. The @after clause and the order
that the order that the messages are found define the timeline.

4.6.3.3 Nodes

Timelines are made up of nodes. The nodes are the points on the timeline where the messages are sent and
the messages are received. The nodes are allowed to generate triggers. These are not generated by the
messages as with the state transitions. All messages originate and arrive at a node, tags are attached to
nodes and the nodes are generated and cannot be references directly.

A @waitfor node will be generated for any message that does not already have a node. If the @waitfor has
other nodes or things dependant on them and there is an unsatisfied message then all the nodes for that
dependency will also be duplicated.

4.6.3.4 Activations

** currently these are called functions in the code - and they are intended to be changed **

These are collections of messages. The activations (if in a C/C++ file) will populate the parameters for
the call. Any messages that are define within the start/end markers (in C/C++ these are '{' and '}'
respectively). These are referenced counted so that the matching pair are used to bound the activation.
The messages that are found in the activation are attached in the order that they are found.

The activation is similar to the activation as the messages within the activation will be a link of nodes.
except the activation will be integrated within the timeline that calls it.

As the activation cannot be linked until all the files have been read and the places that require activations
have been found. The activations are to be integrated into the timelines that call them, so all the messages
that are used by the activation must be looked up then. So the activation can only list the nodes as lookups.
It needs to track the activation names and trigger names for looking up later.

Also, nodes within a message cannot have a sequence outside the activation. It is also assumed that messages
within a activation are in order and should not have a @after node. (This may change).

Activations can only be called within the current timeline. They can be called within a @waitfor block and
if they are then the message that @waitfor is waiting for is used within any @responds block that does not
have a specific @waitfor.

Activations cannot have any atoms that define which timeline they live in. It has to position independent 
so can be included and fixed up at any time. 

4.6.3.5 Node Fix-ups

The timelines are fixed up in the order of the first timeline to the last found in the sequence diagram. The
nodes are fixed starting with the first node found. Each timeline has control while it is in a sequence. I.e.
when a node the has a dependency or is the last in the chain of dependencies is being processed. Functions
members are also dependencies.

4.7 Final Output

The other input files (*.pmd) will be read in and scanned for the [model:/] url and anyplace that this is
found on the next new line then the diagram will be inserted. The default for the diagrams are text ascii
art representations. Also the output will also be as *.md files. The output will follow the standard 
MarkDown rules. 

The diagrams will be output as group_name_diagram.md for each of the diagrams found in the source tree.

Extension information can be added to the nodes in the sequence trees, these extensions can add links to
the actual source code lines that defines the atom states and the messages and transitions for the message
states.

4.7.1 Document File Structure

All the items that have been collected in to the model are accessed using a URL scheme. This scheme is 
designed to fit in with the Markdown model and make it easy to add to the document. Also it is a flexible
scheme that will be easy to extend.

The format of the URL is as follows:

  [model:/group/part/name/sub_names?parameters]

4.7.1.1 URL Name Format

The group and part are the major bits of the model. The name of the part, so for a state machine in the
"looking_good" group that is called "fair_dinkum", the model would look like. 

  [model:/looking_good/state_machine/fair_dinkum]

This model can be drilled down further, as the state machine could only display the nodes that are attached
to the "start" node. This would look like:

  [model:/looking_good/state_machine/fair_dinkum/start]

So that a complex state machine can be documented in individual states. This can be done for all the 
parts of the model, and the level of drill down is specific to the part in question. Also some of the drill
down will have fixed names, as for say the API part.

4.7.1.2 URL Parameter Format

The URL can have parameters that will request that the render changes the way that the item is rendered.
There are two sets of parameters, ones that are required by all plugins to understand and use, and others
that are optional for the plugin. Also, plugins are allowed to add there own, but all plugins must not fail
if it gets an parameter that it does not understand.

4.7.1.2.1 Parameter Field Types

The parameter field will take three types of parameters. These are boolean, numeric and string.

4.7.1.2.1.1 Boolean

This field has the format name=true or name=false. As the boolean flags default to true, just having the
flag name of the line will cause the value to be set to true. The underlying value MUST default to false
to make this consistent.

4.7.1.2.1.2 Numeric 

This field type has the format of 0xXX.... , (-)nnnn(.ffff). This allows for XX hexadecimal values to be
specified. The other format allows for numeric values to be specified. It is up to the format handling
code to state if the number is to big.

If no number is specified then it item defaults to 0.

4.7.1.2.1.2 String

This field type specifies a string. It is a simple " delimited string that has ascii encoded values. This
should be extended to UTF-8 at some point but currently just ascii (UTF-8 codepage 0).

4.7.1.2.2 Mandatory Parameters

4.7.1.2.2.1	inline

This is a boolean parameter.

This means that the object must be placed without line breaks, or other formatting. This should only be
applied to names or sections of text that will fit into the normal page structure.

4.7.1.2.2.2	paged

This is a boolean parameter.

This states (for formats that support it) that the items must have a page break supplied after it (or any
of it major sub-items).

4.7.1.2.2.3 margin

This is a numeric parameter.

This specifies the margin to be used for the item. It the format and the item supports the changing of the
margin, this margin should be used.

4.7.1.2.2.3 table

This is a boolean value.

This specifies that for diagram items that the table form of the diagram should be used instead of the 
graphical format. For some forms of diagrams tables make it easier to glean some of the information that
the pictorial format does not.

4.7.1.2.2.4 index

This is a numeric item, that specifies the level that the index should stop at.

The item is to be referenced as a index. So it will output the index for the item. This maybe a whole index
block or just a single line entry for the index. If the number is not present (or =0) then the whole subtree
of the item will be output as an index block.

4.7.1.2.2.5 reference

The specifies that a reference should be produced to the item. If this is in the same URL as index then all
the items in the index will have references to the items that they specify (if the format supports references).

4.7.1.2.2.6 number

This is a numeric item, that specifies the level that should be numbered.

This item specifies the level at which numbering stops. So if no value is supplied the tree is not numbered,
as this level defaults to 0. Else, it will say which level the numbering stops at.

4.7.1.2.2.7 number_style

The default style is "full" level numbers. So x.x.x.x.x.
other styles:

"tail" 
This style only the last number of the index is displayed and all the others are blank, each level is indented.
1
 1
  1
   1

4.7.1.1 Referencing and Indexing Items

The URL will specify the parts of the model tree to display. As the URL can specify any level of the tree
and the renderer will display all the levels from the level specified.

The items in the URL are referenced by the name given in the source file (obviously type and group are
inherited) and can be referenced by the heretical name.

Each level of the model tree will have and index number, if index numbering is enabled then this should
be used. This tree can be extended, some of the items are allowed to create new sub items that will have
there number as shown in the tree below.

So the following tree is shown with the index numbering.

	/default/											X.1
		state_machine/									X.1.1
						sm_one\							X.1.1.1
							   state_1					X.1.1.1.1
							   state_2					X.1.1.1.2
							   state_3					X.1.1.1.3
						sm_two\							X.1.1.2
							   ...etc...
		sequence_diagram/								X.1.2
						sq_one\							X.1.1.1
							   timeline_one				X.1.2.1.1
						sq_two\							X.1.1.2
							   ...etc...
		api/											X.1.3
						functions\						X.1.3.1
							   name						X.1.3.1.1
							   description				X.1.3.1.2
							   prototype				X.1.3.1.3
							   parameters				X.1.3.1.4
							   action					X.1.3.1.5
							   returns					X.1.3.1.6
						types\							X.1.3.2
							   name						X.1.3.2.1
							   record					X.1.3.2.2
							   description				X.1.3.2.3
						constants\						X.1.3.3
							   name						X.1.3.3.1
							   constant					X.1.3.3.2
							   description				X.1.3.3.3
		sample/											X.1.4
						samp_one						X.1.4.1
						samp_two						X.1.4.2
							   ...etc...
		application/									X.1.5
						app_one							X.1.5.1
							name	   					X.1.5.1.1
							section 					X.1.5.1.2
								  section_one			X.1.5.1.2.1
									sub_section_one		X.1.5.1.2.1.1
								   ...etc...
							option  					X.1.5.1.3
								  option_one			X.1.5.1.3.1
								   ...etc...
							command 					X.1.5.1.4
								  command_one			X.1.5.1.4.1
								   ...etc...
							synopsis					X.1.5.1.5
								  synopsis_one			X.1.5.1.5.1
								   ...etc...

The item is rooted in the current section number. This will be described later and is represented by
the X in the above scheme. The X represents the level within the document.

The group numbers will increment within the model and cannot be grantee across different builds of the
tree.

4.7.1.2 Special groups

In the URL scheme they are three special groups. These are default, all, document. 

4.7.1.2.1 default

Is not that special a group as it is the default/catch all group for atoms. If any of the atoms cannot
be placed in a group either by a direct reference or by the @group defined in the @file block, then they
will be added to default. Also, default will always be present within the model, even if it is empty.

4.7.1.2.1 all

This is a virtual group that cannot be referenced within the source atoms but can be used within the
document file as it references all the groups.

4.7.1.2.1 document

This non-group does not handle any model items. BUT, it can effect the way that they are rendered. Any
styles that are assigned to the items in the groups in the "document" group will become the default 
styles for the items to follow, unless they are overridden.

Also, all the sections that are defined in the document are added to the document, and this can be used
to generate things like the table of contents in a consistent way.

i.e. 

  [model:/document?index,number=true,reference=true]

The above would produce the table of contents, with numbers and references.

The document group has items a set of special items that referrer to the current document. Currently
the two special items are: page and chapter.

Having these special items allows for control over the format of the output page, so that orientation
and page breaks can be added. To add a page break the following line can be added:

 [model:/document/page?paged=true]

4.7.1.3 Referencing and Indexing Document Sections

In the document each section will also have a generated section number that is created by the index 
number of the file on the commands line. There should be a option to start numbering elsewhere to allow
multi-file documents to be generated (or too allow for a header/title section to be added).

The section numbers are created based on the headers found in the Mark-Down. For each level of header
at the same of lower than the current it will be indexed by one. If a header of an higher level is
found then the sub-level numbers are reset to 1.

All items in the model tree start at level 3. So including any item that will reset the values to level 3.
With the exception of "inline" items as these do not have levels.

So an application section (that is not "inline") will have the index of: 
  
 doc_number.doc_section.group_id.1.5.app_id.2

4.7.2 Markdown

The flavour of the Markdown that is to be supported in based on the Github Flavoured Markdown (GFM) but
with a couple of simplifications.

4.7.2.1 Unsupported Features

4.7.2.1.1 Inline HTML
As the target for the markdown will not only be the web, the inline HTML is not supported.

4.7.3.2 Supported Features

	1. Headers
	2. BlockQuotes
	3. Lists
	4. Code Blocks
	5. Horizontal Rules
	6. Links
	7. Emphasis
	8. Code
	9. Backslash Escape

4.7.3.3 Extended Features

4.7.2.3.1 Model Insertion
The model insertion is based on the Links section, except that the reference starts with the "model:"
URL scheme. Essentially it pretends to be an id. Without the extra boxes.

4.7.2.3.2 Section Numbering

If a number starts at the first characters after the newline and it only contains numbers and '.' then
this will be taken as a section number. Section numbers must have 1 less '.' than the number levels. This
is ensure that the section numbering is intended and not a numeric list.

If the first char is a number and the then it is to be a level number. As with the markdown the actual
number specified within the doc is not to be used. It just allows the doc to be readable as plain text before
it is passed.

so the item number:

```
    1.1.1 Start of Level
    1.1.1 Start of next level
```

would produce:

```
    1.1.1 Start of level
    1.1.2 Start of next level.
```

Also the numbering rules would be the same as the ones specified for index items.

If the section number is specified in the markdown then it will be replaced with the correct section number.


