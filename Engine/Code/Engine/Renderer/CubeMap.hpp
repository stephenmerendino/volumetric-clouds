#pragma once
#pragma warning(disable: 4201)

class RHIDevice;
class RHITexture2D;
//struct ID3D11Texture2D;
//struct ID3D11ShaderResourceView;

enum CubeMapImageOrder : unsigned int
{
	POS_X, NEG_X,
	POS_Y, NEG_Y,
	POS_Z, NEG_Z
};

union CubeMapImagePaths
{
	struct{
		char* posX;
		char* negX;
		char* posY;
		char* negY;
		char* posZ;
		char* negZ;
	};
	char* imagePaths[6];
};

class CubeMap
{
public:
	RHITexture2D* m_texture;

public:
	CubeMap();
	CubeMap(RHIDevice* device, CubeMapImagePaths cubeMapImages);
	//CubeMap(Image* cubeMapImages);

	~CubeMap();

	bool LoadFromImages(RHIDevice* device, CubeMapImagePaths cubeMapImages);
	//bool LoadFromImages(Images* cubeMapImages);
};