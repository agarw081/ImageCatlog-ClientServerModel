# ImageCatlog-ClientServerModel
This application implements a client server model using C language where images stored on server are downloaded by client program that requests it. Downloaded images are verified whether corrupted or not using md5 checksum. This program was written as an assignment for Operating Systems course under Professor Tian He

Student Name1: Jayanth Avasarala.
Student Name2: Umang Agarwal.

Student ID1: 5108471.
Student ID2: 5108031.

To compile the program, type 'make' from the directory the source files are in.
To run the server program, type ./server <port number> <directory>.
To run the client program in passive mode, type ./client <ip_address> <port number> <file_type>.
To run the client program in interactive mode, type ./client <ip_address> <port number>.
