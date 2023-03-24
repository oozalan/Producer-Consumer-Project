# PCP-Simulator
This program was made for the CSE3033 Operating Systems course. It uses the word "publishers" instead of producers and "packagers" instead of consumers. It was developed in Linux environment.

## Program Details
* Publisher threads publish books and packager threads package the books published.
* There can be different types of publishers. Each publisher type publish one type of book and it is different than the types of the books published by other publishers.
* Publisher type amount, publisher amount, the amount of books that each publisher will publish and packager amount are given as command line argument.
* Publisher types have their own buffers. Buffer sizes are given as command line argument.
* If a buffer is full, publisher that will publish a book doubles the size of the buffer.
* When a publisher publish the required amount of books, it exits the system.
* Each packager can take a book from any buffer.
* Each packager thread will put some number of books in a package. This number are given as command line argument.
* After one package is prepared, packager starts to prepare another package.
* Packagers select the book type to put into package randomly. Then, they will take the book from the related buffer and put it into its own package.
* If a buffer of the publisher type randomly chosen by the packager is empty, the packager will check whether there are threads left in the system that belongs to chosen type. If so, it will wait for the publisher to publish a book and put it into the buffer. Otherwise, it will select another buffer.
* If a packager finds all buffers empty after all publishers exit the system, it prints the message: “Only i of j number of books could be packaged.” and leave the system.

## How to Run
* Example command: ./pcp-simulator.c -n 2 3 4 -b 5 -s 6 7
  * 2: Publisher type
  * 3: Publisher amount for each type
  * 4: Packager amount
  * 5: Number of books to be published by each publisher
  * 6: Package size of the packagers
  * 7: Initial size of the buffers
