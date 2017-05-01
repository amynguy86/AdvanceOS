Compiling Instructions
Build the Common Folder first before building the Server and Client folder.
Use the MakeFile to compile.
All the object files and executables will be in the inst directory(created by makefile)

Use the Client to update and read from the server

Running Instructions
Client Commands:

Update 
U KEY VALUE UPDATETOSERVERNO

READ
R KEY READFROMSERVERNO

runServer instructions
Start the master process first and then start other servers
execute the runServer without any arguments to see the directions

Server Commands:

BREAK(severe connection with other servers)
break servername servername ... (eg. break 0 1 2)

ENABLE (restablish connection with server)
enable serverName serverName...

Note: break and enable must be applied to both ends of the connection. 
For example: If you do "break 1" at server 0, then "break 0" should also be done at server 1.

Note: You may have to press enter sometimes to see the prompt(-->), only then enter the commands.

This is because we are enabling and disabling connections using a flag. You are only creating logical partitions.  

Example run on UT dallas dc01-dc09 servers(see the log files for output)
Servers:
./inst/runServer 0 8 16.116.129.23:0000 10.176.66.51 2> server0ErrLog
./inst/runServer 1 8 10.176.66.51:1024 10.176.66.52  2> server1ErrLog
./inst/runServer 2 8 10.176.66.51:1024 10.176.66.53  2> server2ErrLog
./inst/runServer 3 8 10.176.66.51:1024 10.176.66.54  2> server3ErrLog
./inst/runServer 4 8 10.176.66.51:1024 10.176.66.55  2> server4ErrLog
./inst/runServer 5 8 10.176.66.51:1024 10.176.66.56  2> server5ErrLog
./inst/runServer 6 8 10.176.66.51:1024 10.176.66.57  2> server6ErrLog
./inst/runServer 7 8 10.176.66.51:1024 10.176.66.58  2> server7ErrLog

Client:
./inst/runClient

