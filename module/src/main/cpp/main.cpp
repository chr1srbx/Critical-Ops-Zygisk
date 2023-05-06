#include <cstring>
#include <jni.h>
#include <pthread.h>
#include <android/log.h>
#include "zygisk.hpp"


using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

// function predefines
static int enable_hack;
static char *game_data_dir = NULL;
int isGame(JNIEnv *env, jstring appDataDir);
void *hack_thread(void *arg);
void* triggerbot_thread(void* arg);
#define LOG_TAG OBFUSCATE("primetools")
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        env_ = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) {
            return;
        }
        enable_hack = isGame(env_, args->app_data_dir);
    }
    int ret, ret1;
    pthread_t ntid1, ntid;
    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (enable_hack) {
            pthread_create(&ntid, nullptr, hack_thread, nullptr);
            pthread_create(&ntid1, nullptr, triggerbot_thread, nullptr);
        }
    }

private:
    JNIEnv *env_{};
};

REGISTER_ZYGISK_MODULE(MyModule)