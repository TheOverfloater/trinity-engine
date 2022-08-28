/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Texture loader interface
Written by Andrew Lucas
*/

#if !defined ( TGALOADER_H )
#define TGALOADER_H
#if defined( _WIN32 )
#pragma once
#endif

#include "windows.h"
#include "gl/gl.h"
#include "pm_defs.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "dlight.h"
#include "parsemsg.h"
#include "cvardef.h"
#include "rendererdefs.h"

#define	MAX_WADFILES 12

#define DDS_MAGIC 0x20534444

#define DDSD_CAPS                   0x00000001
#define DDSD_PIXELFORMAT            0x00001000
#define DDPF_FOURCC                 0x00000004

#define D3DFMT_DXT1     '1TXD'    //  DXT1 compression texture format 
#define D3DFMT_DXT5     '5TXD'    //  DXT5 compression texture format 

typedef struct
{
    char            identification[4];                     // should be WAD2/WAD3
    int             numlumps;
    int             infotableofs;
} wadinfo_t;

typedef struct
{
    int             filepos;
    int             disksize;
    int             size;
    char            type;
    char            compression;
    char            pad1, pad2;
    char            name[16];
} lumpinfo_t;

struct wadfile_t
{
	byte *wadfile;
	wadinfo_t *info;

	lumpinfo_t *lumps;
	int numlumps;
};

struct tga_header_t
{
	byte	idlength;
	byte	colourmaptype;
	byte	datatypecode;
	byte	colourmaporigin[2]; //how come you have short ints there?
	byte	colourmaplength[2];
	byte	colourmapdepth;
	byte	x_origin[2];
	byte	y_origin[2];
	byte	width[2];
	byte	height[2];
	byte	bitsperpixel;
	byte	imagedescriptor;
};

struct dds_header_t 
{
	byte bMagic[4];
	byte bSize[4];
	byte bFlags[4];
	byte bHeight[4];
	byte bWidth[4];
	byte bPitchOrLinearSize[4];

	byte bPad1[52];

	byte bPFSize[4];
	byte bPFFlags[4];
	byte bPFFourCC[4];
};

/*
=======================
CTextureLoader

=======================
*/
class CTextureLoader
{
public:
	void			Init ( void );
	void			VidInit( void );
	void			Shutdown( void );
	
	bool			IsPowerOfTwo( int iWidth, int iHeight );
	void			WriteTGA( byte *pixels, int bpp, int width, int height, char *szpath );

	void			LoadWADFiles( void );
	void			FreeWADFiles( void );

	cl_texture_t	*LoadTexture( char* szFile, int iAltIndex = 0, bool bPrompt = false, bool bNoMip = false, bool bBorder = false );
	cl_texture_t	*LoadWADTexture( char *szTexture, int iAltIndex = 0 );
	cl_texture_t	*HasTexture( char *szFile );

	bool			LoadTGAFile( byte *pFile, cl_texture_t *pTexture, bool bNoMip, bool bBorder );
	bool			LoadDDSFile( byte *pFile, cl_texture_t *pTexture, bool bNoMip );
	void			LoadPallettedTexture( byte *data, byte *pal, cl_texture_t *pTexture );

	void			LoadTextureScript( void );
	bool			TextureHasFlag( char *szModel, char *szTexture, int iFlag );
public:
	PFNGLCOMPRESSEDTEXIMAGE2DARBPROC	glCompressedTexImage2DARB;

	cl_texture_t						m_pTextures[MAX_TGA_LOADER_TEXTURES];
	int									m_iNumTextures;

	texentry_t							m_pTextureEntries[MAX_CACHE_MODELS*64];
	int									m_iNumTextureEntries;

	wadfile_t							m_pWADFiles[MAX_WADFILES];
	int									m_iNumWADFiles;
};
extern CTextureLoader gTextureLoader;
#endif