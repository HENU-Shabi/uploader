//
// Created by root on 3/14/20.
//

#include "uploader/media/FFmpeg.h"
#include <iostream>
#include <memory>
#include <utility>
using namespace uploader;
FFmpeg::FFmpeg(boost::filesystem::path ffmpeg, boost::filesystem::path rootDir,boost::filesystem::path fileName,const std::string& res) {
    this->rootDir = std::move(rootDir);
    this->fileName = std::move(fileName);
    map_init(this->resMap)
            (std::string("360P"),VidRes::RES360P)
            (std::string("480P"),VidRes::RES480P)
            (std::string("720P"),VidRes::RES720P)
            (std::string("1080P"),VidRes::RES1080P);
    this->res = resMap[res];
    this->ffmpeg = std::move(ffmpeg);
}

template<>
std::string uploader::transcodeCmd<VidRes::RES360P>(const boost::filesystem::path& rootDir,const boost::filesystem::path& fileName) {
    return std::string(" -i ")+(rootDir / fileName).native()+std::string(" -c:a copy -vf \"scale=-2:360\" -c:v libx264 -profile:v baseline -level:v 3.0 -x264-params scenecut=0:open_gop=0:min-keyint=72:keyint=72 -minrate 600k -maxrate 600k -bufsize 600k -b:v 600k -y ")+rootDir.native()+"/"+std::string("h264_baseline_360p_600.mp4");
}

std::unique_ptr<std::vector<uploader::InStream>> FFmpeg::operator()() {
    auto streams = std::make_unique<std::vector<uploader::InStream>>();
    streams->push_back(uploader::InStream("audio", "h264_baseline_360p_600.mp4", "audio.mp4"));
    auto ffmpegChildren = std::vector<boost::process::child>();
    ffmpegChildren.push_back(thumbnailExec(this->ffmpeg, "480x360", this->rootDir, this->fileName));
    if(this->res >= 360){
        ffmpegChildren.push_back(transcodeExec<VidRes::RES360P>(this->ffmpeg, this->rootDir, this->fileName));
        streams->push_back(uploader::InStream("video", "h264_baseline_360p_600.mp4", "h264_360p.mp4"));
    }
    if(this->res >= 480){
        ffmpegChildren.push_back(transcodeExec<VidRes::RES480P>(this->ffmpeg, this->rootDir, this->fileName));
        streams->push_back(uploader::InStream("video", "h264_main_480p_1000.mp4", "h264_480p.mp4"));
    }
    if(this->res >= 720){
        ffmpegChildren.push_back(transcodeExec<VidRes::RES720P>(this->ffmpeg, this->rootDir, this->fileName));
        streams->push_back(uploader::InStream("video", "h264_main_720p_3000.mp4", "h264_720p.mp4"));
    }
    if(this->res >= 1080){
        ffmpegChildren.push_back(transcodeExec<VidRes::RES1080P>(this->ffmpeg, this->rootDir, this->fileName));
        streams->push_back(uploader::InStream("video", "h264_high_1080p_6000.mp4", "h264_1080p.mp4"));
    }
    for(auto &&child : ffmpegChildren) {
        child.wait();
        if (child.exit_code() != 0) {
            spdlog::error(std::string("FFmpeg returned exit code ") + std::to_string(child.exit_code()) +
                          std::string(" during transcoding process exited for ") +
                          std::string((this->rootDir / this->fileName).native()));
        } else {
            spdlog::info(std::string("FFmpeg transcoding process exited successfully for ") +
                         std::string((this->rootDir / this->fileName).native()));
        }
    }
    return streams;
}

boost::process::child uploader::thumbnailExec(const boost::filesystem::path &ffmpeg, const std::string &res,
                                              const boost::filesystem::path &rootDir,
                                              const boost::filesystem::path &fileName) {
    return boost::process::child(
            ffmpeg.native() + " " + " -i " + (rootDir / fileName).native() + " -vcodec mjpeg -vframes 1 -an -s " + res +
            " -f rawvideo -ss 30 " + (rootDir / "thumb.jpg").native());
}

template<VidRes vidRes>
boost::process::child
uploader::transcodeExec(const boost::filesystem::path &ffmpeg, const boost::filesystem::path &rootDir,
                        const boost::filesystem::path &fileName) {
    return boost::process::child(ffmpeg.native() + " " + transcodeCmd<vidRes>(rootDir, fileName));
}


template<>
std::string uploader::transcodeCmd<VidRes::RES480P>(const boost::filesystem::path &rootDir,
                                                    const boost::filesystem::path &fileName) {
    return std::string(" -i ") + (rootDir / fileName).native() + std::string(
            " -c:a copy -vf \"scale=-2:480\" -c:v libx264 -profile:v main -level:v 3.1 -x264-params scenecut=0:open_gop=0:min-keyint=72:keyint=72 -minrate 1000k -maxrate 1000k -bufsize 1000k -b:v 1000k -y ") +
           rootDir.native() + "/" + std::string("h264_main_480p_1000.mp4");
}
template <>
std::string  uploader::transcodeCmd<VidRes::RES720P>(const boost::filesystem::path& rootDir,const boost::filesystem::path& fileName){
    return std::string(" -i ")+(rootDir / fileName).native()+std::string(" -c:a copy -vf \"scale=-2:720\" -c:v libx264 -profile:v main -level:v 4.0 -x264-params scenecut=0:open_gop=0:min-keyint=72:keyint=72 -minrate 3000k -maxrate 3000k -bufsize 3000k -b:v 3000k -y ")+rootDir.native()+"/"+std::string("h264_main_720p_3000.mp4");
}
template <>
std::string  uploader::transcodeCmd<VidRes::RES1080P>(const boost::filesystem::path& rootDir,const boost::filesystem::path& fileName){
    return std::string(" -i ")+(rootDir / fileName).native()+std::string(" -c:a copy -vf \"scale=-2:1080\" -c:v libx264 -profile:v high -level:v 4.2 -x264-params scenecut=0:open_gop=0:min-keyint=72:keyint=72 -minrate 6000k -maxrate 6000k -bufsize 6000k -b:v 6000k -y ")+rootDir.native()+"/"+std::string("h264_high_1080p_6000.mp4");
}