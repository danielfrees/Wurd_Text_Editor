# Wurd_Text_Editor

### OVERVIEW:

This is a very fast, fairly simple text editor akin to vim which can run on Mac or Linux. Additional capabilities on top of standard text editing include undo, spell checking, and spelling suggestions.

The speed was accomplished by clever use of C++ standard library structures and careful implementation of all methods to achieve strict Big O requirements (these reqs set by Professor David Smallberg of UCLA's CS32).

### MY WORK: 

I wrote StudentTextEditor.h, StudentTextEditor.cpp, StudentUndo.h, StudentUndo.cpp, StudentSpellCheck.h, StudentSpellCheck.cpp. The other files in this project were either written by my professors, or are text files for the purpose of playing with the text editor.

My files implement the core editing functionality of the text editor, provide undo capabilities, and spell check words (the word you are hovering over with your cursor) with spelling suggestions [This last piece was implemented using a Trie].
