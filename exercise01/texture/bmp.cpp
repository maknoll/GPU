#include <fstream>
#include <iostream>
#include <cstdlib>
#include <GLUT/glut.h>

GLuint loadBMP(const char *fname)
{
	using namespace std;
	unsigned int width;
	unsigned int height;
	unsigned char* data;
	unsigned short planes; // number of planes in image (must be 1)
	unsigned short bpp; // number of bits per pixel (must be 24)
	ifstream fin(fname, ios::in | ios::binary);
	if(!fin)
	{
		cerr << "File not found " << fname << '\n';

		return 0;
	}
	fin.seekg(18, ios::cur);
	fin.read((char *)&width, sizeof(unsigned));
	fin.read((char *)&height, sizeof(unsigned));
	fin.read((char *)&planes, sizeof(unsigned short));
	if(planes != 1)
	{
		cout << "Planes from " << fname << " is not 1: " << planes << "\n";

		return 0;
	}	
	fin.read((char *)&bpp, sizeof(unsigned short));	
	if(bpp != 24)
	{
		cout << "Bpp from " << fname << " is not 24: " << bpp << "\n";

		return 0;
	}
	fin.seekg(24, ios::cur);
	/* size of the image in bytes (3 is to RGB component) */
	unsigned size(width * height * 3);	
	data = new unsigned char[size];	
	fin.read((char *)data, size);	
	unsigned char tmp; // temporary color storage for bgr-rgb conversion.	
	for(unsigned int i(0); i < size; i += 3)
	{	
		tmp = data[i];		
		data[i] = data[i+2];
		data[i+2] = tmp;
	}

	/* Diese Variable speichert den Texturnamen */
	GLuint handle = 0;

	/* Erzeugen eines Texturnames (handle) */
	glGenTextures(1, &handle);

	/* Binden der Textur. (Hinweis: Das target heißt GL_TEXTURE_2D) */
	glBindTexture(GL_TEXTURE_2D, handle);

	/* Füllen der Textur. Dabei sollen automatisch mip map levels erzeugt
	 * werden. (Hinweis: Nutzen sie dafür gluBuild2DMipmaps)
	 * Es handelt sich hierbei um eine Textur mit 3 Komponenten, des Formats
	 * GL_RGB und des Typs GL_UNSIGNED_BYTE. width, height und data sind
	 * bereits oben geladen worden.
	 */
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE,
			data);

	/* Setzen des TexParameter "GL_TEXTURE_MIN_FILTER" auf den Wert
	 * GL_LINEAR_MIPMAP_LINEAR.
	 */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);

	/* Setzen des TexParameter "GL_TEXTURE_MAG_FILTER" auf den Wert
	 * GL_LINEAR_MIPMAP_LINEAR.
	 */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);

	/* Die Textur muss nicht mehr gebunden sein */
	glBindTexture(GL_TEXTURE_2D, 0);
	
	/* Die Daten aus dem Hauptspeicher können weg, da sie nun im Video RAM
	 * liegen.
	 */
	if(data)
		free(data);

	/* Zurückgeben des Texturnamens */
	return handle;
}
