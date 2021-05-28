#ifndef __SHADER_H__
#define __SHADER_H__
#pragma once

#include "mathes.hpp"
#include "texture.hpp"
#include <cstdio>

struct Shader
{
	virtual ~Shader(){ }
	virtual vec4f vertex(const vec3f& vertex,const vec3f& normal,
						 const vec3f& textureUV,
						int index,const vec3f& light) = 0;
	virtual Color fragment(vec3f& lerpCorrect) = 0;
};

struct SimpleShader : public Shader 
{
	mat MVP;
	float vary_Intensity;

	vec4f vertex( const vec3f& vertex,const vec3f& normal,
                  const vec3f& textureUV,
                  int index,const vec3f& light ) override
	{
		vary_Intensity = (std::max)(0.0f,normal*light);
		return (MVP * mat(vertex)).ToVec4f();
	}

	Color fragment(vec3f& lerpCorrect) override
    {
		int inten = 255*vary_Intensity;
		return Color(inten,inten,inten);
	}
};

struct TextureShader : public Shader
{
	mat     MVP;
	Texture Tex;
	// Use perspective correction 
	float   vary_Intensity[3];
	vec3f   vary_UVs[3];
	float   correct_vary_Intensity;
	vec3f   correct_vary_UV;

	vec4f vertex( const vec3f& vertex,const vec3f& normal,
                  const vec3f& textureUV,
                  int index,const vec3f& light ) override
	{
		vary_UVs[index] = textureUV;
		vary_Intensity[index] = (std::max)(0.0f,normal*light);
		return (MVP * mat(vertex)).ToVec4f();
	}

	Color fragment(vec3f& lerpCorrect) override
    {
		float normalizer = 1.f / ( lerpCorrect.x + lerpCorrect.y + lerpCorrect.z );
        
		correct_vary_Intensity = (  ( vary_Intensity[0] * lerpCorrect.x )
							      + ( vary_Intensity[1] * lerpCorrect.y )
						          + ( vary_Intensity[2] * lerpCorrect.z ) )
                                 * normalizer;
                                 
		correct_vary_UV =   ( vary_UVs[0] * lerpCorrect.x * normalizer )
						  + ( vary_UVs[1] * lerpCorrect.y * normalizer )
						  + ( vary_UVs[2] * lerpCorrect.z * normalizer );

        Color rgb  = Tex.GetPixel( correct_vary_UV.x, correct_vary_UV.y );

        // calculate intensity ( light to shadow )
        rgb.r = (float)rgb.r * ( 1.f - correct_vary_Intensity );
        rgb.g = (float)rgb.g * ( 1.f - correct_vary_Intensity );
        rgb.b = (float)rgb.b * ( 1.f - correct_vary_Intensity );
        
		return rgb;
	}
};

#endif /// of __SHADER_H__