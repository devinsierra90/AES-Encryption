/*

									AES Encryption Program

									Written by Devin Sierra
									Finished for 3/25/18

					This is a program that implements the AES encryption algorithm to encrypt and decrpyt files.
					Users must have a file to be encrypted/decrypted in the proper folder and must provide certain parameters.

					Parameters : 
					1. Function : -e for encryption, -d for decryption
					2. Key  : 32 hex digits or 16 characters long
					3. Mode : ecb for electronic codebook, cbc for cipher block chain mode
					4. Input file
					5. Output file


					The program will use the provided parameters to encrypt or decrypt an input file with the provided
					key in the requested mode and output to the user provided output file name.

					The only console output consists of the run time of the actual encryption/decryption

*/

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <ctime>
using namespace std;

/* Global Variables */
unsigned char state[4][4];
int numberOfRounds = 10;
int subBytesTable[16][16];
string originalFileSize;
bool isCBC = false;
unsigned char keySched[11][4][4];
const unsigned int rcon[] = { 1,2,4,8,16,32,64,128, 0x1b, 0x36 };		//round constants
string infileName, outfileName;
std::ofstream outfile;
string plaintext;	//text to write to file
string IV;			//IV for CBC encryption
char whitening[16], nextWhitening[16], initVector[16];


//SubBytes + InvSubBytes Lookup Tables
unsigned char subBytesMap[256] =
{
	
	0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
	0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
	0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
	0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
	0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
	0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
	0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
	0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
	0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
	0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
	0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
	0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
	0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
	0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
	0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
	0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};


unsigned char invSubBytesMap[256] =
{
	0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
	0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
	0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
	0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
	0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
	0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
	0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
	0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
	0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
	0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
	0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
	0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
	0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
	0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
	0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
	0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

//MixColumn and InvMixColumn Lookup Tables
unsigned char multByTwo[256]{
	0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,
	0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
	0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
	0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,
	0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,
	0xa0,0xa2,0xa4,0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,0xbe,
	0xc0,0xc2,0xc4,0xc6,0xc8,0xca,0xcc,0xce,0xd0,0xd2,0xd4,0xd6,0xd8,0xda,0xdc,0xde,
	0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xee,0xf0,0xf2,0xf4,0xf6,0xf8,0xfa,0xfc,0xfe,
	0x1b,0x19,0x1f,0x1d,0x13,0x11,0x17,0x15,0x0b,0x09,0x0f,0x0d,0x03,0x01,0x07,0x05,
	0x3b,0x39,0x3f,0x3d,0x33,0x31,0x37,0x35,0x2b,0x29,0x2f,0x2d,0x23,0x21,0x27,0x25,
	0x5b,0x59,0x5f,0x5d,0x53,0x51,0x57,0x55,0x4b,0x49,0x4f,0x4d,0x43,0x41,0x47,0x45,
	0x7b,0x79,0x7f,0x7d,0x73,0x71,0x77,0x75,0x6b,0x69,0x6f,0x6d,0x63,0x61,0x67,0x65,
	0x9b,0x99,0x9f,0x9d,0x93,0x91,0x97,0x95,0x8b,0x89,0x8f,0x8d,0x83,0x81,0x87,0x85,
	0xbb,0xb9,0xbf,0xbd,0xb3,0xb1,0xb7,0xb5,0xab,0xa9,0xaf,0xad,0xa3,0xa1,0xa7,0xa5,
	0xdb,0xd9,0xdf,0xdd,0xd3,0xd1,0xd7,0xd5,0xcb,0xc9,0xcf,0xcd,0xc3,0xc1,0xc7,0xc5,
	0xfb,0xf9,0xff,0xfd,0xf3,0xf1,0xf7,0xf5,0xeb,0xe9,0xef,0xed,0xe3,0xe1,0xe7,0xe5
};

unsigned char multByThree[256]{
	0x00,0x03,0x06,0x05,0x0c,0x0f,0x0a,0x09,0x18,0x1b,0x1e,0x1d,0x14,0x17,0x12,0x11,
	0x30,0x33,0x36,0x35,0x3c,0x3f,0x3a,0x39,0x28,0x2b,0x2e,0x2d,0x24,0x27,0x22,0x21,
	0x60,0x63,0x66,0x65,0x6c,0x6f,0x6a,0x69,0x78,0x7b,0x7e,0x7d,0x74,0x77,0x72,0x71,
	0x50,0x53,0x56,0x55,0x5c,0x5f,0x5a,0x59,0x48,0x4b,0x4e,0x4d,0x44,0x47,0x42,0x41,
	0xc0,0xc3,0xc6,0xc5,0xcc,0xcf,0xca,0xc9,0xd8,0xdb,0xde,0xdd,0xd4,0xd7,0xd2,0xd1,
	0xf0,0xf3,0xf6,0xf5,0xfc,0xff,0xfa,0xf9,0xe8,0xeb,0xee,0xed,0xe4,0xe7,0xe2,0xe1,
	0xa0,0xa3,0xa6,0xa5,0xac,0xaf,0xaa,0xa9,0xb8,0xbb,0xbe,0xbd,0xb4,0xb7,0xb2,0xb1,
	0x90,0x93,0x96,0x95,0x9c,0x9f,0x9a,0x99,0x88,0x8b,0x8e,0x8d,0x84,0x87,0x82,0x81,
	0x9b,0x98,0x9d,0x9e,0x97,0x94,0x91,0x92,0x83,0x80,0x85,0x86,0x8f,0x8c,0x89,0x8a,
	0xab,0xa8,0xad,0xae,0xa7,0xa4,0xa1,0xa2,0xb3,0xb0,0xb5,0xb6,0xbf,0xbc,0xb9,0xba,
	0xfb,0xf8,0xfd,0xfe,0xf7,0xf4,0xf1,0xf2,0xe3,0xe0,0xe5,0xe6,0xef,0xec,0xe9,0xea,
	0xcb,0xc8,0xcd,0xce,0xc7,0xc4,0xc1,0xc2,0xd3,0xd0,0xd5,0xd6,0xdf,0xdc,0xd9,0xda,
	0x5b,0x58,0x5d,0x5e,0x57,0x54,0x51,0x52,0x43,0x40,0x45,0x46,0x4f,0x4c,0x49,0x4a,
	0x6b,0x68,0x6d,0x6e,0x67,0x64,0x61,0x62,0x73,0x70,0x75,0x76,0x7f,0x7c,0x79,0x7a,
	0x3b,0x38,0x3d,0x3e,0x37,0x34,0x31,0x32,0x23,0x20,0x25,0x26,0x2f,0x2c,0x29,0x2a,
	0x0b,0x08,0x0d,0x0e,0x07,0x04,0x01,0x02,0x13,0x10,0x15,0x16,0x1f,0x1c,0x19,0x1a
};

unsigned char multByNine[256]{
	0x00,0x09,0x12,0x1b,0x24,0x2d,0x36,0x3f,0x48,0x41,0x5a,0x53,0x6c,0x65,0x7e,0x77,
	0x90,0x99,0x82,0x8b,0xb4,0xbd,0xa6,0xaf,0xd8,0xd1,0xca,0xc3,0xfc,0xf5,0xee,0xe7,
	0x3b,0x32,0x29,0x20,0x1f,0x16,0x0d,0x04,0x73,0x7a,0x61,0x68,0x57,0x5e,0x45,0x4c,
	0xab,0xa2,0xb9,0xb0,0x8f,0x86,0x9d,0x94,0xe3,0xea,0xf1,0xf8,0xc7,0xce,0xd5,0xdc,
	0x76,0x7f,0x64,0x6d,0x52,0x5b,0x40,0x49,0x3e,0x37,0x2c,0x25,0x1a,0x13,0x08,0x01,
	0xe6,0xef,0xf4,0xfd,0xc2,0xcb,0xd0,0xd9,0xae,0xa7,0xbc,0xb5,0x8a,0x83,0x98,0x91,
	0x4d,0x44,0x5f,0x56,0x69,0x60,0x7b,0x72,0x05,0x0c,0x17,0x1e,0x21,0x28,0x33,0x3a,
	0xdd,0xd4,0xcf,0xc6,0xf9,0xf0,0xeb,0xe2,0x95,0x9c,0x87,0x8e,0xb1,0xb8,0xa3,0xaa,
	0xec,0xe5,0xfe,0xf7,0xc8,0xc1,0xda,0xd3,0xa4,0xad,0xb6,0xbf,0x80,0x89,0x92,0x9b,
	0x7c,0x75,0x6e,0x67,0x58,0x51,0x4a,0x43,0x34,0x3d,0x26,0x2f,0x10,0x19,0x02,0x0b,
	0xd7,0xde,0xc5,0xcc,0xf3,0xfa,0xe1,0xe8,0x9f,0x96,0x8d,0x84,0xbb,0xb2,0xa9,0xa0,
	0x47,0x4e,0x55,0x5c,0x63,0x6a,0x71,0x78,0x0f,0x06,0x1d,0x14,0x2b,0x22,0x39,0x30,
	0x9a,0x93,0x88,0x81,0xbe,0xb7,0xac,0xa5,0xd2,0xdb,0xc0,0xc9,0xf6,0xff,0xe4,0xed,
	0x0a,0x03,0x18,0x11,0x2e,0x27,0x3c,0x35,0x42,0x4b,0x50,0x59,0x66,0x6f,0x74,0x7d,
	0xa1,0xa8,0xb3,0xba,0x85,0x8c,0x97,0x9e,0xe9,0xe0,0xfb,0xf2,0xcd,0xc4,0xdf,0xd6,
	0x31,0x38,0x23,0x2a,0x15,0x1c,0x07,0x0e,0x79,0x70,0x6b,0x62,0x5d,0x54,0x4f,0x46
};

unsigned char multByEleven[256]{
	0x00,0x0b,0x16,0x1d,0x2c,0x27,0x3a,0x31,0x58,0x53,0x4e,0x45,0x74,0x7f,0x62,0x69,
	0xb0,0xbb,0xa6,0xad,0x9c,0x97,0x8a,0x81,0xe8,0xe3,0xfe,0xf5,0xc4,0xcf,0xd2,0xd9,
	0x7b,0x70,0x6d,0x66,0x57,0x5c,0x41,0x4a,0x23,0x28,0x35,0x3e,0x0f,0x04,0x19,0x12,
	0xcb,0xc0,0xdd,0xd6,0xe7,0xec,0xf1,0xfa,0x93,0x98,0x85,0x8e,0xbf,0xb4,0xa9,0xa2,
	0xf6,0xfd,0xe0,0xeb,0xda,0xd1,0xcc,0xc7,0xae,0xa5,0xb8,0xb3,0x82,0x89,0x94,0x9f,
	0x46,0x4d,0x50,0x5b,0x6a,0x61,0x7c,0x77,0x1e,0x15,0x08,0x03,0x32,0x39,0x24,0x2f,
	0x8d,0x86,0x9b,0x90,0xa1,0xaa,0xb7,0xbc,0xd5,0xde,0xc3,0xc8,0xf9,0xf2,0xef,0xe4,
	0x3d,0x36,0x2b,0x20,0x11,0x1a,0x07,0x0c,0x65,0x6e,0x73,0x78,0x49,0x42,0x5f,0x54,
	0xf7,0xfc,0xe1,0xea,0xdb,0xd0,0xcd,0xc6,0xaf,0xa4,0xb9,0xb2,0x83,0x88,0x95,0x9e,
	0x47,0x4c,0x51,0x5a,0x6b,0x60,0x7d,0x76,0x1f,0x14,0x09,0x02,0x33,0x38,0x25,0x2e,
	0x8c,0x87,0x9a,0x91,0xa0,0xab,0xb6,0xbd,0xd4,0xdf,0xc2,0xc9,0xf8,0xf3,0xee,0xe5,
	0x3c,0x37,0x2a,0x21,0x10,0x1b,0x06,0x0d,0x64,0x6f,0x72,0x79,0x48,0x43,0x5e,0x55,
	0x01,0x0a,0x17,0x1c,0x2d,0x26,0x3b,0x30,0x59,0x52,0x4f,0x44,0x75,0x7e,0x63,0x68,
	0xb1,0xba,0xa7,0xac,0x9d,0x96,0x8b,0x80,0xe9,0xe2,0xff,0xf4,0xc5,0xce,0xd3,0xd8,
	0x7a,0x71,0x6c,0x67,0x56,0x5d,0x40,0x4b,0x22,0x29,0x34,0x3f,0x0e,0x05,0x18,0x13,
	0xca,0xc1,0xdc,0xd7,0xe6,0xed,0xf0,0xfb,0x92,0x99,0x84,0x8f,0xbe,0xb5,0xa8,0xa3
};

unsigned char multByThirteen[256]{
	0x00,0x0d,0x1a,0x17,0x34,0x39,0x2e,0x23,0x68,0x65,0x72,0x7f,0x5c,0x51,0x46,0x4b,
	0xd0,0xdd,0xca,0xc7,0xe4,0xe9,0xfe,0xf3,0xb8,0xb5,0xa2,0xaf,0x8c,0x81,0x96,0x9b,
	0xbb,0xb6,0xa1,0xac,0x8f,0x82,0x95,0x98,0xd3,0xde,0xc9,0xc4,0xe7,0xea,0xfd,0xf0,
	0x6b,0x66,0x71,0x7c,0x5f,0x52,0x45,0x48,0x03,0x0e,0x19,0x14,0x37,0x3a,0x2d,0x20,
	0x6d,0x60,0x77,0x7a,0x59,0x54,0x43,0x4e,0x05,0x08,0x1f,0x12,0x31,0x3c,0x2b,0x26,
	0xbd,0xb0,0xa7,0xaa,0x89,0x84,0x93,0x9e,0xd5,0xd8,0xcf,0xc2,0xe1,0xec,0xfb,0xf6,
	0xd6,0xdb,0xcc,0xc1,0xe2,0xef,0xf8,0xf5,0xbe,0xb3,0xa4,0xa9,0x8a,0x87,0x90,0x9d,
	0x06,0x0b,0x1c,0x11,0x32,0x3f,0x28,0x25,0x6e,0x63,0x74,0x79,0x5a,0x57,0x40,0x4d,
	0xda,0xd7,0xc0,0xcd,0xee,0xe3,0xf4,0xf9,0xb2,0xbf,0xa8,0xa5,0x86,0x8b,0x9c,0x91,
	0x0a,0x07,0x10,0x1d,0x3e,0x33,0x24,0x29,0x62,0x6f,0x78,0x75,0x56,0x5b,0x4c,0x41,
	0x61,0x6c,0x7b,0x76,0x55,0x58,0x4f,0x42,0x09,0x04,0x13,0x1e,0x3d,0x30,0x27,0x2a,
	0xb1,0xbc,0xab,0xa6,0x85,0x88,0x9f,0x92,0xd9,0xd4,0xc3,0xce,0xed,0xe0,0xf7,0xfa,
	0xb7,0xba,0xad,0xa0,0x83,0x8e,0x99,0x94,0xdf,0xd2,0xc5,0xc8,0xeb,0xe6,0xf1,0xfc,
	0x67,0x6a,0x7d,0x70,0x53,0x5e,0x49,0x44,0x0f,0x02,0x15,0x18,0x3b,0x36,0x21,0x2c,
	0x0c,0x01,0x16,0x1b,0x38,0x35,0x22,0x2f,0x64,0x69,0x7e,0x73,0x50,0x5d,0x4a,0x47,
	0xdc,0xd1,0xc6,0xcb,0xe8,0xe5,0xf2,0xff,0xb4,0xb9,0xae,0xa3,0x80,0x8d,0x9a,0x97
};

unsigned char multByFourteen[256]{
	0x00,0x0e,0x1c,0x12,0x38,0x36,0x24,0x2a,0x70,0x7e,0x6c,0x62,0x48,0x46,0x54,0x5a,
	0xe0,0xee,0xfc,0xf2,0xd8,0xd6,0xc4,0xca,0x90,0x9e,0x8c,0x82,0xa8,0xa6,0xb4,0xba,
	0xdb,0xd5,0xc7,0xc9,0xe3,0xed,0xff,0xf1,0xab,0xa5,0xb7,0xb9,0x93,0x9d,0x8f,0x81,
	0x3b,0x35,0x27,0x29,0x03,0x0d,0x1f,0x11,0x4b,0x45,0x57,0x59,0x73,0x7d,0x6f,0x61,
	0xad,0xa3,0xb1,0xbf,0x95,0x9b,0x89,0x87,0xdd,0xd3,0xc1,0xcf,0xe5,0xeb,0xf9,0xf7,
	0x4d,0x43,0x51,0x5f,0x75,0x7b,0x69,0x67,0x3d,0x33,0x21,0x2f,0x05,0x0b,0x19,0x17,
	0x76,0x78,0x6a,0x64,0x4e,0x40,0x52,0x5c,0x06,0x08,0x1a,0x14,0x3e,0x30,0x22,0x2c,
	0x96,0x98,0x8a,0x84,0xae,0xa0,0xb2,0xbc,0xe6,0xe8,0xfa,0xf4,0xde,0xd0,0xc2,0xcc,
	0x41,0x4f,0x5d,0x53,0x79,0x77,0x65,0x6b,0x31,0x3f,0x2d,0x23,0x09,0x07,0x15,0x1b,
	0xa1,0xaf,0xbd,0xb3,0x99,0x97,0x85,0x8b,0xd1,0xdf,0xcd,0xc3,0xe9,0xe7,0xf5,0xfb,
	0x9a,0x94,0x86,0x88,0xa2,0xac,0xbe,0xb0,0xea,0xe4,0xf6,0xf8,0xd2,0xdc,0xce,0xc0,
	0x7a,0x74,0x66,0x68,0x42,0x4c,0x5e,0x50,0x0a,0x04,0x16,0x18,0x32,0x3c,0x2e,0x20,
	0xec,0xe2,0xf0,0xfe,0xd4,0xda,0xc8,0xc6,0x9c,0x92,0x80,0x8e,0xa4,0xaa,0xb8,0xb6,
	0x0c,0x02,0x10,0x1e,0x34,0x3a,0x28,0x26,0x7c,0x72,0x60,0x6e,0x44,0x4a,0x58,0x56,
	0x37,0x39,0x2b,0x25,0x0f,0x01,0x13,0x1d,0x47,0x49,0x5b,0x55,0x7f,0x71,0x63,0x6d,
	0xd7,0xd9,0xcb,0xc5,0xef,0xe1,0xf3,0xfd,0xa7,0xa9,0xbb,0xb5,0x9f,0x91,0x83,0x8d
};




void convertKeyFromHex(char* key, char keysToFill[16]) {
	/*
		this method takes a string of hex digits and turns combines two of them 
		into their corresponding ascii characters
	*/
	string keyStr = key;
	unsigned int keyValue;
	int index = 0;

	std::stringstream ss;
	for (int i = 0; i < 32; i += 2) {
		//take 2 hex digits at a time and convert them into their correct values, store them
		ss << hex << keyStr.substr(i, 2);
		ss >> keyValue;
		keysToFill[i / 2] = keyValue;
		ss.clear();
	}
}

string convertFileSizeToBytes(unsigned int fileSize) {
	/*
		This method takes an integer (the file size) and converts it into a 4-byte string of characters
	*/
	string convertedSize = "";
	unsigned char first = (fileSize & 0xFF000000) >> 24;;
	unsigned char second = (fileSize & 0x00FF0000) >> 16;
	unsigned char third = (fileSize & 0x0000FF00) >> 8;
	unsigned char fourth = fileSize & 0x000000FF;

	convertedSize.push_back(first);
	convertedSize.push_back(second);
	convertedSize.push_back(third);
	convertedSize.push_back(fourth);
	
	return convertedSize;

}

unsigned int convertBytesToFileSize(string bytes) {
	/*
		This method takes a string of btytes and converts those bytes into an integer
	*/
	unsigned char first = bytes[0];
	unsigned char second = bytes[1];
	unsigned char third = bytes[2];
	unsigned char fourth = bytes[3];

	unsigned int filesize = first;
	filesize <<= 8;
	filesize += second;
	filesize <<= 8;
	filesize += third;
	filesize <<= 8;
	filesize += fourth;

	return filesize;
}

string generateRandomGarbage(int bytesNeeded) {

	/*
	this method is to be used when we need extra characters
	1)When we encrypt the original file size we need 96 bits of extra characters for the left half
	2)When we encrypt a file that is not an even multiple of 8 bytes we need to pad the rest
	3) Generate an IV for cbc mode
	*/


	string garbageTxt = "";
	char nextChar;
	srand(time(0));
	unsigned int nextRandom;
	for (int i = 0; i < bytesNeeded; i++) {
		nextRandom = rand() % 256;
		nextRandom &= 0x000000FF;
		garbageTxt.push_back((char)nextRandom);
	}

	return garbageTxt;


}

void performEncryptionAlgorithm(char* buffer) {

	/*
			This the the actual AES algorithm

			All the steps have been for encryption have been included inside this method 
				-subBytes
				-shiftRows
				-mixColumns
				-addRoundKey
	*/


	//declare variables to be used in each round
	int mixedColumnByte1, mixedColumnByte2, mixedColumnByte3, mixedColumnByte4, temp;

	//load the state
	state[0][0] = buffer[0];
	state[1][0] = buffer[1];
	state[2][0] = buffer[2];
	state[3][0] = buffer[3];
	state[0][1] = buffer[4];
	state[1][1] = buffer[5];
	state[2][1] = buffer[6];
	state[3][1] = buffer[7];
	state[0][2] = buffer[8];
	state[1][2] = buffer[9];
	state[2][2] = buffer[10];
	state[3][2] = buffer[11];
	state[0][3] = buffer[12];
	state[1][3] = buffer[13];
	state[2][3] = buffer[14];
	state[3][3] = buffer[15];
	//state[3][3] = 1;

	//add round key
	state[0][0] ^= keySched[0][0][0];
	state[1][0] ^= keySched[0][1][0];
	state[2][0] ^= keySched[0][2][0];
	state[3][0] ^= keySched[0][3][0];
	state[0][1] ^= keySched[0][0][1];
	state[1][1] ^= keySched[0][1][1];
	state[2][1] ^= keySched[0][2][1];
	state[3][1] ^= keySched[0][3][1];
	state[0][2] ^= keySched[0][0][2];
	state[1][2] ^= keySched[0][1][2];
	state[2][2] ^= keySched[0][2][2];
	state[3][2] ^= keySched[0][3][2];
	state[0][3] ^= keySched[0][0][3];
	state[1][3] ^= keySched[0][1][3];
	state[2][3] ^= keySched[0][2][3];
	state[3][3] ^= keySched[0][3][3];

	//10 rounds
	for (int i = 1; i < 11; i++) {

		/*
		Sub Bytes
		*/
		state[0][0] = subBytesMap[state[0][0]];
		state[1][0] = subBytesMap[state[1][0]];
		state[2][0] = subBytesMap[state[2][0]];
		state[3][0] = subBytesMap[state[3][0]];
		state[0][1] = subBytesMap[state[0][1]];
		state[1][1] = subBytesMap[state[1][1]];
		state[2][1] = subBytesMap[state[2][1]];
		state[3][1] = subBytesMap[state[3][1]];
		state[0][2] = subBytesMap[state[0][2]];
		state[1][2] = subBytesMap[state[1][2]];
		state[2][2] = subBytesMap[state[2][2]];
		state[3][2] = subBytesMap[state[3][2]];
		state[0][3] = subBytesMap[state[0][3]];
		state[1][3] = subBytesMap[state[1][3]];
		state[2][3] = subBytesMap[state[2][3]];
		state[3][3] = subBytesMap[state[3][3]];

		/*
		Shift Rows
		*/

		temp = state[1][0];
		state[1][0] = state[1][1];
		state[1][1] = state[1][2];
		state[1][2] = state[1][3];
		state[1][3] = temp;

		temp = state[2][0];
		state[2][0] = state[2][2];
		state[2][2] = temp;
		temp = state[2][1];
		state[2][1] = state[2][3];
		state[2][3] = temp;

		temp = state[3][0];
		state[3][0] = state[3][3];
		state[3][3] = state[3][2];
		state[3][2] = state[3][1];
		state[3][1] = temp;

		/*
		Mix Columns
		*/

		if (i != 10) {
			//first column
			mixedColumnByte1 = multByTwo[state[0][0]] ^ multByThree[state[1][0]] ^ state[2][0] ^ state[3][0];
			mixedColumnByte2 = state[0][0] ^ multByTwo[state[1][0]] ^ multByThree[state[2][0]] ^ state[3][0];
			mixedColumnByte3 = state[0][0] ^ state[1][0] ^ multByTwo[state[2][0]] ^ multByThree[state[3][0]];
			mixedColumnByte4 = multByThree[state[0][0]] ^ state[1][0] ^ state[2][0] ^ multByTwo[state[3][0]];

			state[0][0] = mixedColumnByte1;
			state[1][0] = mixedColumnByte2;
			state[2][0] = mixedColumnByte3;
			state[3][0] = mixedColumnByte4;

			//second column
			mixedColumnByte1 = multByTwo[state[0][1]] ^ multByThree[state[1][1]] ^ state[2][1] ^ state[3][1];
			mixedColumnByte2 = state[0][1] ^ multByTwo[state[1][1]] ^ multByThree[state[2][1]] ^ state[3][1];
			mixedColumnByte3 = state[0][1] ^ state[1][1] ^ multByTwo[state[2][1]] ^ multByThree[state[3][1]];
			mixedColumnByte4 = multByThree[state[0][1]] ^ state[1][1] ^ state[2][1] ^ multByTwo[state[3][1]];

			state[0][1] = mixedColumnByte1;
			state[1][1] = mixedColumnByte2;
			state[2][1] = mixedColumnByte3;
			state[3][1] = mixedColumnByte4;

			//third column
			mixedColumnByte1 = multByTwo[state[0][2]] ^ multByThree[state[1][2]] ^ state[2][2] ^ state[3][2];
			mixedColumnByte2 = state[0][2] ^ multByTwo[state[1][2]] ^ multByThree[state[2][2]] ^ state[3][2];
			mixedColumnByte3 = state[0][2] ^ state[1][2] ^ multByTwo[state[2][2]] ^ multByThree[state[3][2]];
			mixedColumnByte4 = multByThree[state[0][2]] ^ state[1][2] ^ state[2][2] ^ multByTwo[state[3][2]];

			state[0][2] = mixedColumnByte1;
			state[1][2] = mixedColumnByte2;
			state[2][2] = mixedColumnByte3;
			state[3][2] = mixedColumnByte4;

			//fourth column
			mixedColumnByte1 = multByTwo[state[0][3]] ^ multByThree[state[1][3]] ^ state[2][3] ^ state[3][3];
			mixedColumnByte2 = state[0][3] ^ multByTwo[state[1][3]] ^ multByThree[state[2][3]] ^ state[3][3];
			mixedColumnByte3 = state[0][3] ^ state[1][3] ^ multByTwo[state[2][3]] ^ multByThree[state[3][3]];
			mixedColumnByte4 = multByThree[state[0][3]] ^ state[1][3] ^ state[2][3] ^ multByTwo[state[3][3]];

			state[0][3] = mixedColumnByte1;
			state[1][3] = mixedColumnByte2;
			state[2][3] = mixedColumnByte3;
			state[3][3] = mixedColumnByte4;
		}

		/*
		Add round key
		*/
		state[0][0] ^= keySched[i][0][0];
		state[1][0] ^= keySched[i][1][0];
		state[2][0] ^= keySched[i][2][0];
		state[3][0] ^= keySched[i][3][0];
		state[0][1] ^= keySched[i][0][1];
		state[1][1] ^= keySched[i][1][1];
		state[2][1] ^= keySched[i][2][1];
		state[3][1] ^= keySched[i][3][1];
		state[0][2] ^= keySched[i][0][2];
		state[1][2] ^= keySched[i][1][2];
		state[2][2] ^= keySched[i][2][2];
		state[3][2] ^= keySched[i][3][2];
		state[0][3] ^= keySched[i][0][3];
		state[1][3] ^= keySched[i][1][3];
		state[2][3] ^= keySched[i][2][3];
		state[3][3] ^= keySched[i][3][3];


	}

	string cipherstring = "";
	cipherstring.push_back(state[0][0]);
	cipherstring.push_back(state[1][0]);
	cipherstring.push_back(state[2][0]);
	cipherstring.push_back(state[3][0]);
	cipherstring.push_back(state[0][1]);
	cipherstring.push_back(state[1][1]);
	cipherstring.push_back(state[2][1]);
	cipherstring.push_back(state[3][1]);
	cipherstring.push_back(state[0][2]);
	cipherstring.push_back(state[1][2]);
	cipherstring.push_back(state[2][2]);
	cipherstring.push_back(state[3][2]);
	cipherstring.push_back(state[0][3]);
	cipherstring.push_back(state[1][3]);
	cipherstring.push_back(state[2][3]);
	cipherstring.push_back(state[3][3]);

	if (isCBC) {

		whitening[0] = state[0][0];
		whitening[1] = state[1][0];
		whitening[2] = state[2][0];
		whitening[3] = state[3][0];
		whitening[4] = state[0][1];
		whitening[5] = state[1][1];
		whitening[6] = state[2][1];
		whitening[7] = state[3][1];
		whitening[8] = state[0][2];
		whitening[9] = state[1][2];
		whitening[10] = state[2][2];
		whitening[11] = state[3][2];
		whitening[12] = state[0][3];
		whitening[13] = state[1][3];
		whitening[14] = state[2][3];
		whitening[15] = state[3][3];
	}

	outfile << cipherstring;
}

void encrypt() {
	
	/*
		This method is called when the program is instructed to do an encryption.

		It sets up all the pieces needed to successfully encrypt plaintext and then calls the method
		that contains the AES algorithm.

		This method is responsible for getting the file size that will be encrypted 
		This method is also responsible for determing if any padding is needed
			Padding is needed if the file is not an even multiple of 16 bytes
	*/
	char* buffer = new char[16];

	//get file length and move pointer back to beginning
	unsigned int lengthOfInputFile;
	std::ifstream ifs(infileName, std::ios_base::binary);
	ifs.seekg(0, ios_base::end);
	lengthOfInputFile = ifs.tellg();
	ifs.seekg(0, ios_base::beg);

	//setup output file
	//clear output file incase anything is in there , and open for appending
	outfile.open(outfileName);
	outfile << "";
	outfile.close();
	outfile.open(outfileName, std::ios_base::app | ios::binary);

	/*
		Handle the filesize in order to encrpyt.  The first 12 bytes are random 
		and the last 4 bytes are the file size.
	*/
	string randomBytes = generateRandomGarbage(12);
	string fileSizeToEncrypt = randomBytes + convertFileSizeToBytes(lengthOfInputFile);

	/*
		If the file length is NOT an even multiple of 16 bytes then we need to pad
		Also set the index for the last block. 
	*/
	unsigned int lastBlock = (lengthOfInputFile / 16) * 16;
	if (lengthOfInputFile % 16 != 0) {
		//padding needed
		int paddingNeededInFile = 16 - (lengthOfInputFile % 16);
		string extraGarbage;
		if (paddingNeededInFile != 8) {
			extraGarbage = generateRandomGarbage(paddingNeededInFile);
			lengthOfInputFile += paddingNeededInFile;
		}
	}

	//start timer
	auto start = std::chrono::high_resolution_clock::now();

	//group all the encryptions together to get runtime
	if (isCBC) {
		//encrypt the IV
		IV = generateRandomGarbage(16);
		for (int i = 0; i < 16; i++) {
			initVector[i] = IV[i];
		}
		performEncryptionAlgorithm(initVector); 
	}
	
	for (int i = 0; i < 16; i++) {
		if (isCBC) {
			//if it is CBC mode then the characters need to be XOR'd before encryption as whitening
			buffer[i] = fileSizeToEncrypt[i] ^ IV[i];
		}
		else {
			//load the file size into a 16-byte buffer to be sent through AES encryption
			buffer[i] = fileSizeToEncrypt[i];
		}
	}
	performEncryptionAlgorithm(buffer);
	for (unsigned int i =0; i < lengthOfInputFile; i += 16) {
		
		ifs.read(buffer, 16);
		if (isCBC) {
			//if CBC mode then XOR the plaintext with the previous block as whitening
			buffer[0] ^= whitening[0];
			buffer[1] ^= whitening[1];
			buffer[2] ^= whitening[2];
			buffer[3] ^= whitening[3];
			buffer[4] ^= whitening[4];
			buffer[5] ^= whitening[5];
			buffer[6] ^= whitening[6];
			buffer[7] ^= whitening[7];
			buffer[8] ^= whitening[8];
			buffer[9] ^= whitening[9];
			buffer[10] ^= whitening[10];
			buffer[11] ^= whitening[11];
			buffer[12] ^= whitening[12];
			buffer[13] ^= whitening[13];
			buffer[14] ^= whitening[14];
			buffer[15] ^= whitening[15];

		}
		performEncryptionAlgorithm(buffer);
	}
	auto done = std::chrono::high_resolution_clock::now();
	cout << std::chrono::duration_cast<std::chrono::milliseconds>(done - start).count() / 1000 << "." << std::chrono::duration_cast<std::chrono::milliseconds>(done - start).count() % 1000 << " seconds";
	cout << endl;
	outfile.close();
}

void performDecryptionAlgorithm(char* buffer, int mode) {

	/*
		This method performs the AES decryption algorithm.  
		The steps included in the algorithm are
			-InvSubBytes
			-InvShiftRows
			-InvMixColumn
			-AddRoundKey
		These steps are all performed in reverse order as the AES encryption algorithm.

		This method takes in an array of bytes to be decrypted and also an integer for the mode
		The mode will always be a value of 1,2, or 3 ... explained below
			
		Mode = 
				1  : Mode 1 means we are decrypting the original file size from the file.  This will be held in a variable
				2  : Mode 2 means we are decrpyting the IV that was used in encryption in CBC mode.
				3  : Mode 3 means we are decrypting ciphertext and there is nothing special to do other than write the plaintext to file

		
		--CBC Mode--
		CBC mode means the ciphertext needs to be stored in order to use it for XORing the next block
	*/

	plaintext = "";
	unsigned char* unsignedBuffer = (unsigned char*)buffer;
	//declare variables to be used in each round
	int mixedColumnByte1, mixedColumnByte2, mixedColumnByte3, mixedColumnByte4, temp;

	//load the state
	state[0][0] = buffer[0];
	state[1][0] = buffer[1];
	state[2][0] = buffer[2];
	state[3][0] = buffer[3];
	state[0][1] = buffer[4];
	state[1][1] = buffer[5];
	state[2][1] = buffer[6];
	state[3][1] = buffer[7];
	state[0][2] = buffer[8];
	state[1][2] = buffer[9];
	state[2][2] = buffer[10];
	state[3][2] = buffer[11];
	state[0][3] = buffer[12];
	state[1][3] = buffer[13];
	state[2][3] = buffer[14];
	state[3][3] = buffer[15];

	//add round key
	state[0][0] ^= keySched[10][0][0];
	state[1][0] ^= keySched[10][1][0];
	state[2][0] ^= keySched[10][2][0];
	state[3][0] ^= keySched[10][3][0];
	state[0][1] ^= keySched[10][0][1];
	state[1][1] ^= keySched[10][1][1];
	state[2][1] ^= keySched[10][2][1];
	state[3][1] ^= keySched[10][3][1];
	state[0][2] ^= keySched[10][0][2];
	state[1][2] ^= keySched[10][1][2];
	state[2][2] ^= keySched[10][2][2];
	state[3][2] ^= keySched[10][3][2];
	state[0][3] ^= keySched[10][0][3];
	state[1][3] ^= keySched[10][1][3];
	state[2][3] ^= keySched[10][2][3];
	state[3][3] ^= keySched[10][3][3];

	for (int i = 9; i > -1; i--) {
		/*
			Inverse shift rows
		*/
		temp = state[1][3];
		state[1][3] = state[1][2];
		state[1][2] = state[1][1];
		state[1][1] = state[1][0];
		state[1][0] = temp;

		temp = state[2][0];
		state[2][0] = state[2][2];
		state[2][2] = temp;
		temp = state[2][1];
		state[2][1] = state[2][3];
		state[2][3] = temp;

		temp = state[3][0];
		state[3][0] = state[3][1];
		state[3][1] = state[3][2];
		state[3][2] = state[3][3];
		state[3][3] = temp;

		/*
			Inv SubBytes
		*/
		state[0][0] = invSubBytesMap[state[0][0]];
		state[1][0] = invSubBytesMap[state[1][0]];
		state[2][0] = invSubBytesMap[state[2][0]];
		state[3][0] = invSubBytesMap[state[3][0]];
		state[0][1] = invSubBytesMap[state[0][1]];
		state[1][1] = invSubBytesMap[state[1][1]];
		state[2][1] = invSubBytesMap[state[2][1]];
		state[3][1] = invSubBytesMap[state[3][1]];
		state[0][2] = invSubBytesMap[state[0][2]];
		state[1][2] = invSubBytesMap[state[1][2]];
		state[2][2] = invSubBytesMap[state[2][2]];
		state[3][2] = invSubBytesMap[state[3][2]];
		state[0][3] = invSubBytesMap[state[0][3]];
		state[1][3] = invSubBytesMap[state[1][3]];
		state[2][3] = invSubBytesMap[state[2][3]];
		state[3][3] = invSubBytesMap[state[3][3]];

		/*
			Add Round Key
		*/
		state[0][0] ^= keySched[i][0][0];
		state[1][0] ^= keySched[i][1][0];
		state[2][0] ^= keySched[i][2][0];
		state[3][0] ^= keySched[i][3][0];
		state[0][1] ^= keySched[i][0][1];
		state[1][1] ^= keySched[i][1][1];
		state[2][1] ^= keySched[i][2][1];
		state[3][1] ^= keySched[i][3][1];
		state[0][2] ^= keySched[i][0][2];
		state[1][2] ^= keySched[i][1][2];
		state[2][2] ^= keySched[i][2][2];
		state[3][2] ^= keySched[i][3][2];
		state[0][3] ^= keySched[i][0][3];
		state[1][3] ^= keySched[i][1][3];
		state[2][3] ^= keySched[i][2][3];
		state[3][3] ^= keySched[i][3][3];

		/*
			Inv MixColumn
		*/
		if (i != 0) {
			//first column
			mixedColumnByte1 = multByFourteen[state[0][0]] ^ multByEleven[state[1][0]] ^ multByThirteen[state[2][0]] ^ multByNine[state[3][0]];
			mixedColumnByte2 = multByNine[state[0][0]] ^ multByFourteen[state[1][0]] ^ multByEleven[state[2][0]] ^ multByThirteen[state[3][0]];
			mixedColumnByte3 = multByThirteen[state[0][0]] ^ multByNine[state[1][0]] ^ multByFourteen[state[2][0]] ^ multByEleven[state[3][0]];
			mixedColumnByte4 = multByEleven[state[0][0]] ^ multByThirteen[state[1][0]] ^ multByNine[state[2][0]] ^ multByFourteen[state[3][0]];

			state[0][0] = mixedColumnByte1;
			state[1][0] = mixedColumnByte2;
			state[2][0] = mixedColumnByte3;
			state[3][0] = mixedColumnByte4;

			//second column
			mixedColumnByte1 = multByFourteen[state[0][1]] ^ multByEleven[state[1][1]] ^ multByThirteen[state[2][1]] ^ multByNine[state[3][1]];
			mixedColumnByte2 = multByNine[state[0][1]] ^ multByFourteen[state[1][1]] ^ multByEleven[state[2][1]] ^ multByThirteen[state[3][1]];
			mixedColumnByte3 = multByThirteen[state[0][1]] ^ multByNine[state[1][1]] ^ multByFourteen[state[2][1]] ^ multByEleven[state[3][1]];
			mixedColumnByte4 = multByEleven[state[0][1]] ^ multByThirteen[state[1][1]] ^ multByNine[state[2][1]] ^ multByFourteen[state[3][1]];

			state[0][1] = mixedColumnByte1;
			state[1][1] = mixedColumnByte2;
			state[2][1] = mixedColumnByte3;
			state[3][1] = mixedColumnByte4;

			//third column
			mixedColumnByte1 = multByFourteen[state[0][2]] ^ multByEleven[state[1][2]] ^ multByThirteen[state[2][2]] ^ multByNine[state[3][2]];
			mixedColumnByte2 = multByNine[state[0][2]] ^ multByFourteen[state[1][2]] ^ multByEleven[state[2][2]] ^ multByThirteen[state[3][2]];
			mixedColumnByte3 = multByThirteen[state[0][2]] ^ multByNine[state[1][2]] ^ multByFourteen[state[2][2]] ^ multByEleven[state[3][2]];
			mixedColumnByte4 = multByEleven[state[0][2]] ^ multByThirteen[state[1][2]] ^ multByNine[state[2][2]] ^ multByFourteen[state[3][2]];

			state[0][2] = mixedColumnByte1;
			state[1][2] = mixedColumnByte2;
			state[2][2] = mixedColumnByte3;
			state[3][2] = mixedColumnByte4;

			//fourth column
			mixedColumnByte1 = multByFourteen[state[0][3]] ^ multByEleven[state[1][3]] ^ multByThirteen[state[2][3]] ^ multByNine[state[3][3]];
			mixedColumnByte2 = multByNine[state[0][3]] ^ multByFourteen[state[1][3]] ^ multByEleven[state[2][3]] ^ multByThirteen[state[3][3]];
			mixedColumnByte3 = multByThirteen[state[0][3]] ^ multByNine[state[1][3]] ^ multByFourteen[state[2][3]] ^ multByEleven[state[3][3]];
			mixedColumnByte4 = multByEleven[state[0][3]] ^ multByThirteen[state[1][3]] ^ multByNine[state[2][3]] ^ multByFourteen[state[3][3]];

			state[0][3] = mixedColumnByte1;
			state[1][3] = mixedColumnByte2;
			state[2][3] = mixedColumnByte3;
			state[3][3] = mixedColumnByte4;
		}

	}
	
	if (isCBC && mode!=2) {
		/*
			In CBC mode the last step is to XOR with the previous ciphertext block(the current whitening block).
		*/
		state[0][0] ^= whitening[0];
		state[1][0] ^= whitening[1];
		state[2][0] ^= whitening[2];
		state[3][0] ^= whitening[3];
		state[0][1] ^= whitening[4];
		state[1][1] ^= whitening[5];
		state[2][1] ^= whitening[6];
		state[3][1] ^= whitening[7];
		state[0][2] ^= whitening[8];
		state[1][2] ^= whitening[9];
		state[2][2] ^= whitening[10];
		state[3][2] ^= whitening[11];
		state[0][3] ^= whitening[12];
		state[1][3] ^= whitening[13];
		state[2][3] ^= whitening[14];
		state[3][3] ^= whitening[15];

		//set the next block up for the next decrpytion
			//the whitening for the next block of CBC mode is moved up 
		whitening[0] = nextWhitening[0];
		whitening[1] = nextWhitening[1];
		whitening[2] = nextWhitening[2];
		whitening[3] = nextWhitening[3];
		whitening[4] = nextWhitening[4];
		whitening[5] = nextWhitening[5];
		whitening[6] = nextWhitening[6];
		whitening[7] = nextWhitening[7];
		whitening[8] = nextWhitening[8];
		whitening[9] = nextWhitening[9];
		whitening[10] = nextWhitening[10];
		whitening[11] = nextWhitening[11];
		whitening[12] = nextWhitening[12];
		whitening[13] = nextWhitening[13];
		whitening[14] = nextWhitening[14];
		whitening[15] = nextWhitening[15];

		
	}

	if (mode == 3) {
		plaintext.push_back(state[0][0]);
		plaintext.push_back(state[1][0]);
		plaintext.push_back(state[2][0]);
		plaintext.push_back(state[3][0]);
		plaintext.push_back(state[0][1]);
		plaintext.push_back(state[1][1]);
		plaintext.push_back(state[2][1]);
		plaintext.push_back(state[3][1]);
		plaintext.push_back(state[0][2]);
		plaintext.push_back(state[1][2]);
		plaintext.push_back(state[2][2]);
		plaintext.push_back(state[3][2]);
		plaintext.push_back(state[0][3]);
		plaintext.push_back(state[1][3]);
		plaintext.push_back(state[2][3]);
		plaintext.push_back(state[3][3]);
	}
	else if (mode == 2) {
		//IV mode
		
		initVector[0] = state[0][0];
		initVector[1] = state[1][0];
		initVector[2] = state[2][0];
		initVector[3] = state[3][0];
		initVector[4] = state[0][1];
		initVector[5] = state[1][1];
		initVector[6] = state[2][1];
		initVector[7] = state[3][1];
		initVector[8] = state[0][2];
		initVector[9] = state[1][2];
		initVector[10] = state[2][2];
		initVector[11] = state[3][2];
		initVector[12] = state[0][3];
		initVector[13] = state[1][3];
		initVector[14] = state[2][3];
		initVector[15] = state[3][3];

	}
	else {
		originalFileSize.push_back(state[0][3]);
		originalFileSize.push_back(state[1][3]);
		originalFileSize.push_back(state[2][3]);
		originalFileSize.push_back(state[3][3]);
	}
}

void decrypt() {

	/*
		This method is called when the user requests to decrpyt.

		This method determines 
			-the input file length
			-sets up output file
			-calls AES decryption algorithm method to 
				-decrypt the IV (if in cbc mode)
				-decrypt the original file length
				-decrypt all other plaintext
	*/

	char* buffer = new char[17];
	buffer[16] = '\0';

	//get file length and move pointer back to beginning
	unsigned long lengthOfInputFile;
	std::ifstream ifs(infileName, std::ios_base::binary);
	ifs.seekg(0, ios_base::end);
	lengthOfInputFile = ifs.tellg();
	ifs.seekg(0, ios_base::beg);

	//setup output file
	//clear output file incase anything is in there and open for appending
	outfile.open(outfileName);
	outfile << "";
	outfile.close();
	outfile.open(outfileName, std::ios_base::app | ios::binary);


	int offset = 16;		//offset is to keep track of where in the file to start decrypting.

							//start timer
	auto start = std::chrono::high_resolution_clock::now();

	if (isCBC) {			// 16 bytes account for file size, 16 account for IV (possibly)
		/*
			decrypt the IV
		*/
		ifs.read(buffer, 16);
		performDecryptionAlgorithm(buffer, 2);	//set the IV during decryption
		offset += 16;							//keeps track of how much to parse in the input file, IV = 16 bytes
													// in CBC mode we need to save the IV to file adding an extra 16 bytes to the file
													//so we account for this in the offset variable
		
		//the IV becomes the first block of whitening
		whitening[0] = initVector[0];
		whitening[1] = initVector[1];
		whitening[2] = initVector[2];
		whitening[3] = initVector[3];
		whitening[4] = initVector[4];
		whitening[5] = initVector[5];
		whitening[6] = initVector[6];
		whitening[7] = initVector[7];
		whitening[8] = initVector[8];
		whitening[9] = initVector[9];
		whitening[10] = initVector[10];
		whitening[11] = initVector[11];
		whitening[12] = initVector[12];
		whitening[13] = initVector[13];
		whitening[14] = initVector[14];
		whitening[15] = initVector[15];

	}

	
			
		/*decrypt for file length */
			
		ifs.read(buffer, 16);				//read ciphertext, store it if CBC mode to be used in the next block
		if (isCBC) {
			
			//the whitening we will use for next round needs to be stored until ready to use
				//the whitening for each next block is simply the current blocks ciphertext
			nextWhitening[0] = buffer[0];
			nextWhitening[1] = buffer[1];
			nextWhitening[2] = buffer[2];
			nextWhitening[3] = buffer[3];
			nextWhitening[4] = buffer[4];
			nextWhitening[5] = buffer[5];
			nextWhitening[6] = buffer[6];
			nextWhitening[7] = buffer[7];
			nextWhitening[8] = buffer[8];
			nextWhitening[9] = buffer[9];
			nextWhitening[10] = buffer[10];
			nextWhitening[11] = buffer[11];
			nextWhitening[12] = buffer[12];
			nextWhitening[13] = buffer[13];
			nextWhitening[14] = buffer[14];
			nextWhitening[15] = buffer[15];
		}
		performDecryptionAlgorithm(buffer, 1);
		unsigned int origFilelength = convertBytesToFileSize(originalFileSize);

		//calculate the number of blocks in the original file.  This will help when the file was not
		// an even multiple of 16 bytes so we only keep the bytes we want
		unsigned int numberOfBlocks = origFilelength / 16;
		int remainingChars = origFilelength % 16;
		unsigned int currentBlock = 1;
	

	for (int i = 0; i < lengthOfInputFile - offset; i += 16) {
		ifs.read(buffer, 16);			//read ciphertext
		if (isCBC) {
			// store the ciphertext as the next block of whitening, for the next block of decryption

			nextWhitening[0] = buffer[0];
			nextWhitening[1] = buffer[1];
			nextWhitening[2] = buffer[2];
			nextWhitening[3] = buffer[3];
			nextWhitening[4] = buffer[4];
			nextWhitening[5] = buffer[5];
			nextWhitening[6] = buffer[6];
			nextWhitening[7] = buffer[7];
			nextWhitening[8] = buffer[8];
			nextWhitening[9] = buffer[9];
			nextWhitening[10] = buffer[10];
			nextWhitening[11] = buffer[11];
			nextWhitening[12] = buffer[12];
			nextWhitening[13] = buffer[13];
			nextWhitening[14] = buffer[14];
			nextWhitening[15] = buffer[15];
		}
		performDecryptionAlgorithm(buffer, 3);		//decrypt
		if (currentBlock > numberOfBlocks) {
			//this is the last block, only write the correct number of characters
			plaintext = plaintext.substr(0, remainingChars);
		}
		outfile << plaintext;
		currentBlock++;
	}
	auto done = std::chrono::high_resolution_clock::now();
	cout << std::chrono::duration_cast<std::chrono::milliseconds>(done - start).count() / 1000 << "." << std::chrono::duration_cast<std::chrono::milliseconds>(done - start).count() % 1000 << " seconds";
	cout << endl;
	outfile.close();
}

void generateKeys128Bit(char* key) {

	/*
		This method takes the user-provided key and creates the key schedule to be used in the rounds of encryption/decryption

		The entire algorithm to create the key schedule is within this method
	*/

	unsigned int word0, word1, word2, word3, t, prevWord0, prevWord1, prevWord2, prevWord3;

	//set the initial key in the schedule
	keySched[0][0][0] = key[0];
	keySched[0][1][0] = key[1];
	keySched[0][2][0] = key[2];
	keySched[0][3][0] = key[3];
	keySched[0][0][1] = key[4];
	keySched[0][1][1] = key[5];
	keySched[0][2][1] = key[6];
	keySched[0][3][1] = key[7];
	keySched[0][0][2] = key[8];
	keySched[0][1][2] = key[9];
	keySched[0][2][2] = key[10];
	keySched[0][3][2] = key[11];
	keySched[0][0][3] = key[12];
	keySched[0][1][3] = key[13];
	keySched[0][2][3] = key[14];
	keySched[0][3][3] = key[15];

	//set each previousWord variable to be used in the future rounds
	prevWord0 = keySched[0][0][0];
	prevWord0 <<= 8;
	prevWord0 += keySched[0][1][0];
	prevWord0 <<= 8;
	prevWord0 += keySched[0][2][0];
	prevWord0 <<= 8;
	prevWord0 += keySched[0][3][0];

	prevWord1 = keySched[0][0][1];
	prevWord1 <<= 8;
	prevWord1 += keySched[0][1][1];
	prevWord1 <<= 8;
	prevWord1 += keySched[0][2][1];
	prevWord1 <<= 8;
	prevWord1 += keySched[0][3][1];

	prevWord2 = keySched[0][0][2];
	prevWord2 <<= 8;
	prevWord2 += keySched[0][1][2];
	prevWord2 <<= 8;
	prevWord2 += keySched[0][2][2];
	prevWord2 <<= 8;
	prevWord2 += keySched[0][3][2];


	//t = word 3
	t = keySched[0][0][3];
	t <<= 8;
	t += keySched[0][1][3];
	t <<= 8;
	t += keySched[0][2][3];
	t <<= 8;
	t += keySched[0][3][3];

	prevWord3 = t;

	//rotate left 1 byte
	t &= 0x00FFFFFF;
	t <<= 8;
	t += keySched[0][0][3];


	//substitution then xor with round constant
	int indexForSubstition, firstSub, secondSub, thirdSub, fourthSub;
	indexForSubstition = (t & 0xFF000000) >> 24;
	firstSub = subBytesMap[indexForSubstition];
	indexForSubstition = (t & 0x00FF0000) >> 16;
	secondSub = subBytesMap[indexForSubstition];
	indexForSubstition = (t & 0x0000FF00) >> 8;
	thirdSub = subBytesMap[indexForSubstition];
	indexForSubstition = t & 0x000000FF;
	fourthSub = subBytesMap[indexForSubstition];

	t = 0;
	t += (firstSub << 24) + (secondSub << 16) + (thirdSub << 8) + fourthSub;
	t ^= (rcon[0] << 24);


	/*
	Each round works with 4 words, the variables 'word0' thru 'word4' are variables that will map to the current 4 words being manipulated
	*/
	for (int i = 1; i < 11; i++) {
		word0 = t ^ prevWord0;
		word1 = word0 ^ prevWord1;
		word2 = word1 ^ prevWord2;
		word3 = word2 ^ prevWord3;

		/*
		Store each byte of each word into its proper place
		*/
		keySched[i][0][0] = (word0 & 0xFF000000) >> 24;
		keySched[i][1][0] = (word0 & 0x00FF0000) >> 16;
		keySched[i][2][0] = (word0 & 0x0000FF00) >> 8;
		keySched[i][3][0] = (word0 & 0x000000FF);

		keySched[i][0][1] = (word1 & 0xFF000000) >> 24;
		keySched[i][1][1] = (word1 & 0x00FF0000) >> 16;
		keySched[i][2][1] = (word1 & 0x0000FF00) >> 8;
		keySched[i][3][1] = (word1 & 0x000000FF);

		keySched[i][0][2] = (word2 & 0xFF000000) >> 24;
		keySched[i][1][2] = (word2 & 0x00FF0000) >> 16;
		keySched[i][2][2] = (word2 & 0x0000FF00) >> 8;
		keySched[i][3][2] = (word2 & 0x000000FF);

		keySched[i][0][3] = (word3 & 0xFF000000) >> 24;
		keySched[i][1][3] = (word3 & 0x00FF0000) >> 16;
		keySched[i][2][3] = (word3 & 0x0000FF00) >> 8;
		keySched[i][3][3] = (word3 & 0x000000FF);


		/*
		process to create T
		*/
		t = keySched[i][0][3];
		t <<= 8;
		t += keySched[i][1][3];
		t <<= 8;
		t += keySched[i][2][3];
		t <<= 8;
		t += keySched[i][3][3];

		//rotate left 1 byte
		t &= 0x00FFFFFF;
		t <<= 8;
		t += keySched[i][0][3];

		//substitution
		int indexForSubstition, firstSub, secondSub, thirdSub, fourthSub;
		indexForSubstition = (t & 0xFF000000) >> 24;
		firstSub = subBytesMap[indexForSubstition];
		indexForSubstition = (t & 0x00FF0000) >> 16;
		secondSub = subBytesMap[indexForSubstition];
		indexForSubstition = (t & 0x0000FF00) >> 8;
		thirdSub = subBytesMap[indexForSubstition];
		indexForSubstition = t & 0x000000FF;
		fourthSub = subBytesMap[indexForSubstition];

		t = 0;
		t += (firstSub << 24) + (secondSub << 16) + (thirdSub << 8) + fourthSub;
		t ^= (rcon[i] << 24);

		// store current words to be used next round
		prevWord0 = word0;
		prevWord1 = word1;
		prevWord2 = word2;
		prevWord3 = word3;
	}
}


bool keyIsWrongFormat(char* key) {

	//method to check for proper key formats, a valid key returns FALSE, an invalid key returns TRUE
	//-- 18 characters means 16 characters with 2 single quotations
	//-- 32 characters means hex digits only


	char spaceCharacter[] = " ";
	int keyLength = (unsigned)strlen(key);

	bool hasspace = (strstr(key, spaceCharacter) == nullptr);
	bool ssq = key[0] == '\'';
	bool esq = key[9] == '\'';


	if (keyLength == 18 && key[0] == '\'' && key[17] == '\'') {
		// key is 18 characters
		// key does not contain a space
		// first and last character are single quotations

		return false;
	}
	else if (keyLength == 32) {                             //hex digits only
															//parse the hex digits and make sure they are in range
		string keyAsString = key;
		std::transform(keyAsString.begin(), keyAsString.end(), keyAsString.begin(), ::tolower);
		for (int i = 0; i < keyAsString.size(); i++) {
			if ((keyAsString.at(i) < '0' || keyAsString.at(i) > '9') && (keyAsString.at(i) < 'a' || keyAsString.at(i) > 'f')) {
				return true;
			}
		}
		return false;
	}
	else {
		//not valid
		return true;
	}

}


bool inputFileNotValid(char* inputFile) {
	/*
	If the input file is not found then this method will let the program know
	*/
	std::string fileLocation = inputFile;
	fstream myFile(fileLocation);
	if (!myFile) {
		return true;
	}
	return false;
}


void validateParameters(int argCount, char* arguments[]) {
	/*
	This method takes the command line arguments and checks them for correctness.  If there are not 6 arguments
	then the arguments are not valid.  If there are 6 then each argument must be validated.

	If there is any error in the arguments then the program will halt and alert the user via console.
	*/

	if (argCount < 6) {                                      //check argument count, must be 6
		std::cout << "You have not provided enough arguments." << endl;
		exit(1);
	}

	string modeString = arguments[3];                       //pull the 'mode' argument to set as lowercase
	std::transform(modeString.begin(), modeString.end(), modeString.begin(), ::tolower);



	string cmpStr1 = "-e";
	string cmpStr2 = "-E";
	string cmpStr3 = "-D";
	string cmpStr4 = "-d";

	//check for a correct action
	if (arguments[1] != cmpStr1 && arguments[1] != cmpStr2 && arguments[1] != cmpStr3 && arguments[1] != cmpStr4) {
		cout << "You have provided an invalid action in your arguments.  Use -e to encrypt or -d to decrypt.";
		exit(2);
	}
	//check for correct key format
	else if (keyIsWrongFormat(arguments[2])) {
		cout << "The key you have provided is in an incorrect format.  Please read below for the 3 acceptable formats." << endl;
		cout << "1. The key must 16 hex digits (case insensitive)" << endl;
		cout << "2. The key must be 8 characters long enclosed in single quotations." << endl;
		cout << "\tIf the key contains spaces as a character you must enclose the single-quotation-enclosed key" << endl;
		cout << "\tmust be enclosed in double quotations";
		exit(3);
		//TODO : : change the text to be more readable
	}
	else if (modeString != "ecb" && modeString != "cbc") {
		cout << "You have entered an incorrect mode.  The mode must be either ECB or CBC (both case insensitive)";
		exit(4);
	}

	//if the input file does not exist there is an error
	else if (inputFileNotValid(arguments[4])) {
		cout << "The input file was not found.  Please select a valid file." << endl;
		exit(5);
	}
	else {

	}


}

int main(int argc, char* argv[])
{

	//make sure user entered correct input
	validateParameters(argc, argv);

	//place user inputs in variables
	string action = argv[1];
	char*  inputKey = argv[2];
	string keyAsStr = inputKey;
	string mode = argv[3];
	infileName = argv[4];
	outfileName = argv[5];


	//set CBC mode on or off
	std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
	isCBC = (mode == "cbc");

	//if the key length was 32 then we have HEX digits. these must be converted to 8-bit chars
	if (keyAsStr.length() == 32) {
		char keysToFill[16];
		convertKeyFromHex(inputKey, keysToFill);
		inputKey = keysToFill;
	}
	else {
		//otherwise remove the apostrophes from the key 
		char keyWithoutApostrophe[16];
		for (int i = 1; i < 17; i++) {
			keyWithoutApostrophe[i - 1] = inputKey[i];
		}
		inputKey = keyWithoutApostrophe;
	}

	//generate the keys once the users input key is ready
	generateKeys128Bit(inputKey);

	//determine whether to encrypt or decrypt
	if (action == "-e" || action == "-E") {
		encrypt();
	}
	else {
		decrypt();
	}
	return 0;
}