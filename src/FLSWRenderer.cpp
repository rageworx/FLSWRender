#include "FLSWRenderer.H"
#ifndef __APPLE__
#include <omp.h>
#endif

#ifdef USE_FL_IMGTK
#include <fl_imgtk.h>
#endif

#include "geometry.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

class renderContext
{
    public:
        renderContext()
            : img(NULL), zbuffer(NULL), mesh(NULL), texture(NULL), 
              drawline(false), drawtexture(true) {};
        ~renderContext() {};

    public:
        Fl_RGB_Image*   img;
        float*          zbuffer;
        Mesh*           mesh;
        Texture*        texture;
        sceneparam      sceneParam;
        bool            drawline;
        bool            drawtexture;
        Color           linecolor;

    public:
        void  clearBuffer( bool earsebuff );
        void  rasterizeTriangle(vec4f SV_Vertexs[3],Shader& shader);
        vec3f barycentric(vec2f A,vec2f B,vec2f C,vec2f P);
        inline \
        void  setPixel(const int& x,const int& y,const Color& col);
        void  drawLine(const vec2i& p0,const vec2i& p1,const Color& col);
};

#define TORCTX(_x_) renderContext* _x_ = (renderContext*)ctx

FLSWRenderer::FLSWRenderer( Fl_RGB_Image* target )
 : rtarget( target ),
   ctx( NULL ),
   drawline( false ),
   drawtexture( true )
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
    if ( img != NULL )
    {
        TORCTX( rctx );

        if ( rctx != NULL )
        {
            if ( rctx->texture == NULL )
            {
                rctx->texture = new Texture();
            }
            else
            {
                delete rctx->texture;
                
                rctx->texture = new Texture();
            }
        }

        if ( rctx->texture == NULL )
            return false;
     
        bool alloc = false;
        unsigned d = img->d();
        uchar* rbuff = (uchar*)img->data()[0];

        if ( ( img->w() > 0 ) && ( img->h() > 0 ) && ( d > 0 ) )
        {
            unsigned imgsz = img->w() * img->h();
            switch( d )
            {
                case 1: /// gray.
                {
                    uchar* conv = new uchar[imgsz*3];
                    if ( conv == NULL )
                        return false;
                    #pragma omp parallel for
                    for( size_t cnt=0; cnt<imgsz; cnt++ )
                    {
                        conv[cnt*3] =\
                        conv[cnt*3+1] =\
                        conv[cnt*3+2] = rbuff[cnt];
                    }
                    d = 3;
                    rbuff = conv;
                    alloc = true;
                }
                break;

                case 2: /// gray+alpha
                {
                    uchar* conv = new uchar[imgsz*4];
                    if ( conv == NULL )
                        return false;
                    #pragma omp parallel for
                    for( size_t cnt=0; cnt<imgsz; cnt++ )
                    {
                        conv[cnt*4] =\
                        conv[cnt*4+1] =\
                        conv[cnt*4+2] = rbuff[cnt*2];
                        conv[cnt*4+3] = rbuff[cnt*2+1]; 
                    }
                    d = 4;
                    rbuff = conv;
                    alloc = true;
                }
                break;
            } /// of switch()

            rctx->texture->assign( rbuff, img->w(), img->h(), d );

            if ( alloc == true )
            {
                delete[] rbuff;
            }

            if ( rctx->texture->size() > 0 )
                return true;
        }
    }
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
    
    rctx->sceneParam.meshMove    = {0.f,0.f,0.f};
    rctx->sceneParam.meshRotate  = {0.f,0.f,0.f};
    rctx->sceneParam.meshScale   = {1.f,1.f,1.f};
    rctx->sceneParam.light       = {0.f,0.f,0.f};

    if ( rtarget != NULL )
    {
        rctx->sceneParam.aspect = (float)rtarget->w() / (float)rtarget->h();
    }
    else
    {
        // default is 16:9 FHD.
        rctx->sceneParam.aspect = 16.f/9.f;
    }
    
    rctx->sceneParam.eye         = {0.0f,0.0f,-3.0f};
    rctx->sceneParam.at          = {0.0f,0.0f,0.0f}; 
    rctx->sceneParam.up          = {0.0f,1.0f,0.0f}; 

    rctx->sceneParam.fovY        = 60;
    rctx->sceneParam.farZ        = 0.1f;
    rctx->sceneParam.nearZ       = 100.0f;
}

bool FLSWRenderer::Render( bool earsebuff )
{
    TORCTX( rctx );
    
    if ( ( rtarget == NULL ) || ( rctx == NULL ) )
        return false;
    
    if ( rctx->texture == NULL ) /// create empty one.
    {
        rctx->texture = new Texture();
    }

    // set default colors ...
    Color defcol = Color( (int)(( defaultcolor & 0xFF000000 ) >> 24 ),
                          (int)(( defaultcolor & 0x00FF0000 ) >> 16 ),
                          (int)(( defaultcolor & 0x0000FF00 ) >> 8  ),
                          (int)(  defaultcolor & 0x000000FF ) );
    rctx->texture->SetColor( defcol );
    Color testcol = rctx->texture->GetColor();
    defcol = Color( (int)(( dlinecolor & 0xFF000000 ) >> 24 ),
                    (int)(( dlinecolor & 0x00FF0000 ) >> 16 ),
                    (int)(( dlinecolor & 0x0000FF00 ) >> 8  ),
                    (int)(  dlinecolor & 0x000000FF ) );    
    rctx->linecolor = defcol;
    
    TextureShader shader_texture; 
        
    if ( rctx->texture != NULL )
    {
        shader_texture.Tex = *(rctx->texture);
    }

    rctx->clearBuffer( earsebuff );
    rctx->drawline = drawline;
    rctx->drawtexture = drawtexture;

    shader_texture.MVP = geometry::computeMVP( rctx->sceneParam );
    size_t numFaces = rctx->mesh->faceVertexIndex.size();
    
    // -- currently OpenMP not works --
    // #pragma omp parallel for shared( shader_texture )
    //for( size_t cnt=0; cnt<rctx->mesh->numFaces; cnt++ )
    for( size_t cnt=0; cnt<numFaces; cnt++ )
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
                                                   rctx->sceneParam.light );
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
    
    // not sure this uncaching affects actual result.
    //rtarget->uncache();

    return true;
}

void FLSWRenderer::color( unsigned color )
{
    defaultcolor = color;
}

unsigned FLSWRenderer::color()
{
    return defaultcolor;
}

void FLSWRenderer::linecolor( unsigned color )
{
    dlinecolor = color;
}

unsigned FLSWRenderer::linecolor()
{
    return dlinecolor;
}

void FLSWRenderer::line( bool onoff )
{
    drawline = onoff;
}

bool FLSWRenderer::line()
{
    return drawline;
}

void FLSWRenderer::texture( bool onoff )
{
    drawtexture = onoff;
}

bool FLSWRenderer::texture()
{
    return drawtexture;
}

size_t FLSWRenderer::vertexs()
{
    TORCTX( rctx );

    if ( rctx != NULL )
    {
        if ( rctx->mesh != NULL )
        {
            return rctx->mesh->vertexs.size();
        }
    }
    
    return 0;
}

size_t FLSWRenderer::faces()
{
    TORCTX( rctx );

    if ( rctx != NULL )
    {
        if ( rctx->mesh != NULL )
        {
            return rctx->mesh->faceVertexIndex.size();
        }
    }
    
    return 0;
}

size_t FLSWRenderer::texturecoords()
{
    TORCTX( rctx );

    if ( rctx != NULL )
    {
        if ( rctx->mesh != NULL )
        {
            return rctx->mesh->vertexTextures.size();
        }
    }
    
    return 0;
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

vec3f* FLSWRenderer::shift()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.meshMove;
    }

    return NULL;
}

vec3f* FLSWRenderer::rotate()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.meshRotate;
    }

    return NULL;
}

vec3f* FLSWRenderer::scale()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.meshScale;
    }

    return NULL;
}

vec3f* FLSWRenderer::light()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.light;
    }
    return NULL;
}

vec3f* FLSWRenderer::eye()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.eye;
    }
    return NULL;
}

vec3f* FLSWRenderer::lookat()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.at;
    }
    return NULL;
}

vec3f* FLSWRenderer::upside()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.up;
    }
    return NULL;
}

float* FLSWRenderer::aspectratio()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.aspect;
    }
    return NULL;
}

float* FLSWRenderer::FOV()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.fovY;
    }
    return NULL;
}

float* FLSWRenderer::FarPlane()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.farZ;
    }
    return NULL;
}

float* FLSWRenderer::NearPlane()
{
    TORCTX( rctx );
    if ( rctx != NULL )
    {
        return &rctx->sceneParam.nearZ;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

void renderContext::clearBuffer( bool earsebuff )
{
    if ( img != NULL )
    {
        size_t cmax = img->w() * img->h();
        uchar* buff = (uchar*)img->data()[0];
        
        if ( earsebuff == true )
            memset( buff, 0, cmax * img->d() );
        
        #pragma omp parallel for
        for ( size_t cnt=0; cnt<=cmax; cnt++ )
        {
            zbuffer[cnt] = 1000000.f; /// was 10000.0f;
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

    if ( drawtexture == true )
    {
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
        for( x=(int)xMin; x<int(xMax+1); x++)
        {
            for( y=(int)yMin; y<int(yMax+1); y++)
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
    
    if ( drawline == true )
    {
        vec2i xy1;
        vec2i xy2;
        size_t lq = 0;
        
        for( size_t cj=0; cj<3; cj++ )
        {
            xy1 = { (int)(gl_coord[lq].x), (int)(gl_coord[lq].y)};
            
            lq++;
            if ( lq == 2 ) lq = 0;
            
            xy2 = {(int)(gl_coord[lq].x), (int)(gl_coord[lq].y)};
            
#ifdef DEBUG_LINE_TRACE
            printf( "drawline( %d, %d, %d, %d ) ... \n",
                    xy1.x, xy1.y, xy2.x, xy2.y );
#endif                 
            drawLine( xy1, xy2, linecolor );
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
        dst[3] = (uchar)(col.a);
    }
}

void renderContext::drawLine(const vec2i & p0,const vec2i & p1,const Color & col)
{
    if ( img == NULL )
        return;
    
    int x0 = p0.x; 
    int x1 = p1.x;
    int y0 = p0.y; 
    int y1 = p1.y;
#ifdef USE_FL_IMGTK
    ulong flcol =   ( ( col.r & 0x000000FF ) << 24 )
                  | ( ( col.g & 0x000000FF ) << 16 ) 
                  | ( ( col.b & 0x000000FF ) << 8 )
                  | ( col.a & 0x000000FF );
    fl_imgtk::draw_smooth_line( img, x0, y0, x1, y1, flcol );
#else ///     
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
#endif
}
