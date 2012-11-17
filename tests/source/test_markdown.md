Test Markdown
=============

This is simple block of text to test if wrapping is working.
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.


Ok, this file is here to test the features of markdown that we are going to support.
So lets start with _italic_ in both flavours of *italic*. Now lets tests __bold__ in
both flavours of **bold**. The next simplest is ``code``. This is a $MACRO so it can
be changed on the fly.

Now need to test that \\ actaully produces a thing and this\\ does not have a space in front of it and this text\\text is one word, also
that this \\work has a space before but not after.

This paragraph is here to test the extras [link one][http://github.com] now we need 
to have a look at the footnote [^footnote]. Also we should check that the whole
point of this exercise still actually works [name][doc_gen:/config_file_api/api/function/DGTF_IF_AddString/name?inline=true,reference=true]

Also, might be good to see if the indexes still work:

[doc_gen:/document?index,number,cbreak]

This should be a second paragraph. With a macro name test.

[doc_gen:/$GROUP_MACRO_NAME/api/function/$ITEM_MACRO_NAME]

Header Level 2 (and header tests)
---------------------------------

Now Let's have a list of un-numbered headers upto level 6 (we support more but that
is just silly).

# Header Level 1 #
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

## Header Level 2 #
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

### Header Level 3 #
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

#### Header Level 4 #
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

##### Header Level 5 #
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

###### Header Level 6 #
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

The end '#' does not matter in these cases as they do not a lot. As the code just throws all the ending stuff away, only
need two more tests for the # headers.

## Header Test with mutliple endings ######################
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

## Header Test with no endings
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

1 Numbered header Tests
The following are the numbered headers being tested. These should produce the correct headers for the levels.

1.1 Numbered Header level 2
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

1.1.1 Numbered header level 3
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

1.1.1.1 Numbered header level 4
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

1.1.1.1.1 Numbered header level 5
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

1.1.1 Level 3 increment tests
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

1.1.1 Level 3 Increment again test
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

1.1.1.1 Level 4 test (again)
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

012.3456.7.89 Test with all the numbers, variable (level 4).
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

1.1 testing jumping more than one level (stupid - but people are)
fddfd ff dfd fds fd fdf dsf df dsf sdf dsfds

1.1.1.1 This should work correctly

Now Lets test the Lists
=======================

    -	Simple List item 1
    -	Simple List item 2
    -	Simple List item 3
    -	Simple List item 4
    -	Simple List item 5


1. Single level test.
1. Single level test.
1. Single level test.
1. Single level test.
1. Single level test.
1. Single level test.



1. Numeric List level 1
	1. Numeric list level 2.
		1. Numeric list level 3.
1. Item 2 at level 1.
1. Item 3 at level 1.
	1. Item 1 at item 3 level 2.
	2. Item 2 at item 3 level 2.
	1. item 3 at item 3 level 2.
		4. Item 1 at item 3, item 3 at level 3.
		4. Item 2 at item 3, item 3 at level 3.
		4. Item 3 at item 3, item 3 at level 3.
1. Item 4.
    Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
    Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
    dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
    non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

	Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
    Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
    dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
    non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.


- Dash list using spaces.
    - Dash list using spaces.
    - Dash list using spaces.
        - Dash list using spaces.
        - Dash list using spaces.
- Dash list using spaces.
    - Dash list using spaces.
        - Dash list using spaces.
- Dash line using spaces.


* Asterisk list using spaces.
    * Asterisk list using spaces.
    * Asterisk list using spaces.
        * Asterisk list using spaces.
        * Asterisk list using spaces.
* Asterisk list using spaces.
    * Asterisk list using spaces.
        * Asterisk list using spaces.
* Asterisk line using spaces.

+ Plus list using spaces.
    + Plus list using spaces.
    + Plus list using spaces.
        + Plus list using spaces.
        + Plus list using spaces.
+ Plus list using spaces.
    + Plus list using spaces.
        + Plus list using spaces.
+ Plus line using spaces.

Now test the blocks within a block
==================================

+ This 1 is the first line of a level 1 list With two paragraphs
    Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
    Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
    dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
    non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

	Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
    Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
    dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
    non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

    + This 2 is the first line of a level 2 list With two paragraphs
    Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
    Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
    dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
    non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

    Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
    Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
    dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
    non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

    + This 3 is the first line of a level 2 list With two paragraphs lazy format
    Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
aUt enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
bdolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
cnon proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

    Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
dUt enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
edolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
fnon proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

+ This 3 is the first line of a level 1 list With two paragraphs, the second a quoted block
    Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
    Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
    dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
    non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

    > Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
    > Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
    > dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
    > non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.
	
+ This is the first line of a level 1 list With two paragraphs, the second a code block
    Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
    Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
    dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
    non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

		Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
		Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
		dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
		non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

+ Single line.

Now Lets Test the blocks
========================

First lets test the complicated block quote.

> Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
> Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
> dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
> non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

Oh, we really need this nested.

> This level is just for bloody mindedness.
> > Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
> > Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
> > > Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
> > > Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure
> > > dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
> > > non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.
> > dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat
> > non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.
> Really!!!

Now let's do the simple code block.

    /*----- FUNCTION -----------------------------------------------------------------*
     *  name: level_index_init
     *  desc: This function will initialise the level index.
     *        If the memory has already been allocated for the index, then it will just
     *        reset the table values.
     *--------------------------------------------------------------------------------*/
    void	level_index_init(LEVEL_INDEX* index)
    {
    	if (index->level_index == NULL)
    	{
    		index->level_index  	= calloc(LEVEL_EXTEND_SIZE,sizeof(unsigned short));
    		index->max_levels		= LEVEL_EXTEND_SIZE;
    		index->current_level	= 0;
    		index->api_index_id		= 0;
    	}
    	else
    	{
    		memset(index->level_index,0,sizeof(unsigned short) * index->max_levels);
    		index->current_level	= 0;
    		index->api_index_id		= 0;
    	}
    }

Now lets to the guarded code block

```
/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_init
 *  desc: This function will initialise the level index.
 *        If the memory has already been allocated for the index, then it will just
 *        reset the table values.
 *--------------------------------------------------------------------------------*/
void	level_index_init(LEVEL_INDEX* index)
{
	if (index->level_index == NULL)
	{
		index->level_index  	= calloc(LEVEL_EXTEND_SIZE,sizeof(unsigned short));
		index->max_levels		= LEVEL_EXTEND_SIZE;
		index->current_level	= 0;
		index->api_index_id		= 0;
	}
	else
	{
		memset(index->level_index,0,sizeof(unsigned short) * index->max_levels);
		index->current_level	= 0;
		index->api_index_id		= 0;
	}
}
```

More tests need writing, I think only the escape and the tables.


Markdown Tables
===============

This section is here to test the Markdown tables. It will test the two variants 
of tables that are supported by this markdown parser.

1.1 Simple Tables

These are the more annoying of the tables. It uses a simple two line system that
allows for the layout of the columns to be worked out but the way that the under
lines and the table header are organised. These tables only allow for a single line
of text in the table. The same break chars should be used for the header line and
underline line, or the code cant work out the justification and will assume left.

 Right Justified     Center Justified		Left Justified 
----------------    ------------------		------------------------
text				more text				and more
line 2				23232.222				and much more
line 3				23.1223					just for the lose

The above should be a three column simple table.

1.2 Complex Tables

Funnily these are more simple. Every line of the table must have either a \| or a
\: in them. If the first line of a paragraph has the \| in it then it is a assumed
that the rest is a table. Obviously there is a check that these things are aligned 
for the header and the next lines (an probably others later).

Right Justified | Center Justified | left Justified
---------------:|:----------------:|:-----------------------------------------
text			|	more text	   |and more
line 2			|	23232.222	   |and much more
line 3			|	23.1223		   |just for the lose
multiline 1     |   this does not  | might want to
                |                  | to wrap this column
line 4			|	237.1223	   |just for the lose

This is the bare version of the tables. Now lets look at the boxed version.

|Right Justified | Center Justified | left Justified        |
|---------------:|:----------------:|:--------------        |
|text		 	 |	more text	    |and more				|
|line 2			 |	23232.222	    |and much more			|
|line 3			 |	23.1223		    |just for the lose		|
|                |                  | to wrap this column	|
|line 4			 |	237.1223	    |just for the lose		|

The \| on the end is not required and will be ignored in the output. it will also
accept a version with '=' but that just means that the title will be shaded. if the
format allows for that.


