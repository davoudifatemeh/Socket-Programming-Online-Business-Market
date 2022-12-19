# Socket Programming Online Business Market

In this project, I have implemented an "online Business Market" using socket programming and C language system calls.

## How to do the project
We have a server that wants to hand over a number of projects to expert users. Users participate in groups of 5 for each existing project. Then they offer a price for that project and the user with the lowest price offer wins that project. Finally, the server removes that project from the list of projects.

In this project, we have a central server that always listens on a specific port (port x) and waits for clients (expert users) to connect. The server and each client are a process. After connecting to the server, each user sees the list of available projects. Then, to participate in the group related to each project, it announces its number to the server.

The server is responsible for forming the group and announcing the winner of the project. To form a group, firstly, in the order of user entry, each user is assigned an ```ID``` (to determine the turn) and after the completion of the group, a broadcast port is assigned to that group and the activity of that group begins. Also, after announcing the result of each project group, the server removes it from the list of available projects.
The communication between the server and each user is ```TCP``` type, and after the start of the group activity, the communication between the users of that group will be ```UDP``` type.

After the start of the group activity, users take turns to send their suggested price to the rest of the group's users on the specified port from the server side at the beginning of the group formation. Each user has 10 seconds to announce their proposed price. If this price is higher than the price that has been set so far, or if he does not make any offer after this deadline, it will be the next person's turn. At the end, if in a complete round, all users offer a higher price than the lowest offered price, or if they run out of subscriptions, the winning user announces the project number to the server. Then the server closes that group and removes that project from the list.

## Timer
To measure each user's turn time, I used unix signals and more precisely, ```SIGALRM``` signal.

## System synchronization
Throughout the program (in client and server code) the entire system is running concurrently so that the server can handle multiple clients at the same time. Considering that some of the system calls are blocking, we use the select system call to solve this problem. This call is responsible for monitoring synchronous communication and makes all I/Os to be done asynchronously and no part of the code is blocking.
