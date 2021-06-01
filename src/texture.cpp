#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstdint>
#ifndef __APPLE__
#include <omp.h>
#endif 

using namespace std;

Texture::Texture(const char * path)
{
    stbi_set_flip_vertically_on_load(true); 
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

    pixels.resize( width*height*channels );
    
    #pragma omp parallel for
    for( size_t cnt=0; cnt<(width*height*channels); cnt++ )
    {
        pixels[cnt] = data[cnt];
    }

    stbi_image_free(data);
}

Texture::~Texture()
{
    pixels.clear();
    vector<unsigned char>().swap( pixels );
}

void Texture::assign( unsigned char* p, unsigned w, unsigned h, unsigned d )
{
    if ( ( p != NULL ) && ( w > 0 ) && ( h > 0 ) && ( d > 0 ) )
    {
        if ( pixels.size() > 0 )
        {
            pixels.clear();
            vector<unsigned char>().swap( pixels );
        }
        
        pixels.resize( w*h*d );
        
        #pragma omp parallel for
        for( size_t cnt=0; cnt<(w*h*d); cnt++ )
        {
            pixels[cnt] = p[cnt];
        }
        
        width = w;
        height = h;
        channels = d;
    }
}

Color Texture::GetPixel(float u,float v)
{
    if(u>1.f||v>1.f||u<0.f||v<0.f)
    {
        // returns no color (by alpha is zero )
        return Color(0, 0, 0, 0);
    }

    int x = u * (width-1); 
    int y = v * (height-1);
#if 1
    // returns white pixel if coordination is
    // out of boundary pixel or no-texture.
    int r = defaultcolor.r;
    int g = defaultcolor.g;
    int b = defaultcolor.b;
    int a = defaultcolor.a;

    size_t q = (y*width+x)*channels;
    
    // prevent out of range -
    if ( q < pixels.size() )
    {
        r = pixels[q];
        g = pixels[q+1];
        b = pixels[q+2];
        
        if ( channels > 3 )
            a = pixels[q+3];

        // where alpha belongs ... ?
    }
#else /// testing with error to bilinear filter ( do not eable it )
    float xerr = (u * (width-1)) - (float)x;
    float yerr = (v * (height-1)) - (float)y;
    
    /*
    printf( "GP( %f, %f ) => %d (err=%f), %d (err=%f)..\n",
            u, v, x, xerr, y, yerr );
    */
    int xr = 0;
    int yr = 0;
    if ( xerr > 0.5f ) xr++;
    if ( yerr > 0.5f ) yr++;

    // returns white pixel if coordination is
    // out of boundary pixel or no-texture.
    /*
    int r = defaultcolor.r;
    int g = defaultcolor.g;
    int b = defaultcolor.b;
    int a = defaultcolor.a;
    */
    int r = 0;
    int g = 0;
    int b = 0;
    int a = 0;
    int d = 0;
    
    for( int yy=y; yy<y+yr; yy++ )
    for( int xx=x; xx<x+xr; xx++ )
    {
        size_t q = (yy*width+xx)*channels;
        // prevent out of range -
        if ( q < pixels.size() )
        {
            r += pixels[q];
            g += pixels[q+1];
            b += pixels[q+2];
            
            if ( channels > 3 )
                a += pixels[q+3];
            
            d++;
        }
    }
    
    if ( d > 1 )
    {
        r /= d;
        g /= d;
        b /= d;
        a /= d;
    }
#endif 
   
    return Color(r,g,b,a);
}

void Texture::SetColor( Color c )
{
    defaultcolor = c;
}

Color Texture::GetColor()
{
    return defaultcolor;
}