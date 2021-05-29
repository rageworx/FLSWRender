#include "FLSWRenderer.H"
#ifndef __APPLE__
#include <omp.h>
#endif

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

class renderContext
{
    public:
        renderContext()
            : img(NULL), zbuffer(NULL), mesh(NULL), texture(NULL) {};
        ~renderContext() {};

    public:
        Fl_RGB_Image*   img;
        float*          zbuffer;
        Mesh*           mesh;
        Texture*        texture;

    public:
        void  clearBuffer();
        void  rasterizeTriangle(vec4f SV_Vertexs[3],Shader& shader);
        vec3f barycentric(vec2f A,vec2f B,vec2f C,vec2f P);
        inline \
        void  setPixel(const int& x,const int& y,const Color& col);
        void  drawLine(const vec2i& p0,const vec2i& p1,const Color& col);
};

#define TORCTX(_x_) renderContext* _x_ = (renderContext*)ctx

FLSWRenderer::FLSWRenderer( Fl_RGB_Image* target )
 : rtarget( target ),
   ctx( NULL )
{
    if ( rtarget != NULL )
    {
        if ( ( rtarget->w() == 0 ) || ( rtarget->h() == 0 ) || ( rtarget->d() < 3 ) )
        {
            // not supported target.
            rtarget = NULL;
        }
    }

    // setup context.
    renderContext* rctx = new renderContext();
    if ( rctx != NULL )
    {
        rctx->img = rtarget;
        ctx = (void*)rctx;
    }

    init();
}
        
FLSWRenderer::~FLSWRenderer() 
{ 
    deinit(); 

    TORCTX( rctx );
    if ( rctx != NULL )
    {
        ctx = NULL;
        delete rctx;
    }
}

bool FLSWRenderer::LoadObjects( const char* objfn )
{
    TORCTX( rctx );

    if ( rctx != NULL )
    {
        if ( rctx->mesh == NULL )
        {
            rctx->mesh = new Mesh();
        }
        
        if ( rctx->mesh != NULL )
        {
            if ( ObjParser::ParseMesh( objfn, rctx->mesh ) == true )
            {
                return true;
            }
            
            delete rctx->mesh;
            rctx->mesh = NULL;        
        }
    }
    return false;
}

bool FLSWRenderer::LoadObjectsIndir( const char* objdata, size_t datalen )
{
    TORCTX( rctx );

    if ( rctx != NULL )
    {
        if ( rctx->mesh == NULL )
        {
            rctx->mesh = new Mesh();
        }
        
        if ( rctx->mesh != NULL )
        {
            if ( ObjParser::ParseMesh( objdata, datalen, rctx->mesh ) == true )
            {
                return true;
            }
            
            delete rctx->mesh;
            rctx->mesh = NULL;
        }
    }
    return false;
}

bool FLSWRenderer::FLSWRenderer::LoadTexture( const char* txtfn )
{
    TORCTX( rctx );

    if ( rctx != NULL )
    {
        if ( rctx->texture == NULL )
        {
            rctx->texture = new Texture( txtfn );
        }
        else
        {
            delete rctx->texture;
            
            rctx->texture = new Texture( txtfn );
        }
        
        if ( rctx->texture != NULL )
            if ( rctx->texture->size() > 0 )
                return true;
    }       
    return false;
}

bool FLSWRenderer::LoadTexture( Fl_RGB_Image* img )
{
    TORCTX( rctx );
    return false;
}

void FLSWRenderer::init()
{
    TORCTX( rctx );
    
    if ( ( rtarget != NULL ) && ( rctx != NULL ) )
    {
        if ( ( rtarget->w() > 0 ) && ( rtarget->h() > 0 ) )
        {
            rctx->zbuffer = new float[ rtarget->w() * rtarget->h() ];
        }
    }
    
    senceParam.meshMove    = {0.f,0.f,0.f};
    senceParam.meshRotate  = {0.f,0.f,0.f};
    senceParam.meshScale   = {1.f,1.f,1.f};
    senceParam.light       = {0.f,0.f,0.f};

    if ( rtarget != NULL )
    {
        senceParam.aspect = (float)rtarget->w() / (float)rtarget->h();
    }
    else
    {
        // default is 16:9 FHD.
        senceParam.aspect = 16.f/9.f;
    }
    
    senceParam.eye         = {0.0f,0.0f,-3.0f};
    senceParam.at          = {0.0f,0.0f,0.0f}; 
    senceParam.up          = {0.0f,1.0f,0.0f}; 

    senceParam.fovY        = 60;
    senceParam.farZ        = 0.1f;
    senceParam.nearZ       = 100.0f;
}

bool FLSWRenderer::Render()
{
    TORCTX( rctx );
    
    if ( ( rtarget == NULL ) || ( rctx == NULL ) )
        return false;
    
    TextureShader shader_texture; 
    
    if ( rctx->texture != NULL )
    {
        shader_texture.Tex = *(rctx->texture);
    }
    
    rctx->clearBuffer();

    shader_texture.MVP = geometry::computeMVP( senceParam );
        
    // -- not work -- #pragma omp parallel for
    for( size_t cnt=0; cnt<rctx->mesh->numFaces; cnt++ )
    {
        vec3f triangleVertex[3];
        vec3f triangleNormal[3];
        vec3f triangleUv[3];

        for( size_t cj=0; cj<3; cj++ )
        {
            triangleVertex[cj] = \
                rctx->mesh->vertexs[rctx->mesh->faceVertexIndex[cnt].raw[cj] -1];
            triangleNormal[cj] = \
                rctx->mesh->vertexsNormal[rctx->mesh->faceNormalIndex[cnt].raw[cj] -1];
            triangleUv[cj]     = \
                rctx->mesh->vertexTextures[rctx->mesh->faceTextureIndex[cnt].raw[cj] -1];
        }

        vec4f SV_Vertex[3];
        
        for( size_t cj=0; cj<3; ++cj )
        {
            SV_Vertex[cj] = shader_texture.vertex( triangleVertex[cj],
                                                   triangleNormal[cj],
                                                   triangleUv[cj],
                                                   cj,
                                                   senceParam.light );
        }

        // Perspective division 
        for( size_t cj=0; cj<3; ++cj )
        {
            float re_w = 1.0f / SV_Vertex[cj].w;
            
            SV_Vertex[cj]   = SV_Vertex[cj] * re_w;
            SV_Vertex[cj].w = re_w;
        }

        rctx->rasterizeTriangle( SV_Vertex, shader_texture );
    }
    
    return true;
}

void FLSWRenderer::deinit()
{
    TORCTX( rctx );

    if ( rctx != NULL )
    {
        if ( rctx->zbuffer != NULL )
        {
            delete[] rctx->zbuffer;
            rctx->zbuffer = NULL;
        }
    }
}

/////////////////////////////////////////////////////////////////////////

void renderContext::clearBuffer()
{
    if ( img != NULL )
    {
        size_t cmax = img->w() * img->h();
        uchar* buff = (uchar*)img->data()[0];
        
        memset( buff, 0, cmax * img->d() );
        
        for ( size_t cnt=cmax; cnt--; zbuffer[cnt] = 10000.0f )
        {
            // nothing to do.
            buff[0] = 0;
        }
    }
}

void renderContext::rasterizeTriangle(vec4f SV_vertexs[3],Shader& shader)
{
    if ( img == NULL )
        return;

    float fw = (float)img->w();
    float fh = (float)img->h();

    // 1. Viewport conversion (-1,1) => (0,width/height) 
    // Simply mapped to the full screen, don¡¯t use the ViewPort matrix. 
    vec3f gl_coord[3];

    //Used for perspective interpolation correction 
    float re_w[3] = { 0.f };
    for( size_t cnt=0; cnt<3; cnt++ )
    {
        re_w[cnt]       = SV_vertexs[cnt].w;
        gl_coord[cnt].x = (SV_vertexs[cnt].x + 1.0f) * fw / 2.f;
        gl_coord[cnt].y = (SV_vertexs[cnt].y + 1.0f) * fh / 2.f;
        gl_coord[cnt].z = (SV_vertexs[cnt].z + 1.0f) / 2.f; 
    }

    // 2. Calculate the bounding box 
	float xMax = (std::max)({gl_coord[0].x, gl_coord[1].x, gl_coord[2].x});
	float xMin = (std::min)({gl_coord[0].x, gl_coord[1].x, gl_coord[2].x});
    float yMax = (std::max)({gl_coord[0].y, gl_coord[1].y, gl_coord[2].y});
    float yMin = (std::min)({gl_coord[0].y, gl_coord[1].y, gl_coord[2].y});

    xMax = (std::min)(xMax, fw -1.f );
    xMin = (std::max)(xMin, 0.f);
    yMax = (std::min)(yMax, fh -1.f );
    yMin = (std::max)(yMin, 0.f);

    int x = 0;
    int y = 0;
    
    // Traverse the pixels in the bounding box 
    for(x=xMin; x<int(xMax+1); x++)
    {
        for(y=yMin; y<int(yMax+1); y++)
        {
            // Calculate the center of gravity triangle 
            vec2f current_pixel = vec2f(x,y);
            vec2f A = vec2f(gl_coord[0].x,gl_coord[0].y);
            vec2f B = vec2f(gl_coord[1].x,gl_coord[1].y);
            vec2f C = vec2f(gl_coord[2].x,gl_coord[2].y);
            vec3f weight = barycentric(A,B,C,current_pixel);

            // Pixels not inside the triangle are skipped 
            if( weight.x<0 || weight.y<0 || weight.z< 0 ) 
                continue;

            // Depth interpolation is non-linear 
            // because there is no perspective correction 
            float currentDepth = weight.x * gl_coord[0].z
                                 + weight.y * gl_coord[1].z
                                 + weight.z * gl_coord[2].z;

            // Depth test 
            if( currentDepth > zbuffer[y*img->w()+x] ) 
                continue;

            // Depth sorting 
            zbuffer[y*img->w()+x] = currentDepth;

            // Perspective correction interpolation 
            float weight0 = re_w[0] * weight.x;
            float weight1 = re_w[1] * weight.y;
            float weight2 = re_w[2] * weight.z;
            vec3f lerpWeight = vec3f(weight0,weight1,weight2);
            Color fragmenCol = shader.fragment(lerpWeight);
            
            setPixel(x,y,fragmenCol);
        }
    }
}

vec3f renderContext::barycentric(vec2f A,vec2f B,vec2f C,vec2f P)
{
    vec2f ab = B - A;
    vec2f ac = C - A;
    vec2f ap = P - A;

    float factor = 1.f / (ab.x * ac.y - ab.y * ac.x);
    float s = (ac.y * ap.x - ac.x * ap.y) * factor;
    float t = (ab.x * ap.y - ab.y * ap.x) * factor;

    vec3f weights = vec3f(1.f - s - t, s, t);

    return weights;
}

inline void renderContext::setPixel(const int & x,const int & y,const Color & col)
{
    uchar* buff = (uchar*)img->data()[0];
    uchar *dst = &(buff[y * img->w() * img->d() + x*img->d()]);
    
    dst[0] = (uchar)(col.r);
    dst[1] = (uchar)(col.g);  
    dst[2] = (uchar)(col.b);
    
    if ( img->d() > 3 )
    {
        dst[3] = 0xFF;
    }
}

void renderContext::drawLine(const vec2i & p0,const vec2i & p1,const Color & col)
{
    int x0 = p0.x; 
    int x1 = p1.x;
    int y0 = p0.y; 
    int y1 = p1.y;
    bool steep = false;
        
    if (std::abs( x0 - x1 ) < std::abs( y0 - y1 ))
    {
        std::swap( x0 , y0 );
        std::swap( x1 , y1 );
        steep = true;
    }

    if (x0 > x1)
    {
        std::swap( x0 , x1 );
        std::swap( y0 , y1 );
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int dy2 = std::abs( dy ) * 2;
    int d = 0;
    int y = y0;

    for (int x = x0; x <= x1; x++)
    {
        if (steep)
        {
            setPixel( y , x , col );
        }
        else
        {
            setPixel( x , y , col );
        }

        d += dy2;
        if (d > dx)
        {
            y += ( y1 > y0 ? 1 : -1 );
            d -= dx * 2;
        }
    }
}
