/*
 * Tencent is pleased to support the open source community by making wechat-matrix available.
 * Copyright (C) 2018 THL A29 Limited, a Tencent company. All rights reserved.
 * Licensed under the BSD 3-Clause License (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Created by liyongjie on 2017/12/7.
//

#include "fd_canary.h"
#include <thread>
#include <android/log.h>
#include <time.h>
namespace fdcanary {

    FDCanary& FDCanary::Get() {
        static FDCanary kInstance;
        return kInstance;
    }

    FDCanary::FDCanary() {
        exit_ = false;
        //std::thread detect_thread(&FDCanary::Detect, this);
        //detect_thread.detach();
    }

    /*void FDCanary::SetIssuedCallback(OnPublishIssueCallback issued_callback) {
        issued_callback_ = issued_callback;
    }*/

    void FDCanary::OnOpen(const char *pathname, int flags, mode_t mode,
                          int open_ret, const JavaContext& java_context) {
        
        std::string value;
        dumpStack(value);
        collector_.OnOpen(open_ret, value);
        //dumpStack();
    }

    void FDCanary::AshmemCreateRegion(const char *name, size_t size, int fd) {

        __android_log_print(ANDROID_LOG_DEBUG, "FDCanary.JNI", "ProxyAshMemCreateRegion name:%s, size:%zu, fd:%d", name, size, fd);
    }

    void FDCanary::OnClose(int fd, int close_ret) {
        
        collector_.OnClose(fd);
        //dumpStack();
        //todo 调用太多了
        
        //OfferFileFDInfo(info);
    }

    void FDCanary::dumpStack(std::string& stack) {
        time_t t1;
        time(&t1);
        
        call_stack_.dumpCallStack(stack);
        time_t t2;
        time(&t2);
        __android_log_print(ANDROID_LOG_WARN, "FDCanary.JNI", "t1:[%ld], t2:[%ld], speed time:%ld",t1, t2, (t2-t1));
        //__android_log_print(ANDROID_LOG_WARN, "FDCanary.JNI", "stack: %s, ", stack.c_str());    
    }
   /*void FDCanary::OfferFileFDInfo(std::shared_ptr<FDInfo> file_fd_info) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_.push_back(file_fd_info);
        queue_cv_.notify_one();
        lock.unlock();
    }

    int FDCanary::TakeFileFDInfo(std::shared_ptr<FDInfo> &file_fd_info) {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        while (queue_.empty()) {
            queue_cv_.wait(lock);
            if (exit_) {
                return -1;
            }
        }

        file_fd_info = queue_.front();
        queue_.pop_front();
        return 0;
    }

    void FDCanary::Detect() {
        std::vector<Issue> published_issues;
        std::shared_ptr<FDInfo> file_fd_info;
        while (true) {
            published_issues.clear();

            int ret = TakeFileFDInfo(file_fd_info);

            if (ret != 0) {
                break;
            }

            for (auto detector : detectors_) {
                detector->Detect(env_, *file_fd_info, published_issues);
            }

            if (issued_callback_ && !published_issues.empty()) {
                issued_callback_(published_issues);
            }

            file_fd_info = nullptr;
        }
    }*/

    FDCanary::~FDCanary() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        exit_ = true;
        lock.unlock();
        queue_cv_.notify_one();

        //detectors_.clear();
    }
}
