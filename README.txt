Compiling Instructions
Build the Common Folder first before building the Server and Client folder.
Use the MakeFile to compile.

Running Instructions
ClientCmd Commands
Read
R KEY SERVERNO 

Update 
U KEY VALUE

Insert
I KEY VALUE

Hash(returns the serverNo that the key hashes to. Useful for read command)
H KEY

Break(break connection with the server)
B SERVERNO

runServer instructions
Start the master process first and then start other servers
execute the runServer without any arguments to see the directions