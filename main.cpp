#include "App.h"
#include <mutex>
#include <thread>
#include <condition_variable>

std::mutex mutx;
std::condition_variable cv;
std::string_view outputMsg;
bool lExit = false;

uWS::App app ;
struct PerSocketData {};
uWS::WebSocket<false, true, PerSocketData> *gws=nullptr;

void producerThread(std::string_view msg) {
    /*
    Adding "[ECHO] " text to the every incoming message
    */

    std::unique_lock<std::mutex> ul(mutx);                        //mutex unique lock to lock the thread
    std::string echo = "[ECHO] ";
    std::string m = static_cast<std::string>(msg);
    std::cout << "Incoming message: " << m <<std::endl;

}

void consumerThread(uWS::OpCode opCode) {
    std::unique_lock<std::mutex> ul(mutx);
    cv.wait(ul,[]{ return lExit; });                 //wait until producer produces a message
    std::cout << "Outgoing message: " << outputMsg << std::endl;
    gws->send(outputMsg,opCode,true);          //send the response through socket
    lExit = false;
    cv.notify_one();
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

                std::thread t2(consumerThread,opCode);
                std::thread t1(producerThread,message);
                t1.join();
                t2.join();

            },
            .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {

            }
    }).listen(9001, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << 9001 << std::endl;
        }
    }).run();
}