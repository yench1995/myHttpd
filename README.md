# myHttpd
#### tinyHttpd
THe Analysis_of_tinyHttpd folder includes the source code of tinyHttpd project by JDB. It is a simple and lightweight 
http server to learn the basic priciple and internal logic of web server.


#### the version of myHttpd
Using epoll and multithread to realize a simple http server. In the main thread, it uses epoll to deal with the connection and create a thread for the further implementation. Now it only supports the "GET" method, and other methods will be added afterwards.   
  
##### version 1.0
Supports the "GET" method, create a thread for each connection.


##### version 1.1
Add thread\_pool for the server.The initial thread\_pool size is 8.
