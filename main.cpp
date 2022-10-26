#include "App.h"
#include <mutex>
#include <thread>
#include <condition_variable>
#include <pthread.h>
#define THREAD_NUM 4

std::mutex mutx;
std::condition_variable cv;
std::string_view outputMsg;
bool lExit = false;

uWS::App app ;
struct PerSocketData {};
uWS::WebSocket<false, true, PerSocketData> *gws=nullptr;
//uWS::OpCode Op;

pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

std::string_view taskQueue[1000];
int taskCount = 0;

void producerThread(std::string_view msg) {

}

void consumerThread(uWS::OpCode opCode) {

}

void executeTask(std::string_view msg){
    gws->send(msg,OpCode,true);
    std::cout << "Outgoing message: " << msg << std::endl;
}

void* startThread(void* args){
    while (1){
        std::string_view message;

        pthread_mutex_lock(&mutexQueue);
        while (taskCount == 0){
            pthread_cond_wait(&condQueue, &mutexQueue);
        }

        message = taskQueue[0];
        for (int i=0; i<taskCount-1;i++){
            taskQueue[i] = taskQueue[i+1];
        }
        taskCount--;
        pthread_mutex_unlock(&mutexQueue);
        executeTask(message);
    }
}

void submitTask(std::string_view message){
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = message;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

int main() {
    pthread_t th[THREAD_NUM];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);

    for (int i = 0; i < THREAD_NUM; i++) {
        if (pthread_create(&th[i], NULL, &startThread, NULL) != 0) {
            perror("Failed to create the thread");
        }
    }

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

                submitTask(message);

            },
            .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {

            }
    }).listen(9001, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << 9001 << std::endl;
        }
    }).run();

}