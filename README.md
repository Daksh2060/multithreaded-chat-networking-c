# UDP Multithreaded Chat Networking in C

Welcome to the UDP Multithreaded Chat Networking in C project! This application is a powerful and efficient chat app built using the C programming language, featuring multithreading and UDP for robust and responsive networking. The primary goal of this project is to demonstrate advanced concepts in network programming within a multithreaded environment, showcasing a seamless peer-to-peer chat experience.

## Features

- **Multithreaded Architecture:** The application employs multithreading to handle concurrent tasks efficiently. Each thread is dedicated to specific responsibilities, such as keyboard input handling, UDP message exchange, and real-time message display.

- **Efficient Networking with UDP:** UDP is utilized to ensure efficient and low-latency communication between peers. This choice of protocol is particularly suitable for real-time applications, providing a streamlined and responsive chat experience.

- **Threaded Keyboard Input Handling:** The application efficiently manages keyboard input using threads, allowing users to interact with the chat system seamlessly. This design ensures that input processing does not hinder other aspects of the application.

- **Real-time Message Display:** Messages are displayed in real-time, providing users with an immersive chat experience. The multithreaded design allows for simultaneous message sending and receiving, making the conversation flow smoothly.

## Why UDP?

In this project, UDP (User Datagram Protocol) was chosen over TCP (Transmission Control Protocol) for its specific advantages in the context of a real-time chat application. UDP is a connectionless and lightweight protocol that offers faster communication compared to TCP. While TCP ensures reliable and ordered delivery of data, UDP sacrifices these guarantees for speed and efficiency.

## Getting Started

Follow these steps to set up and run the UDP Multithreaded Chat Networking in C:

1. Clone the repository to your local machine.
   ```bash
   git clone https://github.com/your_username/udp-multithreaded-chat-networking-c.git
   ```

2. Compile the source code using makefile (recommended in WSL):
    ```bash
    make
    ```

3. To test by yourself, open 2 terminals, in the first terminal run the following command:
    ```bash
    ./chat [user1_port_number] localhost [user2_port_number]
    ```

    For example:
    ```bash
    ./chat 6000 localhost 7000
    ```

4. In the second terminal, run the following command:
    ```bash
    ./chat_app [user2_port_number] localhost [user1_port_number]
    ```

    For example:
    ```bash
    ./chat 7000 localhost 6000
    ```

Now, the UDP Multithreaded Chat Networking in C is set up and ready to use. 

## Contact

Feel free to reach out if you have any questions, suggestions, or feedback:

- **Email:** your.email@example.com
- **Twitter:** [@your_twitter_handle](https://twitter.com/your_twitter_handle)

