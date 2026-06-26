#ifndef REDIS_CLI_H
#define REDIS_CLI_H

#include "HashMap.h"

class RedisCLI {
    private:
        HashMap<std::string, std::string> redisData;
    public:
        void run();
};

#include "../src/RedisCLI.cpp"

#endif