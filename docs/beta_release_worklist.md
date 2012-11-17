Beta Release worklist
=====================
(# = done , T = done and tested)

The following items need to be done before the beta 1 release.

# 1. Add the special targets.
   This changes the way that the output is produced, esp. the 'all' target. Also, this will be needed
   for the index changes.

# 2. Add the new parameters.
   simply add these and there flags to the DRAW_STATE.

# 3. Add the input indexing.
#   change the way that input is handled.
   	The current input files have a "type:" in front of them, if the document is going to be a single
	object, then the files will have to drop the type and a new parameter '-f' output format will have
	to take a list for formats that the output should be output in.

#  pre-parse the input.
   	For the document sections to work, an index of the document will need to be built, or the
	ToC won't work. So there will need to have an index built.

	document Toc needs: section -> sub_section

	section has document name and it's index number.
	sub_section has level and index and header name.

#   document sections with indexes.
    This will need to be added to the model. This should just be a list of section (by file) and sub-sections
	added as they are found. This obviously should be created by the pre-parser.

# 4. Model indexing.
#   Add the index generation functions.
#     This simply should work off the model to generate the index, in the correct format.

#	All items will need an index item, to make referencing easier.

#5. Fix the markdown parser to fully support the markdown.
#   Simply need to go thru the daring fireball webpage and pick out the features that are missing. So be a couple
   of minor changes. The main being the underline which will be the most difficult to implement.

#   Need to implement block-quoting.

#   Also, need to add the section numbering. Again should be a minor change, add a couple of new states to the
   parser state machine.

#   Add Macros to the files.

#	Add tables.

#6. CSS for the html output. (design the page layout)
   This is simply a design job, and the html will probably change to add some classes to the html where ness.

7. Make sections use consume_line so that they pass through the markdown parser.

