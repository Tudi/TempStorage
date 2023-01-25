# scoring-fileserver
Store files based on type+id indexing.
Supports sharding based on file "id".
Able to merge files into single file to avoid storing millions of files(Might make "save" slower).
Able to create directory tree to avoid very long file open times.
