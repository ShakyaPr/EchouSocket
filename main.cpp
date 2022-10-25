#include "App.h"
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>

uWS::App app ;
struct PerSocketData {};
uWS::WebSocket<false, true, PerSocketData> *gws=nullptr;

std::queue<std::string> msgQueue;

std::string addEcho(std::string_view msg){
    std::string echo = "[ECHO] ";
    std::string m = static_cast<std::string>(msg);
    m = echo + m;
    std::string res = m;
    m = " ";
    return res;

}

void producerThread(std::string_view msg) {



        std::cout << "Incoming message: " << msg << "\n" << std::endl;
        std::string outMessage = addEcho(msg);
        msgQueue.push(outMessage);


}

void consumerThread(uWS::OpCode opCode) {


        while (!msgQueue.empty()){

            std::cout << "Outgoing message: " << msgQueue.front() << msgQueue.size() << "\n" <<std::endl;
            std::string_view res = msgQueue.front();
            gws->send(res,opCode,true);          //send the response through socket
            msgQueue.pop();
        }



}

int main() {

    app.ws<PerSocketData>("/*", {
            /* Settings */
            .compression = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR_4KB | uWS::DEDICATED_DECOMPRESSOR),
            .maxPayloadLength = 100 * 1024 * 1024,
            .idleTimeout = 16,
            .maxBackpressure = 100 * 1024 * 1024,
            .closeOnBackpressureLimit = false,
            .resetIdleTimeoutOnSend = false,
            .sendPingsAutomatically = true,
            /* Handlers */
            .upgrade = nullptr,
            .open = [](auto *ws) {
                gws = ws;
            },
            .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {

                // two threads for process the incoming message and send the response
                std::thread t1(producerThread,message);
                std::thread t2(consumerThread,opCode);
                t1.join();
                t2.join();
//                  msgQueue.push(message);



            },
            .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {

            }
    }).listen(9001, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << 9001 << std::endl;
        }
    }).run();
}