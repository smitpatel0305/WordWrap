CS214 Systems Programming Project 2: Word Wrap by Smit Patel and Aditi Patel.

To test my code, we created a number of large and small text files, as well as checked for several specific cases, which include, but are not limited to:

- various buffer sizes

- different widths

- using numerous special characters (e.g., spaces and tabs) to begin and end words

- having a term that is longer than the breadth


To determine if the text came from stdin, we redirected a file into stdin with varying lengths of text to see if the content would wordwrap to stdout. 

To see if my software could work with directories and produce text files with word-wrapped text.
I mostly made folders with a mix of large and small files, and I also tested for:

- absolute path to files

- file paths that are relative

- subdirectories of subdirectories

- Paths that are invalid


We tried all of these scenarios to see whether our programme functioned properly, and I also used numerous softwares to check for memory leaks. 
