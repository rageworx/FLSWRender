#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__
#pragma once

#include "mathes.hpp"

typedef struct _sceneparam
{
	// Model attributes : MESH
	vec3f meshMove;	 /// The model is shifted in world space
	vec3f meshRotate;	 /// Model rotation value
	vec3f meshScale;	 /// model scale value

	// Light attributes
	vec3f light;		/// position of light

	// camera properties
	vec3f eye;			/// The position of the camera in world space
	vec3f at;			/// Camera viewpoint
	vec3f up;			/// Camera upward direction vector
    
    // scene config-
	float aspect;		/// Aspect ratio
	float fovY;		    /// Field of view
	float farZ;		    /// Near plane
	float nearZ;		/// Far plane 
}sceneparam;

namespace geometry
{
	mat computeModelMatrix(const sceneparam&);
	mat computeViewMatrix(const sceneparam&);
	mat computeProjectMatrix(const sceneparam&);	
	mat computeViewportMatrix(float,float,float,float,int,int);
	mat computeMVP(const sceneparam&);
};

#endif /// of __GEOMETRY_H__