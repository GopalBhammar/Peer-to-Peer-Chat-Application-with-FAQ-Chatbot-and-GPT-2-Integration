Requirements:
Ensure that UUID is supported on your system.

gcc server.c -o server -luuid
./server

gcc client.c -o client -luuid
./client

Hardcoded Configurations:
The IP address is currently hardcoded as "127.0.0.1" and the port as "5566". If you want to use different IP addresses or ports, you need to modify both the client and server codes accordingly.
Details of the Program and How to Access Features:
Note: Sometimes the user > prompt may not appear; pressing Enter will display it. However, the functionality works fine without it.

/active

Description: Retrieve a list of active clients.
Usage: /active
Example: /active
/send <dest_uuid> <message>

Description: Send messages to other clients using their unique IDs.
Usage: /send <dest_uuid> <message>
Example: /send <uuid> Hello there!
/logout

Description: Request to exit the application.
Usage: /logout
Example: /logout
/chatbot login

Description: Enable the chatbot feature.
Usage: /chatbot login
Example: /chatbot login
/chatbot logout

Description: Disable the chatbot feature.
Usage: /chatbot logout
Example: /chatbot logout
/history <recipient_id>

Description: Retrieve the conversation history between the requesting client and the specified recipient.
Usage: /history <recipient_id>
Example: /history 987654321
/history_delete <recipient_id>

Description: Delete chats of the specified recipient from the requesting client's chat history.
Usage: /history_delete <recipient_id>
Example: /history_delete 987654321
/delete_all

Description: Delete the complete chat history of the requesting client.
Usage: /delete_all
Example: /delete_all
/chatbot_v2 login

Description: Enable the GPT-2 FAQ chatbot feature.
Usage: /chatbot_v2 login
Example: /chatbot_v2 login
/chatbot_v2 logout

Description: Disable the GPT-2 FAQ chatbot feature.
Usage: /chatbot_v2 logout
Example: /chatbot_v2 logout
Task Details (all tasks are implemented in the same file):
Peer-to-Peer Chat:

Clients connect to the server and receive a unique identifier upon successful connection.
Clients can view active clients, send messages to other clients, and request to exit the application.
The server manages client connections, message sharing, and error handling.
FAQ Chatbot:

Implements a FAQ chatbot feature with predefined questions and answers.
Clients can toggle the chatbot feature on/off using commands.
The server maintains the chatbot status for each client and responds to queries with appropriate messages.
Chat History:

Maintains a chat history for each client, storing previous messages and conversations on the server.
Clients can retrieve conversation history with specific recipients, delete chats of specified recipients, or delete their complete chat history.
Messages sent between clients are stored in the chat history for both sender and recipient.
FAQ Chatbot (using GPT-2):

Implements a FAQ chatbot feature with predefined questions and answers using the GPT-2 model.
Clients can toggle the chatbot feature on/off using commands.
The server maintains the chatbot status for each client and responds to queries with appropriate messages.