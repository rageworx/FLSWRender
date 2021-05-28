#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstdint>
#include <omp.h>

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
		return Color(0, 0, 0);
	}

	int x = u * (width-1); 
	int y = v * (height-1);
    
    size_t q = (y*width+x)*channels;

    int r = 255;
    int g = 255;
    int b = 255;
    
    // prevent out of range -
    if ( q < pixels.size() )
    {
        r = pixels[q];
        g = pixels[q+1];
        b = pixels[q+2];
    }
    
    return Color(r,g,b);
}
