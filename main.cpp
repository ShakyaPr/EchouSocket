#include "App.h"
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>

std::mutex mutx;
std::condition_variable cv;
std::string_view outputMsg;
bool lExit = false;

uWS::App app ;
struct PerSocketData {};
uWS::WebSocket<false, true, PerSocketData> *gws=nullptr;

std::queue<std::string_view> msgQueue;

void producerThread(std::string_view msg) {
    /*
    Adding "[ECHO] " text to the every incoming message
    */

//    std::unique_lock<std::mutex> ul(mutx);                        //mutex unique lock to lock the thread
//    //std::string echo = "[ECHO] ";
//    std::string m = static_cast<std::string>(msg);
    if (mutx.try_lock()){
        std::cout << "Incoming message: " << msg <<std::endl;
        msgQueue.push(msg);
        mutx.unlock();
    }


}

void consumerThread(uWS::OpCode opCode) {
    //wait until producer produces a message
    //std::unique_lock<std::mutex> ul(mutx);
    //cv.wait(ul,[]{ return lExit; });
    if (mutx.try_lock()){
        while (!msgQueue.empty()){
            std::cout << "Outgoing message: " << msgQueue.front() << std::endl;
            gws->send(msgQueue.front(),opCode,true);          //send the response through socket
            msgQueue.pop();
        }
        mutx.unlock();
    }

//    lExit = false;
//    cv.notify_one();


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

            },
            .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {

            }
    }).listen(9001, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << 9001 << std::endl;
        }
    }).run();
}