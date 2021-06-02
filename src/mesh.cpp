#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "mesh.hpp"

using namespace std;

bool ObjParser::ParseMesh(const char* path, Mesh* mesh)
{
    ifstream in;

    in.open(path,ifstream::in);

    if( in.fail() )
    {
#ifdef DEBUG
        printf( "file %s cannot open\n", path );
#endif         
        return false;
    }

    string line;
    while (!in.eof())
    {
        char trash;
        getline(in,line);
        istringstream ssline(line.c_str());

        if (!line.compare(0,2,"v ")) //vertex
        {
            ssline >> trash; //push "v" in trash.
            vec3f vert;
            for (int i = 0; i < 3; i++)
            {
                float tmp;
                ssline >> tmp;
                // strange, actual coorddination must be negative.
                vert.raw[i] = -tmp;
            }
            mesh->vertexs.push_back(vert);
        }

        if(!line.compare(0,3,"vn "))
        {
            ssline >> trash >> trash; //push "vn" in trash.
            vec3f normal;
            for (int i = 0; i < 3; i++)
            {
                float tmp;
                ssline >> tmp;
                normal.raw[i] = tmp;
            }
            mesh->vertexsNormal.push_back(normal);
        }

        if(!line.compare(0,3,"vt "))
        {
            ssline >> trash >> trash; //push "vn" in trash.
            vec3f uv;
            for (int i = 0; i < 2; i++)
            {
                float tmp;
                ssline >> tmp;
                uv.raw[i] = tmp;
            }
            mesh->vertexTextures.push_back(uv);
        }

        if(!line.compare(0,2,"f "))
        {
            ssline >> trash; //push "f" in trash.

            int tmp[3][3];
            ssline >> tmp[0][0]>>trash>>tmp[1][0]>>trash>>tmp[2][0];
            ssline >> tmp[0][1]>>trash>>tmp[1][1]>>trash>>tmp[2][1];
            ssline >> tmp[0][2]>>trash>>tmp[1][2]>>trash>>tmp[2][2];

            mesh->faceVertexIndex.push_back(vec3i(tmp[0]));
            mesh->faceTextureIndex.push_back(vec3i(tmp[1]));
            mesh->faceNormalIndex.push_back(vec3i(tmp[2]));
        }
    }
    
#ifdef DEBUG
    printf( "vertexes : %lu, faces : %lu\n", 
            mesh->vertexs.size(), 
            mesh->faceVertexIndex.size() );
#endif /// of DEBUG

    in.clear();
    in.seekg(0, in.beg);
    in.close();
    
    return true;
}

vector<string> split(const string& s, char seperator)
{
    vector<string> output;
    string::size_type prev_pos = 0;
    string::size_type pos = 0;

    while( ( pos = s.find(seperator, pos)) != string::npos )
    {
        string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back( s.substr( prev_pos, pos-prev_pos ) ); // Last word

    return output;
}

bool ObjParser::ParseMesh(const char* data, size_t datalen, Mesh* mesh )
{
    if ( ( data == NULL ) || ( datalen == 0 ) || ( mesh == NULL ) )
        return false;
    
    string srcdata;
    srcdata.assign( data, datalen );
    vector<string> lines = split( srcdata, '\n' );

    for( size_t cnt=0; cnt<lines.size(); cnt++ )
    {
        char trash;
        istringstream ssline(lines[cnt].c_str());

        if (!lines[cnt].compare(0,2,"v "))
        {
            ssline >> trash;
            vec3f vert;
            for (int i = 0; i < 3; i++)
            {
                float tmp;
                ssline >> tmp;
                vert.raw[i] = tmp;
            }
            mesh->vertexs.push_back(vert);
        }

        if(!lines[cnt].compare(0,3,"vn "))
        {
            ssline >> trash >> trash;
            vec3f normal;
            for (int i = 0; i < 3; i++)
            {
                float tmp;
                ssline >> tmp;
                normal.raw[i] = tmp;
            }
            mesh->vertexsNormal.push_back(normal);
        }

        if(!lines[cnt].compare(0,3,"vt "))
        {
            ssline >> trash >> trash;
            vec3f uv;
            for (int i = 0; i < 2; i++)
            {
                float tmp;
                ssline >> tmp;
                uv.raw[i] = tmp;
            }
            mesh->vertexTextures.push_back(uv);
        }

        if(!lines[cnt].compare(0,2,"f "))
        {
            ssline >> trash;

            int tmp[3][3];
            ssline >> tmp[0][0]>>trash>>tmp[1][0]>>trash>>tmp[2][0];
            ssline >> tmp[0][1]>>trash>>tmp[1][1]>>trash>>tmp[2][1];
            ssline >> tmp[0][2]>>trash>>tmp[1][2]>>trash>>tmp[2][2];

            mesh->faceVertexIndex.push_back(vec3i(tmp[0]));
            mesh->faceTextureIndex.push_back(vec3i(tmp[1]));
            mesh->faceNormalIndex.push_back(vec3i(tmp[2]));
        }
    }

#ifdef DEBUG
    printf( "vertexes : %lu, faces : %lu\n", 
            mesh->vertexs.size(),
            mesh->faceVertexIndex.size() );
#endif /// of DEBUG
    
    return true;    
}
