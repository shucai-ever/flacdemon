//
//  Player.cpp
//  FlacDemon
//
//  Created by merryclarke on 28/09/2015.
//  Copyright (c) 2015 c4software. All rights reserved.
//

#include "Player.h"

FlacDemon::Player::Player(){
    ao_initialize();
    this->defaultDriverID = ao_default_driver_id();
    cout << "default driver: " << defaultDriverID << endl;
}
FlacDemon::Player::~Player(){
    
}
void FlacDemon::Player::setDatabase(FlacDemon::Database * idatabase){
    this->database = idatabase;
}
void FlacDemon::Player::stop(){
    this->killPlaybackFlag = 1;
}
void FlacDemon::Player::playTrackWithID(long ID){
    FlacDemon::Track * track = this->database->trackForID(ID);
    if(track){
        this->playTrack(track);
    }
}
void FlacDemon::Player::playTrack(FlacDemon::Track * track){
    if(!track->openFilePath() || !track->file->isMediaFile()){
        return;
    }
    
    //move variables to members of the class where possible

//    AVCodecContext * codecContext = track->file->formatContext->streams[0]->codec;
//    
//    if(!codecContext){
//        cout << "error getting codec context";
//        return;
//    }
    
    this->stopAudio();
    this->killPlaybackFlag = 0;
    
    /* find the mpeg audio decoder */
    AVCodec * codec = avcodec_find_decoder(track->file->codecID);
    if (!codec) {
        cout << "Codec not found" << endl;
        return;
    }
    AVCodecContext * codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        cout << "Could not allocate audio codec context" << endl;
        return;
    }
    /* open it */
    if (avcodec_open2(codecContext, codec, NULL) < 0) {
        cout << "could not open codec" << endl;

    }
    av_dump_format(track->file->formatContext, 0, "", 0);
    int bits=16;
    int planar = 0;
    switch (codecContext->sample_fmt) {
        case AV_SAMPLE_FMT_U8P:
            planar = 1;
        case AV_SAMPLE_FMT_U8:
            bits = 8;
            break;
            
        case AV_SAMPLE_FMT_S16P:
            planar = 1;
        case AV_SAMPLE_FMT_S16:
            bits = 16;
            break;
            
        case AV_SAMPLE_FMT_S32P:
            planar = 1;
        case AV_SAMPLE_FMT_S32:
            bits = 32;
            break;
            
        case AV_SAMPLE_FMT_NONE:
            break;
        default:
            cout << "unsupported sample format" <<endl;
            // should clean up before return;
            return;
            break;
    }
    this->sampleFormat.rate = (int)track->file->mediaStreamInfo->sampleRate;
    this->sampleFormat.bits = bits;
    
    AVFrame * frame = av_frame_alloc();
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    
    this->playerThread = new std::thread(&FlacDemon::Player::playAudio, this, track, codecContext, &packet, frame, planar);
    
}

void FlacDemon::Player::playAudio(FlacDemon::Track * track, AVCodecContext * codecContext, AVPacket * packet, AVFrame * frame, int planar){
    cout << "playing audio" << endl;
    int error, endoffile=0, gotFrame=0, samplelength=0;
    
    this->device = ao_open_live(this->defaultDriverID, &this->sampleFormat, NULL);
    

    while(endoffile == 0 && error >=0 && this->killPlaybackFlag == 0){
        if((error = av_read_frame(track->file->formatContext, packet)) < 0){
            if(error == AVERROR_EOF){
                endoffile = 1;
            } else {
                cout << "error reading frame from file " << track->valueForKey("filepath") << endl;
                break;
            }
        }
        
        
        
        if((error = avcodec_decode_audio4(codecContext, frame, &gotFrame, packet)) < 0){
            char errbuf[255];
            av_strerror(error, errbuf, sizeof(errbuf));
            cout << "could not read decode audio frame: " << *track->valueForKey("filepath") << ". " << errbuf << endl;
            break;
        }
//        cout << "got audio frame " << endl;
        samplelength = error;
        uint size = frame->linesize[0];
        uint8_t * samples;
        if(planar){ // this doesnt work, need to resample to interleaved for libao, for future development
//            samples = frame->data[0];
            size *= 2;
            samples = this->interleave(frame);
        } else {
            samples = frame->extended_data[0];
        }
        if(ao_play(this->device, (char*)samples, size) == 0){
            cout << "ao play error" << endl;
            //close device
            error = -1;
        }
        if(planar)
            av_freep(&samples);
    }
    
    ao_close(this->device);
    av_frame_free(&frame);
}
void FlacDemon::Player::stopAudio(){
    if(this->playerThread && this->playerThread->joinable()){
        this->killPlaybackFlag = 1;
        this->playerThread->join();
    }
}
uint8_t * FlacDemon::Player::interleave(AVFrame * frame){
    //unfinished feature
    if(!this->audioResampleContext){
        this->audioResampleContext = avresample_alloc_context();
        av_opt_set_int(this->audioResampleContext, "in_channel_layout",  AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(this->audioResampleContext, "out_channel_layout", AV_CH_LAYOUT_STEREO,  0);
        av_opt_set_int(this->audioResampleContext, "in_sample_rate",     this->sampleFormat.rate,0);
        av_opt_set_int(this->audioResampleContext, "out_sample_rate",    this->sampleFormat.rate,0);
        av_opt_set_int(this->audioResampleContext, "in_sample_fmt",      AV_SAMPLE_FMT_S16P,   0);
        av_opt_set_int(this->audioResampleContext, "out_sample_fmt",     AV_SAMPLE_FMT_S16,    0);
        int error;
        if((error = avresample_open(this->audioResampleContext)) < 0){
            cout << "open resampler failed" << endl;
            //free avr
        }
    }
    uint8_t * output;
    int out_linesize,
        samples_written;
    
    av_samples_alloc(&output, &out_linesize, 2, frame->linesize[0], AV_SAMPLE_FMT_S16, 0);
    samples_written = avresample_convert(this->audioResampleContext, &output, out_linesize, frame->linesize[0], &frame->data[0], 0, frame->linesize[0]);

    return output;
}
