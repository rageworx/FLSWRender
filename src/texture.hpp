#ifndef __TEXTURE_H__
#define __TEXTURE_H__
#pragma once

#include "mathes.hpp"

class Texture
{
    public:
        Texture() { };
        Texture(const char* path);
        ~Texture();

    public:
        Color    GetPixel(float u,float v);
        
    public:
        void     assign( unsigned char* p, unsigned w, unsigned h, unsigned d );
        int      w() { return width; }
        int      h() { return height; }
        int      d() { return channels; }
        size_t   size() { return pixels.size(); }

    private:
        int     width;
        int     height;
        int     channels;

    protected:
        std::vector<unsigned char> pixels;
};

#endif /// of __TEXTURE_H__
