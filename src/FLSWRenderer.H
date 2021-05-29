#ifndef __FLSWRENDERER_H__
#define __FLSWRENDERER_H__
#pragma once

/*******************************************************************************
*
*  FLTK Software 3D Renderer
*  ========================================
*  (C)Copyrighted 2021, Raphael Kim
*  [ source at ] 
*
*******************************************************************************/

#include <FL/Fl.H>
#include <FL/Fl_RGB_Image.H>

#include "geometry.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

class FLSWRenderer
{
    public:
        FLSWRenderer( Fl_RGB_Image* target = NULL );
        ~FLSWRenderer() { deinit(); }
    
    public:
        bool LoadObjects( const char* objfn = NULL );
        bool LoadObjectsIndir( const char* objdata = NULL, size_t datalen = 0 );
        bool LoadTexture( const char* txtfn = NULL );
        bool LoadTexture( Fl_RGB_Image* texture );
        bool Render();

    public:
        sceneparam* parameter() { return &senceParam; }

    protected:
        void init();
        void deinit();

    protected:
        sceneparam      senceParam;
        
    private:
        float*          zbuffer;
        Mesh*           mesh;
        Texture*        texture;
        Fl_RGB_Image*   rtarget;
        
    private:
        void  clearBuffer();
        void  rasterizeTriangle(vec4f SV_Vertexs[3],Shader& shader);
        vec3f barycentric(vec2f A,vec2f B,vec2f C,vec2f P);
        inline \
        void  setPixel(const int& x,const int& y,const Color& col);
        void  drawLine(const vec2i& p0,const vec2i& p1,const Color& col);
};

#endif /// of __FLSWRENDERER_H__
