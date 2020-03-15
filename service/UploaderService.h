//
// Created by root on 3/14/20.
//

#ifndef UPLOADER_UPLOADERSERVICE_H
#define UPLOADER_UPLOADERSERVICE_H

#include <Component.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include "media/FFmpeg.h"
#include "media/Shaka.h"
namespace uploader {
    struct Vid{
        std::string id;
        std::string res;
        char * data;
        unsigned long long len;
        Vid(const std::string& id,const std::string& res,char * data, unsigned long long len);
        ~Vid();
    };
    class UploaderService : Component {
    private:
        boost::filesystem::path httpdRoot,ffmpeg,shaka;
    public:
        explicit UploaderService(cppcms::application &ctx);
        UploaderService &operator<<(const Vid& vid);
    };
}


#endif //UPLOADER_UPLOADERSERVICE_H
