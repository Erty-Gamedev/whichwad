#pragma once

/*
	Based on the Unofficial BSP v30 File Spec by dixxi1 (Bernhard Gruber)
	https://web.archive.org/web/20240313170323/https://hlbsp.sourceforge.net/index.php?content=bspdef
*/
namespace BSPFormat
{
	static constexpr int c_MaxMapHulls =			  4;
	static constexpr int c_MaxMapModels =           400;
	static constexpr int c_MaxMapBrushes =         4096;
	static constexpr int c_MaxMapEntities =        1024;
	static constexpr int c_MaxMapEntString = 128 * 1024;
	static constexpr int c_MaxMapPlanes =		  32767;
	static constexpr int c_MaxMapNodes =		  32767;
	static constexpr int c_MaxMapClipNodes =	  32767;
	static constexpr int c_MaxMapLeaves =		   8192;
	static constexpr int c_MaxMapVertices =		  65535;
	static constexpr int c_MaxMapFaces =		  65535;
	static constexpr int c_MaxMapMarkSurfaces =	  65535;
	static constexpr int c_MaxMapTexInfo =		   8192;
	static constexpr int c_MaxMapEdges =		 256000;
	static constexpr int c_MaxMapSurfEdges =	 512000;
	static constexpr int c_MaxMapTextures =		    512;
	static constexpr int c_MaxMapMipTex =	   0x200000;
	static constexpr int c_MaxMapLighting =    0x200000;
	static constexpr int c_MaxMapVisibility =  0x200000;
	static constexpr int c_MaxMapPortals =		  65535;

	enum LumpIndex
	{
		Entities =		 0,
		Planes =		 1,
		Textures =		 2,
		Vertices =		 3,
		Visibility =	 4,
		Nodes =			 5,
		Texinfo =		 6,
		Faces =			 7,
		Lighting =		 8,
		Clipnodes =		 9,
		Leaves =		10,
		Marksurfaces =	11,
		Edges =			12,
		Surfedges =		13,
		Models =		14,
		Headerlumps =	15
	};

#pragma pack(push, 1)
	struct BspLump
	{
		std::int32_t offset;
		std::int32_t length;
	};

	struct BspHeader
	{
		std::int32_t version;
		BspLump lumps[Headerlumps];
	};
#pragma pack(pop)
}
