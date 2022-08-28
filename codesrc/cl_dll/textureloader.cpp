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

#include "hud.h"
#include "textureloader.h"
#include "bsprenderer.h"
#include "propmanager.h"
#include <windows.h>
#include "gl/glext.h"

#pragma warning( disable: 4018 )

/*
====================
Init

====================
*/
void CTextureLoader::Init( void )
{
	glCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)wglGetProcAddress("glCompressedTexImage2DARB");
}

/*
====================
VidInit

====================
*/
void CTextureLoader::VidInit( void )
{
	if(!m_iNumTextures)
		return;

	for(int i = 0; i < m_iNumTextures; i++)
		glDeleteTextures(1, &m_pTextures[i].iIndex);

	memset(m_pTextures, NULL, sizeof(m_pTextures));
	m_iNumTextures = NULL;
}

/*
====================
Shutdown

====================
*/
void CTextureLoader::Shutdown( void )
{
	VidInit();
}

/*
====================
IsPowerOfTwo

====================
*/
bool CTextureLoader::IsPowerOfTwo(int iWidth,int iHeight)
{
	int iWidthT = iWidth;
	while(iWidthT != 1)
	{
		if((iWidthT % 2) != 0) return false;
		iWidthT /=2;
	}

	int iHeightT = iHeight;
	while(iHeightT != 1)
	{
		if((iHeightT % 2) != 0) return false;
		iHeightT /=2;
	}
	return true;
}

/*
====================
LoadTexture

====================
*/
cl_texture_t *CTextureLoader::LoadTexture(char *szFile, int iAltIndex, bool bPrompt, bool bNoMip, bool bBorder)
{
	int iType = 0;
	char szAlt[64];
	byte *pFile = NULL;

	if(strlen(szFile) >= 64)
	{
		gEngfuncs.Con_Printf("Token too large on %s.\n", szFile);
		return NULL;
	}

	// Try and find a match
	cl_texture_t *pTexture = NULL;
	
	if(!iAltIndex)
		pTexture = HasTexture(szFile);
	
	if(pTexture)
	{
		// Just return regular ones if already loaded
		return pTexture;
	}

	// Some files need to be .tga
	if(!strcmp(&szFile[strlen(szFile)-3], "dds"))
		pFile = (byte *)gEngfuncs.COM_LoadFile(szFile, 5, NULL);

	if(!pFile)
	{	
		// Check for .tga then
		strcpy(szAlt, szFile);
		strcpy(&szAlt[strlen(szAlt)-3], "tga");

		pFile = (byte *)gEngfuncs.COM_LoadFile(szAlt, 5, NULL);
		iType = 1;
	}

	if(!pFile)
	{	
		if(bPrompt) gEngfuncs.Con_Printf("Failed to load image: %s\n", szFile);
		else gEngfuncs.Con_DPrintf("Failed to load image: %s\n", szFile);
		return NULL;
	}

	//
	// Allocate cache
	//
	pTexture = &m_pTextures[m_iNumTextures];
	m_iNumTextures++;

	if(!iAltIndex)
	{
		pTexture->iIndex = current_ext_texture_id;
		current_ext_texture_id++;
	}
	else
	{
		pTexture->iIndex = iAltIndex;
	}

	// Copy the name over
	strcpy(pTexture->szName, szFile);

	// Load DDS file
	if(iType == 0)
	{
		if(!LoadDDSFile(pFile, pTexture, bNoMip))
		{
			gEngfuncs.Con_Printf("Error! Failed to load: %s.\n", szFile);
			gEngfuncs.COM_FreeFile(pFile);

			memset(pTexture, 0, sizeof(cl_texture_t));
			m_iNumTextures--;
			return NULL;
		}
	}
	else if(iType == 1)
	{
		if(!LoadTGAFile(pFile, pTexture, bNoMip, bBorder))
		{
			gEngfuncs.Con_Printf("Error! Failed to load: %s.\n", szFile);
			gEngfuncs.COM_FreeFile(pFile);

			memset(pTexture, 0, sizeof(cl_texture_t));
			m_iNumTextures--;
			return NULL;
		}
	}

	gEngfuncs.COM_FreeFile(pFile);
	return pTexture;
}

/*
====================
LoadTGAFile

====================
*/
bool CTextureLoader::LoadTGAFile( byte *pFile, cl_texture_t *pTexture, bool bNoMip, bool bBorder )
{
	// Set basic information
	tga_header_t *pHeader = (tga_header_t *)pFile;
	if(pHeader->datatypecode != 2 && pHeader->datatypecode != 10
		|| pHeader->bitsperpixel != 24 && pHeader->bitsperpixel != 32)
	{
		gEngfuncs.Con_Printf("Error! %s is using a non-supported format. Only 24 bit and 32 bit true color formats are supported.\n", pTexture->szName);
		return false;
	}

	pTexture->iWidth = ByteToUShort(pHeader->width);
	pTexture->iHeight = ByteToUShort(pHeader->height);

	if(!IsPowerOfTwo(pTexture->iWidth, pTexture->iHeight))
	{
		gEngfuncs.Con_Printf("Error! %s is not a power of two texture!\n", pTexture->szName);
		return false;
	}

	// Allocate data
	pTexture->iBpp = pHeader->bitsperpixel/8;
	int iSize = pTexture->iWidth*pTexture->iHeight;
	int iImageSize = iSize*pTexture->iBpp;

	byte *pOriginal = new byte[iImageSize];
	memset(pOriginal, 0, sizeof(byte)*iImageSize);

	// Load based on type
	byte *pCurrent = pFile+18;
	if(pHeader->datatypecode == 2)
	{
		//Uncompressed TGA
		if(pTexture->iBpp == 3)
		{
			for(int i = 0; i < iImageSize; i += 3)
			{
				pOriginal[i] = pCurrent[i+2];
				pOriginal[i+1] = pCurrent[i+1];
				pOriginal[i+2] = pCurrent[i];
			}
		}
		else if(pTexture->iBpp == 4)
		{
			for(int i = 0; i < iImageSize; i += 4)
			{
				pOriginal[i] = pCurrent[i+2];
				pOriginal[i+1] = pCurrent[i+1];
				pOriginal[i+2] = pCurrent[i];
				pOriginal[i+3] = pCurrent[i+3];
			}
		}
	}
	else
	{
		// RLE Compression
		int i = 0;
		if(pTexture->iBpp == 3)
		{
			while(i < iImageSize)
			{
				if(*pCurrent & 0x80)
				{
					byte bLength = *pCurrent-127;
					pCurrent++;

					for(int j = 0; j < bLength; j++, i+= pTexture->iBpp)
					{
						pOriginal[i] = pCurrent[2];
						pOriginal[i+1] = pCurrent[1];
						pOriginal[i+2] = pCurrent[0];
					}
					
					pCurrent += pTexture->iBpp;
				}
				else
				{
					byte bLength = *pCurrent+1;
					pCurrent++;

					for(int j = 0; j < bLength; j++, i+= pTexture->iBpp, pCurrent += pTexture->iBpp)
					{
						pOriginal[i] = pCurrent[2];
						pOriginal[i+1] = pCurrent[1];
						pOriginal[i+2] = pCurrent[0];
					}
				}
			}
		}
		else
		{
			while(i < iImageSize)
			{
				if(*pCurrent & 0x80)
				{
					byte bLength = *pCurrent-127;
					pCurrent++;

					for(int j = 0; j < bLength; j++, i+= pTexture->iBpp)
					{
						pOriginal[i] = pCurrent[2];
						pOriginal[i+1] = pCurrent[1];
						pOriginal[i+2] = pCurrent[0];
						pOriginal[i+3] = pCurrent[3];
					}
					
					pCurrent += pTexture->iBpp;
				}
				else
				{
					byte bLength = *pCurrent+1;
					pCurrent++;

					for(int j = 0; j < bLength; j++, i+= pTexture->iBpp, pCurrent += pTexture->iBpp)
					{
						pOriginal[i] = pCurrent[2];
						pOriginal[i+1] = pCurrent[1];
						pOriginal[i+2] = pCurrent[0];
						pOriginal[i+3] = pCurrent[3];
					}
				}
			}
		}
	}

	// Flip vertically
	byte *pFlipped = new byte[iImageSize];
	for(int i = 0; i < pTexture->iHeight; i++)
	{
		GLubyte *dst = pFlipped + i*pTexture->iWidth*pTexture->iBpp;
		GLubyte *src = pOriginal + (pTexture->iHeight-i-1)*pTexture->iWidth*pTexture->iBpp;
		memcpy(dst, src, sizeof(GLubyte)*pTexture->iWidth*pTexture->iBpp);
	}

	//Add border if asked to
	if(bBorder)
	{
		byte *pCurrent = pFlipped;
		for(int i = 0; i < pTexture->iHeight; i++)
		{
			for(int j = 0; j < pTexture->iWidth; j++)
			{
				if(i == 0 || i == (pTexture->iHeight-1) || j == 0 || j == (pTexture->iWidth-1))
				{
					pCurrent[0] = 0;
					pCurrent[1] = 0;
					pCurrent[2] = 0;
				}

				if(pTexture->iBpp == 3) pCurrent += 3;
				else pCurrent += 4;
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D, pTexture->iIndex);

	if(bNoMip)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(pTexture->iBpp == 3)
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, pTexture->iWidth, pTexture->iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pFlipped );
	else
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, pTexture->iWidth, pTexture->iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pFlipped );

	delete [] pOriginal;
	delete [] pFlipped;
	return true;
}

/*
====================
LoadDDSFile

====================
*/
bool CTextureLoader::LoadDDSFile(byte *pFile, cl_texture_t *pTexture, bool bNoMip )
{
	dds_header_t *pHeader = (dds_header_t *)pFile;
	unsigned int iFlags = ByteToUInt(pHeader->bFlags);
	unsigned int iMagic = ByteToUInt(pHeader->bMagic);
	unsigned int iFourCC = ByteToUInt(pHeader->bPFFourCC);
	unsigned int iPFFlags = ByteToUInt(pHeader->bPFFlags);
	unsigned int iLinSize = ByteToUInt(pHeader->bPitchOrLinearSize);
	unsigned int iSize = ByteToUInt(pHeader->bSize);

	pTexture->iWidth = ByteToUInt(pHeader->bWidth);
	pTexture->iHeight = ByteToUInt(pHeader->bHeight);

	if(!IsPowerOfTwo(pTexture->iWidth, pTexture->iHeight))
	{
		gEngfuncs.Con_Printf("Error! %s is not a power of two texture!\n", pTexture->szName);
		return false;
	}
		
	if(iMagic != DDS_MAGIC)
		return false; // Not DDS file

	if(iSize != 124)
		return false; // Not correct size

	if(!(iFlags & DDSD_PIXELFORMAT))
		return false; // Not correct format

	if(!(iFlags & DDSD_CAPS))
		return false; // Not correct format

	if(!(iPFFlags & DDPF_FOURCC))
		return false; // Not correct type

	if(iFourCC != D3DFMT_DXT1 && iFourCC != D3DFMT_DXT5)
	{
		gEngfuncs.Con_Printf("Error! Incorrect compression on: %s! Only DXT1 and DXT5 are supported!\n", pTexture->szName);
		return false; // Not correct compression
	}

	// Copy data over
	byte *pData = new byte[iLinSize];
	memcpy(pData, (pFile+128), iLinSize);

	glBindTexture(GL_TEXTURE_2D, pTexture->iIndex);
	if(bNoMip)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Upload to OpenGL
	glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, (iFourCC == D3DFMT_DXT1) ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, pTexture->iWidth, pTexture->iHeight, 0, iLinSize, pData);

	delete [] pData;
	return true;
}

/*
====================
HasTexture

====================
*/
cl_texture_t *CTextureLoader::HasTexture( char *szFile )
{
	for(int i = 0; i < m_iNumTextures; i++)
	{
		if(!strcmp(m_pTextures[i].szName, szFile))
			return &m_pTextures[i];
	}
	return NULL;
}

/*
====================
LoadWADFiles

====================
*/
void CTextureLoader::LoadWADFiles( void )
{
	char szWAD[64];
	char szFile[32];

	char *pWAD = gPropManager.ValueForKey(&gPropManager.m_pBSPEntities[0], "wad");
	if(!pWAD)
		return;

	int iLength = strlen(pWAD);
	int iCur = 0;;
	while(1)
	{
		if(iCur >= iLength)
			return;

		int iLen = 0;
		while(1)
		{
			if(pWAD[iCur] == ';' || iCur >= iLength)
			{
				szWAD[iLen] = 0;
				iLen++; iCur++;
				break;
			}

			szWAD[iLen] = pWAD[iCur];
			iCur++; iLen++;
		}

		FilenameFromPath(szWAD, szFile);
		strcat(szFile, ".wad");
	
		int iSize = 0;
		byte *pFile = gEngfuncs.COM_LoadFile(szFile, 5, &iSize);
		if(!pFile)
			continue;

		wadinfo_t *pInfo = (wadinfo_t *)pFile;
		if(strncmp("WAD3", pInfo->identification, 4))
		{
			gEngfuncs.COM_FreeFile(pFile);
			continue;
		}

		wadfile_t *pWADFile = &m_pWADFiles[m_iNumWADFiles];
		m_iNumWADFiles++;

		pWADFile->wadfile = pFile;
		pWADFile->info = (wadinfo_t *)pWADFile->wadfile;

		pWADFile->lumps = new lumpinfo_t[pWADFile->info->numlumps];
		memcpy(pWADFile->lumps, (pWADFile->wadfile+pWADFile->info->infotableofs), sizeof(lumpinfo_t)*pWADFile->info->numlumps);
		pWADFile->numlumps = pWADFile->info->numlumps;
	}
}

/*
====================
FreeWADFiles

====================
*/
void CTextureLoader::FreeWADFiles( void )
{
	if(!m_iNumWADFiles)
		return;

	for(int i = 0; i < m_iNumWADFiles; i++)
	{
		delete [] m_pWADFiles[i].lumps;
		gEngfuncs.COM_FreeFile(m_pWADFiles[i].wadfile);
	}

	memset(m_pWADFiles, 0, sizeof(m_pWADFiles));
	m_iNumWADFiles = 0;
}

/*
====================
LoadWADTexture

====================
*/
cl_texture_t *CTextureLoader::LoadWADTexture( char *szTexture, int iAltIndex )
{
	char szName[32];
	cl_texture_t *pTexture = NULL;

	for(int i = 0; i < m_iNumWADFiles; i++)
	{
		byte *pFile = m_pWADFiles[i].wadfile;
		wadinfo_t *pInfo = m_pWADFiles[i].info;
		for(int j = 0; j < pInfo->numlumps; j++)
		{
			lumpinfo_t *pLump = &m_pWADFiles[i].lumps[j];
			if(pLump->type != 0 && !(pLump->type & 0x43))
				continue;

			strcpy(szName, pLump->name);
			strLower(szName);

			if(!strcmp(szName, szTexture))
			{
				pTexture = &m_pTextures[m_iNumTextures];
				m_iNumTextures++;

				// Fill in data
				strcpy(pTexture->szName, szTexture);
				pTexture->iWidth = ByteToUInt(pFile+pLump->filepos+16);
				pTexture->iHeight = ByteToUInt(pFile+pLump->filepos+20);
				pTexture->iBpp = 4;

				// Get offsets
				int iIndexOffset = ByteToUInt(pFile+pLump->filepos+24);
				int iMip3Offset = ByteToUInt(pFile+pLump->filepos+36);	

				byte *pPalette;
				if(pLump->type & 0x43) pPalette = pFile+pLump->filepos+iMip3Offset+((pTexture->iWidth/8)*(pTexture->iHeight/8))+2;
				else pPalette = pFile+pLump->filepos+iIndexOffset+(pTexture->iWidth*pTexture->iHeight)+2; 

				if(iAltIndex) pTexture->iIndex = iAltIndex;
				byte *pPixels = pFile+pLump->filepos+iIndexOffset;
				LoadPallettedTexture(pPixels, pPalette, pTexture);
				return pTexture;
			}
		}
	}

	return NULL;
}

/*
====================
LoadPallettedTexture

====================
*/
void CTextureLoader::LoadPallettedTexture( byte *data, byte *pal, cl_texture_t *pTexture )
{
	int		row1[1024], row2[1024], col1[1024], col2[1024];
	byte	*pix1, *pix2, *pix3, *pix4;
	byte	alpha1, alpha2, alpha3, alpha4;

	// convert texture to power of 2
	int outwidth;
	for (outwidth = 1; outwidth < pTexture->iWidth; outwidth <<= 1);
	if (outwidth > 1024) outwidth = 1024;

	int outheight;
	for (outheight = 1; outheight < pTexture->iHeight; outheight <<= 1);
	if (outheight > 1024) outheight = 1024;

	byte	*out, *tex;
	tex = out = new byte[outwidth*outheight*4];

	for (int i = 0; i < outwidth; i++)
	{
		col1[i] = (int) ((i + 0.25) * (pTexture->iWidth / (float)outwidth));
		col2[i] = (int) ((i + 0.75) * (pTexture->iWidth / (float)outwidth));
	}

	for (int i = 0; i < outheight; i++)
	{
		row1[i] = (int) ((i + 0.25) * (pTexture->iHeight / (float)outheight)) * pTexture->iWidth;
		row2[i] = (int) ((i + 0.75) * (pTexture->iHeight / (float)outheight)) * pTexture->iWidth;
	}

	for (int i = 0; i<outheight; i++)
	{
		for (int j = 0; j<outwidth; j++, out += 4)
		{
			pix1 = &pal[data[row1[i] + col1[j]] * 3];
			pix2 = &pal[data[row1[i] + col2[j]] * 3];
			pix3 = &pal[data[row2[i] + col1[j]] * 3];
			pix4 = &pal[data[row2[i] + col2[j]] * 3];
			alpha1 = 0xFF; alpha2 = 0xFF; alpha3 = 0xFF; alpha4 = 0xFF;

			if(pTexture->szName[0] == '{')
			{
				if (data[row1[i] + col1[j]] == 0xFF) 
				{
					pix1[0] = 0; pix1[1] = 0; pix1[2] = 0; alpha1 = 0;							
				} 

				if (data[row1[i] + col2[j]] == 0xFF) 
				{
					pix2[0] = 0; pix2[1] = 0; pix2[2] = 0; alpha2 = 0;				
				} 

				if (data[row2[i] + col1[j]] == 0xFF) 
				{
					pix3[0] = 0; pix3[1] = 0; pix3[2] = 0; alpha3 = 0;
				} 

				if (data[row2[i] + col2[j]] == 0xFF) 
				{
					pix4[0] = 0; pix4[1] = 0; pix4[2] = 0; alpha4 = 0;
				}
			}

			out[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			out[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			out[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			out[3] = (alpha1 + alpha2 + alpha3 + alpha4)>>2;
		}
	}

	if(!pTexture->iIndex)
	{
		pTexture->iIndex = current_ext_texture_id;
		current_ext_texture_id++;
	}

	glBindTexture( GL_TEXTURE_2D, pTexture->iIndex ); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, outwidth, outheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex );

	delete [] tex;
}

/*
====================
LoadTextureScript

====================
*/
void CTextureLoader::LoadTextureScript( void )
{
	int iFlags = 0;
	char szFlag[32];
	char szModel[32];
	char szTexture[32];

	//Clear previous list
	if(m_iNumTextureEntries)
	{
		memset(m_pTextureEntries, 0, sizeof(m_pTextureEntries));
		m_iNumTextureEntries = NULL;
	}

	int iSize = NULL;
	char *pFile = (char *)gEngfuncs.COM_LoadFile("gfx/textures/texture_flags.txt", 5, &iSize);

	if(!pFile)
	{
		gEngfuncs.Con_Printf("Could not load gfx/textures/texture_flags.txt!\n");
		return;
	}

	int i = NULL;
	while(1)
	{
		// Reset
		iFlags = 0;

		// Reached EOF
		if(i >= iSize)
			break;

		// Skip to next token
		while(1)
		{
			if(i >= iSize)
				break;

			if(pFile[i] != ' ' && pFile[i] != '\n' && pFile[i] != '\r')
				break;

			i++;
		}

		if(i >= iSize)
			break;

		// Read token in
		int j = NULL;
		while(1)
		{
			if(i >= iSize)
				break;

			if(pFile[i] == ' ' || pFile[i] == '\n' || pFile[i] == '\r')
				break;

			szModel[j] = pFile[i];
			j++; i++;
		}

		//Terminator
		szModel[j] = 0;

		if(i >= iSize)
			break;

		// Skip to next token
		while(1)
		{
			if(i >= iSize)
				break;

			if(pFile[i] != ' ' && pFile[i] != '\n' && pFile[i] != '\r')
				break;

			i++;
		}

		if(i >= iSize)
			break;

		// Read token in
		j = NULL;
		while(1)
		{
			if(i >= iSize)
				break;

			if(pFile[i] == ' ' || pFile[i] == '\n' || pFile[i] == '\r')
				break;

			szTexture[j] = pFile[i];
			j++; i++;
		}

		//Terminator
		szTexture[j] = 0;

		if(i >= iSize)
			break;

		while(1)
		{
			// Skip to next token
			while(1)
			{
				if(i >= iSize)
					break;

				if(pFile[i] != ' ' && pFile[i] != '\n' && pFile[i] != '\r')
					break;

				i++;
			}

			if(i >= iSize)
				break;

			// Read token in
			j = NULL;
			while(1)
			{
				if(i >= iSize)
					break;

				if(pFile[i] == ' ' || pFile[i] == '\n' || pFile[i] == '\r')
					break;

				szFlag[j] = pFile[i];
				j++; i++;
			}

			//Terminator
			szFlag[j] = 0;
			strLower(szFlag);
			
			// Only this flag for now
			if(!strcmp(szFlag, "alternate"))
				iFlags |= TEXFLAG_ALTERNATE;
			else if(!strcmp(szFlag, "fullbright"))
				iFlags |= TEXFLAG_FULLBRIGHT;
			else if(!strcmp(szFlag, "none"))
				iFlags |= TEXFLAG_NONE;
			else if(!strcmp(szFlag, "nomipmap"))
				iFlags |= TEXFLAG_NOMIPMAP;
			else if(!strcmp(szFlag, "eraseflags"))
				iFlags |= TEXFLAG_ERASE;

			// See if there's anything else ahead
			while(1)
			{
				if(i >= iSize)
					break;

				if(pFile[i] == ' ' || pFile[i] == '\n' || pFile[i] == '\r')
					break;
			}

			if(pFile[i] == '\n' || pFile[i] == '\r' || i >= iSize)
				break; // End of entry
		}

		if(iFlags)
		{
			texentry_t *pEntry = &m_pTextureEntries[m_iNumTextureEntries];
			m_iNumTextureEntries++;

			strcpy(pEntry->szModel, szModel); strLower(pEntry->szModel);
			strcpy(pEntry->szTexture, szTexture); strLower(pEntry->szTexture);
			pEntry->iFlags = iFlags;
		}
	}

	gEngfuncs.COM_FreeFile(pFile);
}

/*
====================
TextureHasFlag

====================
*/
bool CTextureLoader::TextureHasFlag( char *szModel, char *szTexture, int iFlag )
{
	if(!m_iNumTextureEntries)
		return false;

	for(int i = 0; i < m_iNumTextureEntries; i++)
	{
		if(!strcmp(m_pTextureEntries[i].szModel, szModel)
			&& !strcmp(m_pTextureEntries[i].szTexture, szTexture)
			&& m_pTextureEntries[i].iFlags & iFlag)
			return true;
	}

	return false;
}

/*
====================
WriteTGA

====================
*/
void CTextureLoader::WriteTGA( byte *pixels, int bpp, int width, int height, char *szpath )
{
	int iSize = width*height*bpp;
	byte *pBuf = new byte[(iSize+18)];
	memset(pBuf, 0, sizeof(byte)*(iSize+18));

	tga_header_t *pHeader = (tga_header_t *)pBuf;
	pHeader->datatypecode = 2;
	pHeader->bitsperpixel = bpp*8;
	pHeader->width[0] = (width & 0xFF);
	pHeader->width[1] = ((width >> 8) & 0xFF);
	pHeader->height[0] = (height & 0xFF);
	pHeader->height[1] = ((height >> 8) & 0xFF);

	for(int i = 0; i < height; i++)
	{
		GLubyte *dst = pBuf+18 + i*width*bpp;
		GLubyte *src = pixels + (height-i-1)*width*bpp;
		memcpy(dst, src, sizeof(byte)*width*bpp);

		if(bpp == 4)
		{
			for(int j = 0; j < width*bpp; j+=bpp)
			{
				dst[j] = src[j+2];
				dst[j+1] = src[j+1];
				dst[j+2] = src[j];
				dst[j+3] = src[j+3];
			}
		}
		else
		{
			for(int j = 0; j < width*bpp; j+=bpp)
			{
				dst[j] = src[j+2];
				dst[j+1] = src[j+1];
				dst[j+2] = src[j];
			}
		}
	}

	char szPath[64];
	sprintf(szPath, "%s/imagedump/%s.tga", gEngfuncs.pfnGetGameDirectory(), szpath);

	FILE *pFile = fopen(szPath, "wb");
	if(!pFile)
	{
		delete [] pBuf;
		return;
	}

	fwrite(pBuf, sizeof(byte), (iSize+18), pFile);
	fclose(pFile);

	// Free memory
	delete [] pBuf;
}