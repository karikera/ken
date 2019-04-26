#pragma once

#define VERSION_OF_3DS 3

//>------ Primary Chunk, at the beginning of each file
#define PRIMARY       0x4D4D

//>------ Main Chunks
#define EDIT3DS       0x3D3D
#define VERSION       0x0002
#define MESHVERSION   0x3D3E
#define EDITKEYFRAME  0xB000

//>------ sub defines of EDIT3DS
#define EDITMATERIAL  0xAFFF
#define EDITOBJECT    0x4000

//>------ sub defines of EDITMATERIAL
#define MATNAME       0xA000  
#define MATLUMINANCE  0xA010
#define MATDIFFUSE    0xA020
#define MATSPECULAR   0xA030
#define MATSHININESS  0xA040
#define MATMAP        0xA200
#define MATMAPFILE    0xA300

#define OBJTRIMESH    0x4100

//>------ sub defines of OBJTRIMESH
#define TRIVERT       0x4110
#define TRIFACE       0x4120
#define TRIFACEMAT    0x4130
#define TRIUV         0x4140
#define TRISMOOTH     0x4150
#define TRILOCAL      0x4160

//>------ sub defines of EIDTKEYFRAME
#define KFMESH        0xB002
#define KFHEIRARCHY   0xB030
#define KFNAME        0xB010


//>>------  these define the different color chunk types
#define RGBF   0x0010
#define RGB24  0x0011

