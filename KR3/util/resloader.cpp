#include "stdafx.h"
#include "resloader.h"


kr::krb::File::File(const fchar_t* path) noexcept
{
	krb_fopen_std_file(this, path, _TF("rb"));
}
kr::krb::File::~File() noexcept
{
	krb_fclose(this);
}

#ifdef _DEBUG
#pragma comment(lib, "ken-res-loaderd.lib")
#else
#pragma comment(lib, "ken-res-loader.lib")
#endif